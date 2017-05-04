from shutil import copyfile
import fileinput
import re

regex_map = {r'\bunsigned int\b': r'uint16_t',
             r'\bunsigned long\b': r'uint32_t',
             #if int not preceded by unsigned , replace with int16_t
             r'(?<!\bunsigned\s)(\bint\b)': 'int16_t', 
             #if long not preceded by unsigned , replace with int32_t
             r'(?<!\bunsigned\s)(\blong\b)': 'int32_t',
             r'\bdouble\b': r'float'}


copyfile('sketch/sketch.ino', 'program.cpp')
for line in fileinput.input("program.cpp", inplace=True):
  for k, v in regex_map.items():
    line = re.sub(k, v, line.rstrip())
  print(line)

