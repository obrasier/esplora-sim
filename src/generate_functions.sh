#!/bin/bash
# NOTE: There is definitely a better way to do this, but it works.

# ctags get the function names, but it just grabs the line of code that the function
# is declared on. If it spans multiple lines, then this does not work. So.

# We get all the functions 
# Ignore setup and loop
# Get the last argument
# Remove whitespace

# Then we loop through all the results
# for most cases which includes ( on the same line
#   find everything up to it and remove the ( to get the type and function name
# otherwise
# delete all newline characters and regex until ) then append ;

# Case where it doesn't work:
# unsigned
# int my_func()
# AND
# int
# my_func()
# But that also doesn't work in Arduino

CODE=program.cpp
H_FILE=program.h
touch $H_FILE && rm $H_FILE
echo "#include <stdint.h>" >> $H_FILE
#CMD=$(ctags -x --c++-kinds=pf --language-force=c++ program.cpp | egrep -v '^setup\s|^loop\s' | awk '{$1=$2=$3=$4=""; print $0}' | sed -e 's/^[ \t]*//' | egrep -o "^.*?\(|^.*?" | egrep -o "[^\(]*")
CMD=$(ctags -x --c++-kinds=pf --language-force=c++ program.cpp | egrep -v '^setup\s|^loop\s' | awk '{$1=$2=$3=$4=""; print $0}' | sed -e 's/^[ \t]*//')
IFS=$'\n'
for val in $CMD
do
    if echo $val | egrep -q "\("; then
        val=$(echo $val | egrep -o "^.*?\(" | egrep -o "[^\(]*")
    fi
    tr -d '\n' < $CODE | grep -oE "$val[^\)]*" | sed 's/$/\);/' >> $H_FILE
    sed -i '1s/^/\#include "program.h"\n/' $CODE
done
