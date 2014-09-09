#include <node.h>
#include "py_object_wrapper.h"
#include <datetime.h>
#include <time.h>
#include "utils.h"


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
    Local<Value> jsVal;

    // undefined
    if(obj == Py_None) {
        jsVal = Local<Value>::New(Undefined());
    }

    else if (PyDate_CheckExact(obj)) {
        printf("%s\n", "wat");
    }

    else if (PyDateTime_CheckExact(obj)) {
        printf("%s\n", "wat");
        /* Fields are packed into successive bytes, each viewed as unsigned and
         * big-endian, unless otherwise noted:
         *
         * byte offset
         *  0 		year     2 bytes, 1-9999
         *  2		month    1 byte, 1-12
         *  3 		day      1 byte, 1-31
         *  4		hour     1 byte, 0-23
         *  5 		minute   1 byte, 0-59
         *  6 		second   1 byte, 0-59
         *  7 		usecond  3 bytes, 0-999999
         * 10
         */

         /*
            #define PyDateTime_GET_YEAR(o)     ((((PyDateTime_Date*)o)->data[0] << 8) | \
                                     ((PyDateTime_Date*)o)->data[1])
            #define PyDateTime_GET_MONTH(o)    (((PyDateTime_Date*)o)->data[2])
            #define PyDateTime_GET_DAY(o)      (((PyDateTime_Date*)o)->data[3])

            #define PyDateTime_DATE_GET_HOUR(o)        (((PyDateTime_DateTime*)o)->data[4])
            #define PyDateTime_DATE_GET_MINUTE(o)      (((PyDateTime_DateTime*)o)->data[5])
            #define PyDateTime_DATE_GET_SECOND(o)      (((PyDateTime_DateTime*)o)->data[6])
            #define PyDateTime_DATE_GET_MICROSECOND(o)      \
                ((((PyDateTime_DateTime*)o)->data[7] << 16) |   \
                     (((PyDateTime_DateTime*)o)->data[8] << 8)  |   \
                      ((PyDateTime_DateTime*)o)->data[9])


            #define PyDateTime_TIME_GET_HOUR(o)        (((PyDateTime_Time*)o)->data[0])
            #define PyDateTime_TIME_GET_MINUTE(o)      (((PyDateTime_Time*)o)->data[1])
            #define PyDateTime_TIME_GET_SECOND(o)      (((PyDateTime_Time*)o)->data[2])
            #define PyDateTime_TIME_GET_MICROSECOND(o)      \
                ((((PyDateTime_Time*)o)->data[3] << 16) |   \
                     (((PyDateTime_Time*)o)->data[4] << 8)  |   \
                      ((PyDateTime_Time*)o)->data[5])
        */

        double microseconds = (double) 
                    ((((PyDateTime_DateTime*)obj)->data[0] << 4096) |   \
                     (((PyDateTime_DateTime*)obj)->data[1] << 2048) |   \
                     (((PyDateTime_DateTime*)obj)->data[2] << 1024) |   \
                     (((PyDateTime_DateTime*)obj)->data[3] << 512) |   \
                     (((PyDateTime_DateTime*)obj)->data[4] << 256) |   \
                     (((PyDateTime_DateTime*)obj)->data[5] << 128) |   \ 
                     (((PyDateTime_DateTime*)obj)->data[6] << 64) |   \
                     (((PyDateTime_DateTime*)obj)->data[6] << 32) |   \
                     (((PyDateTime_DateTime*)obj)->data[7] << 16) |   \
                     (((PyDateTime_DateTime*)obj)->data[8] << 8)  |   \
                      ((PyDateTime_DateTime*)obj)->data[9]);
        printf("%d\n", microseconds);

        jsVal = v8::Date::New(microseconds);
    }

    // double
    else if(PyFloat_CheckExact(obj)) {
        double d = PyFloat_AsDouble(obj);
        jsVal = Local<Value>::New(Number::New(d));
    }
    // integer (can be 64b)
    else if(PyInt_CheckExact(obj)) {
        long i = PyInt_AsLong(obj);
        jsVal = Local<Value>::New(Number::New((double) i));
    }
    // string
    else if(PyString_CheckExact(obj)) {
        // ref to internal representation: no need to dealloc
        char *str = PyString_AsString(obj);
        if(str) {
            jsVal = Local<Value>::New(String::New(str));
        }
    }
    else if(PyBool_Check(obj)) {
        int b = PyObject_IsTrue(obj);
        if(b != -1) {
            jsVal = Local<Value>::New(Boolean::New(b));
        }
    }

    if(PyErr_Occurred()) {
        Py_XDECREF(obj);
        return ThrowPythonException();
    }
    
    if(jsVal.IsEmpty()) {
        // printf("%s\n", "wat");
        Local<Object> jsObj = py_function_template->GetFunction()->NewInstance();
        PyObjectWrapper* wrapper = new PyObjectWrapper(obj);
        wrapper->Wrap(jsObj);
        jsVal = Local<Value>::New(jsObj);
    }
    else {
        Py_XDECREF(obj);
    }
    return scope.Close(jsVal);
}

