#include <node.h>
#include <Python.h>
#include "py_object_wrapper.h"
#include "utils.h"
#include "datetime.h"
#include "time.h"

Persistent<FunctionTemplate> PyObjectWrapper::py_function_template;

void PyObjectWrapper::Initialize() {
    HandleScope scope;

    PyDateTime_IMPORT;

    Local<FunctionTemplate> fn_tpl = FunctionTemplate::New();
    Local<ObjectTemplate> proto = fn_tpl->PrototypeTemplate();
    Local<ObjectTemplate> obj_tpl = fn_tpl->InstanceTemplate();

    obj_tpl->SetInternalFieldCount(1);

    // this has first priority. see if the properties already exist on the python object
    obj_tpl->SetNamedPropertyHandler(Get, Set);

    // If we're calling `toString`, delegate to our version of ToString
    proto->SetAccessor(String::NewSymbol("toString"), ToStringAccessor); 

    // likewise for valueOf
    obj_tpl->SetAccessor(String::NewSymbol("valueOf"), ValueOfAccessor); 

    // Python objects can be called as functions.
    obj_tpl->SetCallAsFunctionHandler(Call, Handle<Value>());

    py_function_template = Persistent<FunctionTemplate>::New(fn_tpl);
}

Handle<Value> PyObjectWrapper::New(PyObject* obj) {
    HandleScope scope;
    Local<Value> jsObj;

    try {
        jsObj = ConvertToJavaScript(obj);
    } catch (int e) {
        if (PyErr_Occurred()) {
            return ThrowPythonException();
        } 
    }

    if (PyErr_Occurred()) {
        return ThrowPythonException();
    }  

    return scope.Close(jsObj);
}

Handle<Value> PyObjectWrapper::Get(Local<String> key, const AccessorInfo& info) {
    HandleScope scope;
    PyObjectWrapper* wrapper = ObjectWrap::Unwrap<PyObjectWrapper>(info.Holder());
    String::Utf8Value utf8_key(key);
    string value(*utf8_key);
    PyObject* result = wrapper->InstanceGet(value);
    if(result) {
        return scope.Close(PyObjectWrapper::New(result));
    }
    return Handle<Value>();
}

Handle<Value> PyObjectWrapper::Set(Local<String> key, Local<Value> value, const AccessorInfo& info) {
    // we don't know what to do.
    return Undefined();
}

Handle<Value> PyObjectWrapper::CallAccessor(Local<String> property, const AccessorInfo& info) {
    HandleScope scope;
    Local<FunctionTemplate> func = FunctionTemplate::New(Call);
    return scope.Close(func->GetFunction());
}

Handle<Value> PyObjectWrapper::ToStringAccessor(Local<String> property, const AccessorInfo& info) {
    HandleScope scope;
    Local<FunctionTemplate> func = FunctionTemplate::New(ToString);
    return scope.Close(func->GetFunction());
}

Handle<Value> PyObjectWrapper::ValueOfAccessor(Local<String> property, const AccessorInfo& info) {
    HandleScope scope;
    Local<FunctionTemplate> func = FunctionTemplate::New(ValueOf);
    return scope.Close(func->GetFunction());
}

Handle<Value> PyObjectWrapper::Call(const Arguments& args) {
    HandleScope scope;
    PyObjectWrapper* pyobjwrap = ObjectWrap::Unwrap<PyObjectWrapper>(args.This());
    Handle<Value> result = pyobjwrap->InstanceCall(args);
    return scope.Close(result);
}

Handle<Value> PyObjectWrapper::ToString(const Arguments& args) {
    HandleScope scope;
    PyObjectWrapper* pyobjwrap = ObjectWrap::Unwrap<PyObjectWrapper>(args.This());
    Local<String> result = String::New(pyobjwrap->InstanceToString(args).c_str());
    return scope.Close(result);
}

