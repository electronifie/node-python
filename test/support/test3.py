from decimal import *

def getPythonDict():
  return dict(one=1, two=str(Decimal(2)), three=3)

def getPythonNestedDict():
  return dict(one=dict(four=4, five=5), two=str(Decimal(2)), three=3)

def getPythonList():
  return [1, str(Decimal(2)), 'three']

def getPythonNestedList():
  return [[4, 5, 6], str(Decimal(2)), 'three']