Handle<Value> PyObjectWrapper::Get(Local<String> key, const AccessorInfo& info) {
    // returning an empty Handle<Value> object signals V8 that we didn't
    // find the property here, and we should check the "NamedAccessor" functions
    HandleScope scope;
    PyObjectWrapper* wrapper = ObjectWrap::Unwrap<PyObjectWrapper>(info.Holder());
    String::Utf8Value utf8_key(key);
    string value(*utf8_key);
    PyObject* result = wrapper->InstanceGet(value);
    if(result) {
        return PyObjectWrapper::New(result);
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
            PyObject *key = PySequence_GetItem(keys, i),
                *value = PySequence_GetItem(values, i),
                *key_as_string = PyObject_Str(key);
            char* cstr = PyString_AsString(key_as_string);

            Local<Object> jsobj = PyObjectWrapper::py_function_template->GetFunction()->NewInstance();
            PyObjectWrapper* obj_out = new PyObjectWrapper(value);
            obj_out->Wrap(jsobj);
            Py_XDECREF(key);
            Py_XDECREF(key_as_string);
        }
        Py_XDECREF(keys);
        Py_XDECREF(values);
        return scope.Close(object);
    }
    return Undefined();
}

PyObject* PyObjectWrapper::ConvertToPython(const Handle<Value>& value) {
    int len;
    HandleScope scope;

    if(value->IsString()) {
        return PyString_FromString(*String::Utf8Value(value->ToString()));
    } else if (value->IsBoolean()) {
    	if (value->ToBoolean()->IsTrue()) {
    		return Py_True;
    	} else {
    		return Py_False;
    	}
	} else if(value->IsNumber()) {
        return PyFloat_FromDouble(value->NumberValue());
    } else if(value->IsDate()) {
    	Handle<Date> date = Handle<Date>::Cast(value);
        PyObject* floatObj = PyFloat_FromDouble(date->NumberValue() / 1000.0 ); // javascript returns milliseconds since epoch. python wants seconds since epoch
    	PyObject* timeTuple = Py_BuildValue("(O)", floatObj);
        Py_DECREF(floatObj);
        PyObject* dateTime = PyDateTime_FromTimestamp(timeTuple);
        Py_DECREF(timeTuple);
        return dateTime;
    } else if(value->IsObject()) {
    	if(value->IsArray()) {
			Local<Array> array = Array::Cast(*value);
			len = array->Length();
			PyObject* py_list = PyList_New(len);
			for(int i = 0; i < len; ++i) {
				Local<Object> obj = array->Get(i)->ToObject();
				if (!obj->FindInstanceInPrototypeChain(PyObjectWrapper::py_function_template).IsEmpty()) {
					PyObjectWrapper* python_object = ObjectWrap::Unwrap<PyObjectWrapper>(obj);
					PyObject* pyobj = python_object->InstanceGetPyObject();
					PyList_SET_ITEM(py_list, i, pyobj);
				} else {
					Local<Value> js_val = array->Get(i);
					PyList_SET_ITEM(py_list, i, ConvertToPython(js_val));
				}
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
				for(int i = 0; i < len; ++i) {
					Local<String> str = property_names->Get(i)->ToString();
					Local<Value> js_val = obj->Get(str);
					PyDict_SetItemString(py_dict, *String::Utf8Value(str), ConvertToPython(js_val));
				}
				return py_dict;
			}
		}
        return NULL;
    } else if(value->IsArray()) {
		Local<Array> array = Array::Cast(*value);
		len = array->Length();
		PyObject* py_list = PyList_New(len);
		for(int i = 0; i < len; ++i) {
			Local<Value> js_val = array->Get(i);
			PyList_SET_ITEM(py_list, i, ConvertToPython(js_val));
		}
		return py_list;
    } else if(value->IsUndefined()) {
        Py_RETURN_NONE;
    }
    return NULL;
}

Handle<Value> PyObjectWrapper::InstanceCall(const Arguments& args) {
    // for now, we don't do anything.
    HandleScope scope;
    int len = args.Length();
    PyObject* args_tuple = PyTuple_New(len);

    for(int i = 0; i < len; ++i) {
        PyObject* py_arg = ConvertToPython(args[i]);
        if (PyErr_Occurred()) {
            return ThrowPythonException();
        }
        PyTuple_SET_ITEM(args_tuple, i, py_arg);
    }
    PyObject* result = PyObject_CallObject(mPyObject, args_tuple);
    if (PyErr_Occurred()) {
        return ThrowPythonException();
    }

    Py_XDECREF(args_tuple);

    if(result) {
        return scope.Close(PyObjectWrapper::New(result));
    } else {
        return ThrowPythonException();
    }
}

string PyObjectWrapper::InstanceToString(const Arguments& args) {
    PyObject* as_string = PyObject_Str(mPyObject);
    string native_string(PyString_AsString(as_string));
    Py_XDECREF(as_string);
    return native_string;
}

PyObject* PyObjectWrapper::InstanceGet(const string& key) {
    if(PyObject_HasAttrString(mPyObject, key.c_str())) {
        PyObject* attribute = PyObject_GetAttrString(mPyObject, key.c_str());
        return attribute;
    } 
    return (PyObject*)NULL;
}
