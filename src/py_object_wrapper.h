
#ifndef PY_OBJECT_WRAPPER_H
#define PY_OBJECT_WRAPPER_H

#include <string>

#include <node.h>
#include <nan.h>
#include <Python.h>

#include "utils.h"

using namespace v8;
using std::string;

class PyObjectWrapper : public node::ObjectWrap {
    PyObject* mPyObject;
    public:
        static Persistent<FunctionTemplate> py_function_template;
        PyObjectWrapper(PyObject* obj) : node::ObjectWrap(), mPyObject(obj) {};
        virtual ~PyObjectWrapper() {
            Py_XDECREF(mPyObject);
            mPyObject = NULL;
        }

        static void Initialize();

        static Handle<Value> New(PyObject* obj);
        static NAN_METHOD(New);

        static NAN_GETTER(Get);
        static NAN_SETTER(Set);

        static NAN_GETTER(CallAccessor);

        static NAN_GETTER(ToStringAccessor);

        static NAN_GETTER(ValueOfAccessor);

        static NAN_METHOD(Call);

        static NAN_METHOD(ToString);

        static NAN_METHOD(ValueOf);

        static PyObject* ConvertToPython(const Handle<Value>& value);

        static Local<Value> ConvertToJavaScript(PyObject* obj);

        PyObject* InstanceGetPyObject() {
            return mPyObject;
        };

        NAN_METHOD(InstanceCall);

        string InstanceToString(Nan::NAN_METHOD_ARGS_TYPE args);

        PyObject* InstanceGet(const string& key);
};


#endif