Handle<Value> PyObjectWrapper::ValueOf(const Arguments& args) {
    HandleScope scope;
    PyObjectWrapper* pyobjwrap = ObjectWrap::Unwrap<PyObjectWrapper>(args.This());
    PyObject* py_obj = pyobjwrap->InstanceGetPyObject();
    if(PyCallable_Check(py_obj)) {
        Local<FunctionTemplate> call = FunctionTemplate::New(Call);
        return scope.Close(call->GetFunction());
    } else if (PyNumber_Check(py_obj)) {
        long long_result = PyLong_AsLong(py_obj);
        return scope.Close(Integer::New(long_result));
    } else if (PySequence_Check(py_obj)) {
        int len = PySequence_Length(py_obj);
        Local<Array> array = Array::New(len);
        for(int i = 0; i < len; ++i) {
            Handle<Object> jsobj = PyObjectWrapper::py_function_template->GetFunction()->NewInstance();
            PyObject* py_obj_out = PySequence_GetItem(py_obj, i);
            PyObjectWrapper* obj_out = new PyObjectWrapper(py_obj_out);
            obj_out->Wrap(jsobj);
            array->Set(i, jsobj);
        }
        return scope.Close(array);
    } else if (PyMapping_Check(py_obj)) {
        int len = PyMapping_Length(py_obj);
        Local<Object> object = Object::New();
        PyObject* keys = PyMapping_Keys(py_obj);
        PyObject* values = PyMapping_Values(py_obj);
        for(int i = 0; i < len; ++i) {
            PyObject *key = PySequence_GetItem(keys, i);
            PyObject *value = PySequence_GetItem(values, i);
            Local<Object> jsobj = PyObjectWrapper::py_function_template->GetFunction()->NewInstance();
            PyObjectWrapper* obj_out = new PyObjectWrapper(value);
            obj_out->Wrap(jsobj);
        }
        Py_XDECREF(keys);
        Py_XDECREF(values);
        return scope.Close(object);
    }
    return Undefined();
}

Local<Value> PyObjectWrapper::ConvertToJavaScript(PyObject* obj) {
    Local<Value> jsVal;
    // undefined
    if(obj == Py_None) {
        jsVal = Local<Value>::New(Undefined());
    }
    // integer (can be 64b)
    else if(PyInt_CheckExact(obj)) {
        long i = PyInt_AsLong(obj);
        jsVal = Local<Value>::New(Number::New((double) i));
    }
    // double
    else if(PyFloat_CheckExact(obj)) {
        double d = PyFloat_AsDouble(obj);
        jsVal = Local<Value>::New(Number::New(d));
    }
    // long
    else if(PyLong_CheckExact(obj)) {
        double d = PyLong_AsDouble(obj);
        jsVal = Local<Value>::New(Number::New(d));
    }
    // string
    else if(PyString_CheckExact(obj)) {
        jsVal = Local<Value>::New(String::New(PyString_AsString(obj)));
    }
    // bool
    else if(PyBool_Check(obj)) {
        int b = PyObject_IsTrue(obj);
        jsVal = Local<Value>::New(Boolean::New(b));
    }
    // date
    else if (PyDateTime_Check(obj)) {
        struct tm tmp;

        int year = PyDateTime_GET_YEAR(obj);
        int month = PyDateTime_GET_MONTH(obj);
        int day = PyDateTime_GET_DAY(obj);

        tmp.tm_year = year - 1900;
        tmp.tm_mon = month - 1;
        
        if ((day == 28) && (month == 2) && (year % 4 == 0) && (year % 100 == 0 && year % 400 != 0)) {
            tmp.tm_mday = 29;    
        } else {
            tmp.tm_mday = day;
        }
        
        tmp.tm_hour = PyDateTime_DATE_GET_HOUR(obj) + 1;
        tmp.tm_min = PyDateTime_DATE_GET_MINUTE(obj);
        tmp.tm_sec = PyDateTime_DATE_GET_SECOND(obj);

        time_t result = mktime(&tmp) * 1000 + PyDateTime_DATE_GET_MICROSECOND(obj);

        jsVal = v8::Date::New(result);
    }
    // dict
    else if(PyDict_CheckExact(obj)) {
        Local<Object> dict = v8::Object::New();
        PyObject *key, *value;
        Py_ssize_t pos = 0;

        while (PyDict_Next(obj, &pos, &key, &value)) {
            Handle<Value> jsKey = ConvertToJavaScript(key);
            Handle<Value> jsValue = ConvertToJavaScript(value);

            dict->Set(jsKey, jsValue);
        }
        jsVal = dict;
    }
    // list
    else if(PyList_CheckExact(obj)) {
        int size = PyList_Size(obj);
        Local<Array> array = v8::Array::New(size);
        PyObject* value;
        for(int i = 0; i < size; i++ ){
            value = PyList_GetItem(obj, i);
            array->Set(i, ConvertToJavaScript(value));
        }
        jsVal = array;
    } else {
        Py_XINCREF(obj);
        Local<Object> jsObj = py_function_template->GetFunction()->NewInstance();
        PyObjectWrapper* wrapper = new PyObjectWrapper(obj);
        wrapper->Wrap(jsObj);
        jsVal = Local<Value>::New(jsObj);
    }
    
    return jsVal;
}

