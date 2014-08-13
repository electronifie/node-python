
#ifndef PY_OBJECT_WRAPPER_H
#define PY_OBJECT_WRAPPER_H

#include <string>

#include <node.h>
#include <Python.h>

#include "utils.h"

using namespace v8;
using std::string;

class PyObjectWrapper : public node::ObjectWrap {
    PyObject* mPyObject;
    public:
        static Persistent<FunctionTemplate> py_function_template;
        PyObjectWrapper() : node::ObjectWrap(), mPyObject() {};
        PyObjectWrapper(PyObject* obj) : node::ObjectWrap(), mPyObject(obj) {};
        virtual ~PyObjectWrapper() {
            Py_XDECREF(mPyObject);
            mPyObject = NULL;
        }

        static void Initialize();

        static Handle<Value> New(PyObject* obj);
        static Handle<Value> New(const Arguments& args);

        static Handle<Value> Get(Local<String> key, const AccessorInfo& info);
        static Handle<Value> Set(Local<String> key, Local<Value> value, const AccessorInfo& info);

        static Handle<Value> CallAccessor(Local<String> property, const AccessorInfo& info);

        static Handle<Value> ToStringAccessor(Local<String> property, const AccessorInfo& info);

        static Handle<Value> ValueOfAccessor(Local<String> property, const AccessorInfo& info);

        static Handle<Value> Call(const Arguments& args);

        static Handle<Value> ToString(const Arguments& args);

        static Handle<Value> ValueOf(const Arguments& args);

        static PyObject* ConvertToPython(const Handle<Value>& value);

        PyObject* InstanceGetPyObject() {
            return mPyObject;
        };

        Handle<Value> InstanceCall(const Arguments& args);

        string InstanceToString(const Arguments& args);

        PyObject* InstanceGet(const string& key);
};


#endif
