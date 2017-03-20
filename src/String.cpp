#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "String.h"
#include "Arduino.h"

// conversion (and default) constructor converts char * to String
String::String( const char *s )
  : lngth( ( s != 0 ) ? strlen( s ) : 0 )
{
//  increment_counter(100);
  //std::cout << "Conversion (and default) constructor: " << s << std::endl;
  setString( s ); // call utility function
}

// copy constructor
String::String( const String &copy )
  : lngth( copy.lngth )
{
//  increment_counter(100);
  //std::cout << "Copy constructor: " << copy.sPtr << std::endl;
  setString( copy.sPtr ); // call utility function
}

// Destructor
String::~String()
{
  //std::cout << "Destructor: " << sPtr << std::endl;
  delete [] sPtr; // release pointer-based string memory
}

// overloaded = operator; avoids self assignment
const String &String::operator=( const String &right )
{
  //std::cout << "operator= called" << std::endl;

  if ( &right != this ) // avoid self assignment
  {
    delete [] sPtr; // prevents memory leak
    lngth = right.lngth; // new String length
    setString( right.sPtr ); // call utility function
  } // end if
  else
    std::cout << "Attempted assignment of a String to itself" << std::endl;

  return *this;
}

// concatenate right operand to this object and store in this object
const String &String::operator+=( const String &right )
{
  size_t newLength = lngth + right.lngth; // new length
  char *tempPtr = new char[ newLength + 1 ]; // create memory

  strcpy( tempPtr, sPtr ); // copy sPtr
  strcpy( tempPtr + lngth, right.sPtr ); // copy right.sPtr

  delete [] sPtr; // reclaim old space
  sPtr = tempPtr; // assign new array to sPtr
  lngth = newLength; // assign new length to length
  return *this; // enables cascaded calls
}

// is this String empty?
bool String::operator!() const
{
  return lngth == 0;
}

// Is this String equal to right String?
bool String::operator==( const String &right ) const
{
  return strcmp( sPtr, right.sPtr ) == 0;
}

// Is this String less than right String?
bool String::operator<( const String &right ) const
{
  return strcmp( sPtr, right.sPtr ) < 0;
}

// return reference to character in String as a modifiable lvalue
char &String::operator[]( int subscript )
{
  // test for subscript out of range
  if ( subscript < 0 || subscript >= lngth )
  {
    std::cerr << "Error: Subscript " << subscript
         << " out of range" << std::endl;
    exit( 1 ); // terminate program
  } // end if

  return sPtr[ subscript ]; // non-const  modifiable lvalue
}

// return reference to character in String as rvalue
char String::operator[]( int subscript ) const
{
  // test for subscript out of range
  if ( subscript < 0 || subscript >= lngth )
  {
    std::cerr << "Error: Subscript " << subscript
         << " out of range" << std::endl;
    exit( 1 ); // terminate program
  } // end if

  return sPtr[ subscript ]; // returns copy of this element
}

// return a substring beginning at index and of length subLength
String String::operator()( int index, int subLength ) const
{
  // if index is out of range or substring length < 0,
  // return an empty String object
  if ( index < 0 || index >= lngth || subLength < 0 )
    return ""; // converted to a String object automatically

  // determine length of substring
  int len;

  if ( ( subLength == 0 ) || ( index + subLength > lngth ) )
    len = lngth - index;
  else
    len = subLength;

  // allocate temporary array for substring and
  // terminating null character
  char *tempPtr = new char[ len + 1 ];

  // copy substring into char array and terminate string
  strncpy( tempPtr, &sPtr[ index ], len );
  tempPtr[ len ] = '\0';

  // create temporary String object containing the substring
  String tempString( tempPtr );
  delete [] tempPtr;
  return tempString;
}

int String::getLength() const
{
  return lngth;
}

void String::setString( const char *string2 )
{
  sPtr = new char[ lngth + 1 ];

  if ( string2 != 0 )
    strcpy( sPtr, string2 );
  else
    sPtr[ 0 ] = '\0';
}

std::ostream &operator<<(std::ostream &output, const String &s )
{
  output << s.sPtr;
  return output;
}

std::istream &operator>>(std::istream &input, String &s )
{
  char temp[ 100 ];
  input >> std::setw( 100 ) >> temp;
  s = temp;
  return input;
}

String String::operator+(String right )
{
  String temp;
  size_t newLength = lngth + right.lngth;
  char *tempPtr = new char[ newLength + 1 ];
  strcpy( tempPtr, sPtr );
  strcpy( tempPtr + lngth, right.sPtr );
  temp.sPtr = tempPtr;
  temp.lngth = newLength;
  return temp;
}

