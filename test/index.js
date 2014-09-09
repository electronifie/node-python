var python = require('../');
var PythonError = python.PythonError;
var should = require('should');

describe('node-python', function () {
  describe('eval', function () {
    it('should return resulting value from python statement executed', function () {
      var value = python.eval('"1"');
      value.should.equal("1");
    });
    it('should return resulting value from python statement executed, converting to string with complex types', function () {
      var decimal = python.import('decimal');
      var smallNum = decimal.Decimal('0.0000000001');
      smallNum.toString().should.equal('1E-10');
    });
  });
  describe('import', function () {
    it('should return object representing module imported, containing functions from imported module', function () {
      var value = python.import('decimal');
      value.should.have.property('valueOf');
    });
    it('should throw a PythonError when importing a module that does not exist', function () {
      should(function () {
        python.import('jibberish');
      }).throw(/No module named jibberish/);
    });
    it('should throw an Error when importing a module that includes bad syntax', function () {
    should(function () {
        python.import('test');
    }).throw(/Python Error: SyntaxError/)
    });
  });
  it('should convert javascript booleans to python booleans', function () {
    test = python.import('test2');
    var type = test.getPythonTypeName(true);
    type.should.equal('bool');
    var value = test.getPythonValue(true);
    value.should.equal(true);
  });
  it('should convert javascript date to python date', function () {
    test = python.import('test2');
    var now = new Date();
    var type = test.getPythonTypeName(now);
    type.should.equal('datetime');
    var value = test.getPythonValue(now);
    value.total_seconds().should.equal(now.timtestamp());
  });
  it('should convert javascript numbers to python floats', function () {
    test = python.import('test2');
    var type = test.getPythonTypeName(1);
    type.should.equal('float');
  });
  it('should convert javascript arrays to python list', function () {
    test = python.import('test2');
    var type = test.getPythonTypeName([]);
    type.should.equal('list');
  });
  it('should convert javascript objects to python dictionaries', function () {
    test = python.import('test2');
    var type = test.getPythonTypeName({});
    type.should.equal('dict');
  });
  it('should convert javascript nested objects correctly', function () {
    test = python.import('test2');
    var type = test.getPythonTypeName2({
      value: 1
    }, 'value');
    type.should.equal('float');
    var type = test.getPythonTypeName2({
      value: true
    }, 'value');
    type.should.equal('bool');
    var type = test.getPythonTypeName2({
      value: new Date()
    }, 'value');
    type.should.equal('datetime');
    var type = test.getPythonTypeName2({
      value: {}
    }, 'value');
    type.should.equal('dict');
    var type = test.getPythonTypeName2({
      value: ['one', 'two', 'three']
    }, 'value');
    type.should.equal('list');
    var i = 0, arr = [];
    while (i < 10000) {
      arr.push(Math.random().toString())
      i++;
    }
    var type = test.getPythonTypeName(arr);
    type.should.equal('list');
  });
});