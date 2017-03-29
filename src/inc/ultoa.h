#ifndef ULTOA_H_
#define ULTOA_H_

char* ultoa( unsigned long __val, char* __s, int __radix );
char* ltoa( long __val, char* __s, int __radix );
char* itoa( int __val, char* __s, int __radix );
char* utoa( unsigned int __val, char* __s, int __radix );

#endif
