var binding = require('bindings')('binding.node');
var debug = require('debug')('node-python');
var util = require('util');

function PythonError (message, value) {
  if (arguments[0] instanceof Error) {
    var error = arguments[0];
    this.message = error.message;
    this.stack = error.stack;
    this.value = error.value;
    Error.apply(this, arguments);
  } else {
    Error.call(this, message, value);
  }
}

util.inherits(PythonError, Error);

module.exports.PythonError = PythonError;

module.exports.eval = function (string) {
  return binding.eval(string);
}

var _import = module.exports.import = function (string) {
var result = null;

try {
  result = binding.import(string);
} catch (e) {
  e = new PythonError(e);
  debug(e);
  throw e;
}

return result;
}

var os = _import('os');

var pythonPath = os.environ.get('PYTHONPATH');

if (pythonPath === undefined) {
  debug('WARNING: PYTHONPATH environment variable is undefined. This may cause problems finding python modules. see: https://docs.python.org/2/tutorial/modules.html#the-module-search-path');
}