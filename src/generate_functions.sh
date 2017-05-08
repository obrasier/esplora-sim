#!/bin/bash
# NOTE: There is definitely a better way to do this, but it works.

# ctags get the function names, but it just grabs the line of code that the function
# is declared on. If it spans multiple lines, then this does not work. So.

# Case where it doesn't work:

# unsigned
# int my_func()

# AND

# int
# my_func()

# But those cases also don't work in the Arduino preprocessor

CODE=program.cpp
H_FILE=program.h
touch $H_FILE && rm $H_FILE
echo '#include "Arduino.h"' >> $H_FILE
#                   Get functions                                       ignore setup/loop             remove first 4 fields         remove whitespace
CMD=$(ctags -x --c++-kinds=pf --language-force=c++ program.cpp | egrep -v '^setup\s|^loop\s' | awk '{$1=$2=$3=$4=""; print $0}' | sed -e 's/^[ \t]*//')
IFS=$'\n'
for val in $CMD
do
    # if ( is on the same line as function declaration
    if echo $val | egrep -q "\("; then
        # remove the ( and everything after it
        val=$(echo $val | egrep -o "^.*?\(" | egrep -o "[^\(]*")
    fi
    # remove all newline from code, get the function name until ), append );, and put it in header file
    tr -d '\n' < $CODE | egrep -o "$val[^\)]*" | sed 's/$/\);/' >> $H_FILE
    #put header include into code
    sed -i '1s/^/\#include "program.h"\n/' $CODE
done
