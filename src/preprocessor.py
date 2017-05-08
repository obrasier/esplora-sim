from shutil import copyfile
import fileinput
import re

regex_map = {r'\bunsigned\sint\b': r'uint16_t',
             r'\bunsigned\slong\b': r'uint32_t',
             #if int not preceded by unsigned , replace with int16_t
             r'(?<!\bunsigned\b)(\bint\b)': r'int16_t', 
             #if long not preceded by unsigned , replace with int32_t
             r'(?<!\bunsigned\b)(\blong\b)': r'int32_t',
             r'\bdouble\b': r'float'}


# copyfile('sketch/sketch.ino', 'program.cpp')
# for line in fileinput.input("program.cpp", inplace=True):
#   #line = re.sub("\s\s+", " ", line)
#   for k, v in regex_map.items():
#     line = re.sub(k, v, line.rstrip())
#   print(line)


with open('sketch/sketch.ino', 'r') as f:
  for line in f:
    llist = re.split(r'(\s+)', line.rstrip())
    white = ''
    if llist[0] == '' and len(llist) > 2:
      # we start with a blank line, want to preserve whitespace
      white = llist[1]
      for idx, w in enumerate(llist):
        if idx > 2 and w.isspace() and (len(w) > 1 or w == '\t'):
          llist[idx] = " "
    else:
      for idx, w in enumerate(llist):
        if w.isspace() and (len(w) > 1 or w == '\t'):
          llist[idx] = " "
    new_line = white + "".join(llist)
    print(new_line)