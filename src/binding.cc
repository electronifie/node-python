#include <node.h>
#include <Python.h>
#include <datetime.h>
#include "py_object_wrapper.h"
#include "utils.h"
#include <iostream>

using namespace v8;
using namespace node;
using std::string;

NAN_METHOD(eval) {
    Nan::EscapableHandleScope();
    if (info.Length() < 1 || !info[0]->IsString()) {
        Nan::ThrowError("A string expression must be provided.");
    }

    PyCodeObject* code = (PyCodeObject*) Py_CompileString(*String::Utf8Value(info[0]->ToString()), "eval", Py_eval_input);
    PyObject* main_module = PyImport_AddModule("__main__");
    PyObject* global_dict = PyModule_GetDict(main_module);
    PyObject* local_dict = PyDict_New();
    PyObject* obj = PyEval_EvalCode(code, global_dict, local_dict);
    PyObject* result = PyObject_Str(obj);

    Py_XDECREF(code);
    Py_XDECREF(global_dict);
    Py_XDECREF(local_dict);
    Py_XDECREF(obj);

    PyObjectWrapper::New(result);
}

NAN_METHOD(finalize) {
	Nan::HandleScope();
    Py_Finalize();
    Nan::Undefined();
}

NAN_METHOD(import) {
    Nan::EscapableHandleScope();
    if (info.Length() < 1 || !info[0]->IsString()) {
        Nan::ThrowError("I don't know how to import that.");
    }

    PyObject* module_name;
    PyObject* module;

    module_name = PyUnicode_FromString(*String::Utf8Value(info[0]->ToString()));
    module = PyImport_Import(module_name);

    if (PyErr_Occurred()) {
        Nan::ThrowError(ThrowPythonException());
    }

    if (!module) {
        Nan::ThrowError(ThrowPythonException());
    }
    Py_XDECREF(module_name);

    PyObjectWrapper::New(module);
}

void init (Handle<Object> exports) {
    Nan::HandleScope();

    Py_Initialize();
    PyObjectWrapper::Initialize();

    // how to schedule Py_Finalize(); to be called when process exits?

    // module.exports.eval
    Nan::Export(exports, "eval", eval);

    // module.exports.finalize
    Nan::Export(exports, "finalize", finalize);

    // module.exports.import
    Nan::Export(exports, "import", import);

    // module.exports.PyObject
    // Nan::Export(exports, "PyObject", PyObject);

}

NODE_MODULE(binding, init)
