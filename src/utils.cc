
#include <Python.h>
#include <pyerrors.h>

#include "utils.h"

Handle<Value> ThrowPythonException() {
    PyObject *ptype, *pvalue, *ptraceback;
    PyErr_Fetch(&ptype, &pvalue, &ptraceback);
    PyErr_NormalizeException(&ptype, &pvalue, &ptraceback);

    // maybe useless to protect against bad use of ThrowPythonException ?
    if(pvalue == NULL) {
        Nan::ThrowError("No exception found");
    }

    // handle exception message
    Local<v8::String> msg = Nan::New<String>("Python Error: ").ToLocalChecked();

    if (ptype != NULL) {
        msg = v8::String::Concat(msg, Nan::New<String>(PyString_AsString(PyObject_Str(PyObject_GetAttrString(ptype, "__name__")))).ToLocalChecked());
        msg = v8::String::Concat(msg, Nan::New<String>(": ").ToLocalChecked());

        if (pvalue != NULL) {
            msg = v8::String::Concat(msg, Nan::New<String>(PyString_AsString(PyObject_Str(pvalue))).ToLocalChecked());
        }

        msg = v8::String::Concat(msg, Nan::New<String>("\n").ToLocalChecked());
    }

    if (ptraceback != NULL) {

        PyObject *module_name, *pyth_module, *pyth_func;
        module_name = PyString_FromString("traceback");
        pyth_module = PyImport_Import(module_name);
        Py_DECREF(module_name);

        pyth_func = PyObject_GetAttrString(pyth_module, "format_exception");
        Py_DECREF(pyth_module);

        if (pyth_func) {
            PyObject *pyth_val, *pystr, *ret;
            char *str;

            char *full_backtrace;

            pyth_val = PyObject_CallFunctionObjArgs(pyth_func, ptype, pvalue, ptraceback, NULL);
            ret = PyUnicode_Join(PyUnicode_FromString(""), pyth_val);
            pystr = PyObject_Str(ret);
            str = PyString_AsString(pystr);
            full_backtrace = strdup(str);

            Py_DECREF(pyth_func);
            Py_DECREF(pyth_val);
            Py_DECREF(pystr);
            Py_DECREF(str);

            msg = v8::String::Concat(msg, Nan::New<String>("\n").ToLocalChecked());
            msg = v8::String::Concat(msg, Nan::New<String>(full_backtrace).ToLocalChecked());
        } else {
            msg = v8::String::Concat(msg, Nan::New<String>("\n").ToLocalChecked());
            msg = v8::String::Concat(msg, Nan::New<String>(PyString_AsString(PyObject_Str(ptraceback))).ToLocalChecked());
        }

    }

    Local<Value> err;
    if (PyErr_GivenExceptionMatches(ptype, PyExc_ReferenceError)) {
        err = Exception::ReferenceError(msg);
    }
    else if (PyErr_GivenExceptionMatches(ptype, PyExc_SyntaxError)) {
        err = Exception::SyntaxError(msg);
    }
    else if (PyErr_GivenExceptionMatches(ptype, PyExc_TypeError)) {
        err = Exception::TypeError(msg);
    }
    else {
        err = Exception::Error(msg);
    }

    // @TODO : handle stacktrace

    Py_XDECREF(ptype);
    Py_XDECREF(pvalue);
    Py_XDECREF(ptraceback);

    Nan::ThrowError(err);
}