PyObject* PyObjectWrapper::ConvertToPython(const Handle<Value>& value) {
    int len;
    if (value->IsString()) {
        return PyString_FromString(*String::Utf8Value(value->ToString()));
    } else if (value->IsBoolean()) {
        if (value->ToBoolean()->IsTrue()) {
            return Py_True;
        } else {
            return Py_False;
        }
    } else if (value->IsNull() || value->IsUndefined()) {
        return Py_None;
    } else if (value->IsNumber()) {
        return PyFloat_FromDouble(value->NumberValue());
    } else if (value->IsDate()) {
        Handle<Date> dt = Handle<Date>::Cast(value);
	    long sinceEpoch = dt->NumberValue();
    	long time = sinceEpoch / 1000;
    	time_t timestamp = static_cast<time_t>(time);
    	struct tm* tmp = localtime(&timestamp);	
        return PyDateTime_FromDateAndTime(tmp->tm_year + 1900, tmp->tm_mon + 1, tmp->tm_mday, tmp->tm_hour, tmp->tm_min, tmp->tm_sec, sinceEpoch % 1000);;
    } else if (value->IsObject()) {
        if (value->IsArray()) {
            Local<Array> array = Array::Cast(*value);
            len = array->Length();
            PyObject* py_list = PyList_New(len);
            for (int i = 0; i < len; ++i) {
                Local<Value> js_val = array->Get(i);
                PyList_SET_ITEM(py_list, i, ConvertToPython(js_val));
            }
            return py_list;
        } else {
            Local<Object> obj = value->ToObject();

            if(!obj->FindInstanceInPrototypeChain(PyObjectWrapper::py_function_template).IsEmpty()) {
                PyObjectWrapper* python_object = ObjectWrap::Unwrap<PyObjectWrapper>(value->ToObject());
                PyObject* pyobj = python_object->InstanceGetPyObject();
                return pyobj;
            } else {
                Local<Array> property_names = obj->GetPropertyNames();
                len = property_names->Length();
                PyObject* py_dict = PyDict_New();

                for (int i = 0; i < len; ++i) {
                    Local<String> str = property_names->Get(i)->ToString();
                    Local<Value> js_val = obj->Get(str);
                    PyDict_SetItemString(py_dict, *String::Utf8Value(str), ConvertToPython(js_val));
                }
                return py_dict;
            }
        }
        return NULL;
    } else if (value->IsArray()) {
        Local<Array> array = Array::Cast(*value);
        len = array->Length();
        PyObject* py_list = PyList_New(len);
        for (int i = 0; i < len; ++i) {
            Local<Value> js_val = array->Get(i);
            PyList_SET_ITEM(py_list, i, ConvertToPython(js_val));
        }
        return py_list;
    } else if (value->IsUndefined()) {
        Py_RETURN_NONE;
    }

    return NULL;
}

Handle<Value> PyObjectWrapper::InstanceCall(const Arguments& args) {
    // for now, we don't do anything.
    HandleScope scope;
    
    int len = args.Length();
    PyObject* args_tuple = PyTuple_New(len);
    for (int i = 0; i < len; ++i) {
        PyTuple_SET_ITEM(args_tuple, i, ConvertToPython(args[i]));
    }

    PyObject* result = PyObject_CallObject(mPyObject, args_tuple);
    //Py_XDECREF(args_tuple);
    
    if (PyErr_Occurred()) {
        return ThrowPythonException();
    }

    if (result) {
        Handle<Value> val = PyObjectWrapper::New(result);
        Py_XDECREF(result);
        return scope.Close(val);
    } else {
        return ThrowPythonException();
    }
}

string PyObjectWrapper::InstanceToString(const Arguments& args) {
    PyObject* as_string = PyObject_Str(mPyObject);
    return PyString_AsString(as_string);
}

PyObject* PyObjectWrapper::InstanceGet(const string& key) {
    if(PyObject_HasAttrString(mPyObject, key.c_str())) {
        return PyObject_GetAttrString(mPyObject, key.c_str());
    }
    return (PyObject*)NULL;
}
