#!/bin/bash
arduino --verify --board arduino:avr:esplora src/sketch/sketch.ino 
if [ $? -eq 0 ]
then
  echo "build successful"
  cd src
  python preprocessor.py
  bash generate_functions.sh
  cd ..
  make
fi


