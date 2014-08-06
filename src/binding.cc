#include <node.h>
#include <Python.h>

#include "py_object_wrapper.h"
#include "utils.h"

using namespace v8;
using namespace node;
using std::string;

Handle<Value> eval(const Arguments& args) {
    HandleScope scope; 
    if (args.Length() < 1 || !args[0]->IsString()) {
        return ThrowException(
            Exception::Error(String::New("A string expression must be provided."))
        );
    }
    
    PyCodeObject* code = (PyCodeObject*) Py_CompileString(*String::Utf8Value(args[0]->ToString()), "test", Py_eval_input);
    PyObject* main_module = PyImport_AddModule("__main__");
    PyObject* global_dict = PyModule_GetDict(main_module);
    PyObject* local_dict = PyDict_New();
    PyObject* obj = PyEval_EvalCode(code, global_dict, local_dict);
    PyObject* result = PyObject_Str(obj);

    return scope.Close(PyObjectWrapper::New(result));
}

Handle<Value> import(const Arguments& args) {
    HandleScope scope; 
    if (args.Length() < 1 || !args[0]->IsString()) {
        return ThrowException(
            Exception::Error(String::New("I don't know how to import that."))
        );
    }

    PyObject* module_name;
    PyObject* module;
    
    module_name = PyUnicode_FromString(*String::Utf8Value(args[0]->ToString()));
    module = PyImport_Import(module_name);    

    if (PyErr_Occurred()) {

        Local<v8::String> errMsg = v8::String::Concat(v8::String::New("Could not import "), args[0]->ToString());
        
        PyObject *ptype, *pvalue, *ptraceback;
        PyErr_Fetch(&ptype, &pvalue, &ptraceback);
        PyErr_NormalizeException(&ptype, &pvalue, &ptraceback);
        char *pStrErrorMessage = PyString_AsString(PyObject_Str(pvalue));;
        
        if (ptype != NULL) {
            errMsg = v8::String::Concat(errMsg, v8::String::New(PyString_AsString(PyObject_Str(ptype))));
        }

        if (pvalue != NULL) {
            errMsg = v8::String::Concat(errMsg, v8::String::New("\nPython Error: "));
            errMsg = v8::String::Concat(errMsg, v8::String::New(PyString_AsString(PyObject_Str(pvalue))));
        }

        if (ptraceback != NULL) {
            errMsg = v8::String::Concat(errMsg, v8::String::New("\n"));
            errMsg = v8::String::Concat(errMsg, v8::String::New(PyString_AsString(PyObject_Str(ptraceback))));
        }

        return ThrowException(
            Exception::Error(String::New(*String::Utf8Value(errMsg)))
        );
    }

    if (!module) {
        return ThrowPythonException();
    }
    Py_XDECREF(module_name);

    return scope.Close(PyObjectWrapper::New(module));
}

void init (Handle<Object> exports) {
    HandleScope scope;
    Py_Initialize();

    PyObjectWrapper::Initialize();

    // how to schedule Py_Finalize(); to be called when process exits?

    // module.exports.import
    exports->Set(
        String::NewSymbol("eval"),
        FunctionTemplate::New(eval)->GetFunction()
    );

    // module.exports.import
    exports->Set(
        String::NewSymbol("import"),
        FunctionTemplate::New(import)->GetFunction()
    );

    // module.exports.PyObject
    exports->Set(
        String::NewSymbol("PyObject"),
        PyObjectWrapper::py_function_template->GetFunction()
    );

}

NODE_MODULE(binding, init)