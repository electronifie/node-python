# node-python binding 

python bridge for nodejs!

Hyper-beta, don't hesitate to try and report bugs!

[![Build Status](https://travis-ci.org/JeanSebTr/node-python.png)](https://travis-ci.org/JeanSebTr/node-python)

## Installation

```npm install node-python```

## Usage

```javascript

// python stuff
var python = require('node-python');
var os = python.import('os');

// nodejs stuff
var path = require('path');

assert(os.path.basename(os.getcwd()) == path.basename(process.cwd()))

```

You should now go have fun with that and make it brokes :)

## Current status

What works:

 - (per unit tests)

 * node-python
 *   ✓ should convert javascript null to python NoneType
 *   ✓ should convert javascript undefined to python NoneType
 *   ✓ should convert javascript booleans to python booleans
 *   ✓ should convert javascript date to python date
 *   ✓ should convert javascript numbers to python floats
 *   ✓ should convert javascript arrays to python list
 *   ✓ should convert javascript objects to python dictionaries
 *   ✓ should convert javascript nested objects correctly
 *   ✓ should convert python dicts to javascript objects
 *   ✓ should convert python lists to javascript arrays
 *   ✓ should convert python strings to javascript strings
 *   ✓ should convert python boolean to javascript booleans
 *   ✓ should convert python numbers to javascript numbers
 *   ✓ should convert python dates to javascript dates
 *   ✓ should convert basic python dict to javascript object
 *   ✓ should convert basic python list to javascript array
 *   eval
 *     ✓ should return resulting value from python statement executed
 *     ✓ should return resulting value from python statement executed, converting to string with complex types
 *   import
 *     ✓ should return object representing module imported, containing functions from imported module
 *     ✓ should throw a PythonError when importing a module that does not exist
 *     ✓ should throw an Error when importing a module that includes bad syntax

What to watch out for:

* Losing precision from Python's 64 bits Integer to Node's Number (we hande this by converting decimals to string and using bignumber.js to consume them. better solutions to come)