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
});
