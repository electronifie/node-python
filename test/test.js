var python = require('../');
var PythonError = python.PythonError;
var should = require('should');

function run () {
  test = python.import('test2');
  var date = new Date();
  console.log(' js timestamp before:', date.getTime());
  var value = test.getPythonValue(date);
  console.log(date, date.getHours(), value);
  console.log(' js timestamp after:', value.getTime());
  value.should.eql(date);
}

setInterval(run, 100);
