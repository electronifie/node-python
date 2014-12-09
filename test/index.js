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
      }).throw(/Python Error: SyntaxError/);
    });
  });
  it('should convert javascript null to python NoneType', function () {
    test = python.import('test2');
    var type = test.getPythonTypeName(null);
    type.should.equal('NoneType');
  });
  it('should convert javascript undefined to python NoneType', function () {
    test = python.import('test2');
    var type = test.getPythonTypeName(undefined);
    type.should.equal('NoneType');
  });
  it('should convert javascript booleans to python booleans', function () {
    test = python.import('test2');
    var type = test.getPythonTypeName(true);
    type.should.equal('bool');
  });
  it('should convert javascript date to python date', function () {
    test = python.import('test2');
    var type = test.getPythonTypeName(new Date());
    type.should.equal('datetime');
  });
  it('should convert javascript numbers to python floats', function () {
    test = python.import('test2');
    var type = test.getPythonTypeName(1.1);
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
      arr.push(Math.random().toString());
      i++;
    }
    var type = test.getPythonTypeName(arr);
    type.should.equal('list');
  });
  it('should convert python dicts to javascript objects', function () {
    test = python.import('test2');
    var value = test.getPythonValue({
      value: 1
    });
    value.should.have.property('value', 1);
  });
  it('should convert python lists to javascript arrays', function () {
    test = python.import('test2');
    var value = test.getPythonValue([ '1', 2, '3']);
    value.should.containEql('1');
    value.should.containEql(2);
    value.should.containEql('3');
  });
  it('should convert python strings to javascript strings', function () {
    test = python.import('test2');
    var value = test.getPythonValue('str');
    value.should.equal('str');
  });
  it('should convert python boolean to javascript booleans', function () {
    test = python.import('test2');
    var value = test.getPythonValue(true);
    value.should.equal(true);
  });
  it('should convert python numbers to javascript numbers', function () {
    test = python.import('test2');
    var value = test.getPythonValue(1);
    value.should.equal(1);
  });
  it('should convert python dates to javascript dates', function () {
    test = python.import('test2');
    var date = new Date();
    var value = test.getPythonValue(date);
    value.should.eql(date);
  });
  it('should convert basic python dict to javascript object', function () {
    test = python.import('test3');
    var obj = test.getPythonDict();
    obj.should.have.property('one', 1);
    obj.should.have.property('two', '2');
    obj.should.have.property('three', 3);
  });
  it('should convert nested python dict to javascript object', function () {
    
    test = python.import('test3');

    var dict = test.getPythonNestedDict;

    for(var i = 0; i < 100000; i++) {
      var obj = dict();
      obj.should.have.property('one');
      obj.one.should.have.property('four', 4);
      obj.one.should.have.property('five', 5);
      obj.should.have.property('two', '2');
      obj.should.have.property('three', 3);
    }

  });
  it('should convert basic python list to javascript array', function () {
    test = python.import('test3');
    var obj = test.getPythonList();
    obj.should.have.containEql(1);
    obj.should.have.containEql('2');
    obj.should.have.containEql('three');
  });
  it('should convert nested python list to javascript array', function () {
    test = python.import('test3');
    var obj = test.getPythonNestedList();
    obj.should.have.containEql([4, 5, 6]);
    obj.should.have.containEql('2');
    obj.should.have.containEql('three');
  });
});