String String::operator+( const int number )
{
  String temp;
  std::stringstream ss;
  String right;
  ss << number;
  right = ss.str();
  size_t newLength = lngth + right.lngth;
  char *tempPtr = new char[ newLength + 1 ];
  strcpy( tempPtr, sPtr );
  strcpy( tempPtr + lngth, right.sPtr );
  temp.sPtr = tempPtr;
  temp.lngth = newLength;
  return temp;
}

String String::operator+( unsigned long number )
{
  String temp;
  std::stringstream ss;
  String right;
  ss << number;
  right = ss.str();
  size_t newLength = lngth + right.lngth;
  char *tempPtr = new char[ newLength + 1 ];
  strcpy( tempPtr, sPtr );
  strcpy( tempPtr + lngth, right.sPtr );
  temp.sPtr = tempPtr;
  temp.lngth = newLength;
  return temp;
}

String String::operator+( const char one )
{
  String temp;
  std::stringstream ss;
  String right;
  ss << one;
  right = ss.str();
  size_t newLength = lngth + right.lngth;
  char *tempPtr = new char[ newLength + 1 ];
  strcpy( tempPtr, sPtr );
  strcpy( tempPtr + lngth, right.sPtr );
  temp.sPtr = tempPtr;
  temp.lngth = newLength;
  return temp;
}


const String &String::operator=( std::string right )
{
  char *p;
  lngth = right.length();
  p = (char *)(right.c_str());
  setString(p);
  return *this;
}

const String &String::operator=(const char *right )
{
  const char *p;
  lngth = strlen(right);
  p = right;
  setString(p);
  return *this;
}


String::String( const int x )
{
  char p[10];
  sprintf(p, "%d", x);
  lngth = strlen(p);
  setString(p);
}

String::String( const int x, const int base )
{
  char p[10];
  if (base == BIN)sprintf(p, "BIN%d", x);
  if (base == DEC)sprintf(p, "DEC%d", x);
  if (base == HEX)sprintf(p, "HEX%d", x);
  lngth = strlen(p);
  setString(p);
}

char String::charAt( const int n )
{
  char res;
  res = sPtr[n];
  return (res);
}

int String::compareTo( const String s )
{
  int res;
  if (*this > s)res = -1;
  if (*this < s)res = 1;
  if (*this == s)res = 0;
  return (res);
}

String String::concat( const String s )
{
  *this += s;
  return *this;
}

bool String::endsWith( const String s )
{
  bool res;
  int pos;
  std::string p(sPtr);
  pos = lngth - s.lngth;
  res = p.compare(pos, s.lngth, s.sPtr);
  return (res);
}

bool String::equals( const String s )
{
  return strcmp( sPtr, s.sPtr ) == 0;
}

bool String::equalsIgnoreCase( const String s )
{
  // Not implemented
  return strcmp( sPtr, s.sPtr ) == 0;
}

void String::getBytes( int buf[], int *len )
{
  int i;
  char *p;

  p = sPtr;
  for (i = 0; i < lngth; i++)
  {
    buf[i] = (int) * p; // Issue 14
    p++;
  }
  *len = lngth;

}

int String::indexOf(char val)
{
  // Not implemented
  return (-1);
}

int String::indexOf(String val)
{
  // Not implemented
  return (-1);
}

int String::indexOf(char val, int from)
{
  // Not implemented
  return (-1);
}

int String::indexOf(String val, int from)
{
  // Not implemented
  return (-1);
}

int String::lastIndexOf(char val)
{
  // Not implemented
  return (-1);
}

int String::lastIndexOf(String val)
{
  // Not implemented
  return (-1);
}

int String::lastIndexOf(char val, int from)
{
  // Not implemented
  return (-1);
}

int String::lastIndexOf(String val, int from)
{
  // Not implemented
  return (-1);
}

int String::length()
{
  return (lngth);
}

String String::replace(String sub1, String sub2)
{
  // Not implemented
  String res;
  return (res);
}

void String::setCharAt( int index, char c )
{
  // Not implemented

}

bool String::startsWith( String s )
{
  bool res;
  // Not implemented
  return (res);
}

String String::substring(int from)
{
  String res;
  // Not implemented
  return (res);
}

String String::substring(int from, int to)
{
  String res;
  // Not implemented
  return (res);
}

void String::toCharArray(char buf[], int *len)
{
  // Not implemented

}

void String::toLowerCase()
{
  // Not implemented

}

void String::toUpperCase()
{
  // Not implemented

}

void String::trim()
{
  // Not implemented

}

char* String::getPointer()
{
  return (sPtr);
}