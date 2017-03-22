#include <iostream>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include "Arduino.h"
#include "global_variables.h"

void pinMode(uint8_t pin, int mode)
{
//  increment_counter(20);
  if (mode == INPUT)
    std::cout << "doing input pinmode" << std::endl;
  else if (mode == OUTPUT)
    std::cout << "doing output pinmode" << std::endl;
  else if (mode == INPUT_PULLUP)
    std::cout << "doing output pinmode" << std::endl;

}
void digitalWrite(uint8_t pin, int value)
{
//  increment_counter(20);
  if (value == 0)
    std::cout << "doing digitalWrite low" << std::endl;
  if (value == 1)
    std::cout << "doing digitalWrite high" << std::endl;

}
int digitalRead(uint8_t pin)
{
//  increment_counter(20);
  return -9;
}

void analogWrite(uint8_t pin, int value) //PWM
{
//  increment_counter(20);

}

int analogRead(uint8_t pin)
{
//  increment_counter(20);
  std::cout << "doing analogRead" << std::endl;
  return 0;
}

//------ Advanced I/O ----------------------
void tone(uint8_t pin, unsigned int freq)
{
  std::cout << "called tone" << std::endl;
}

void tone(uint8_t pin, unsigned int freq, unsigned long duration)
{
}

void noTone(uint8_t pin)
{
  std::cout << "called noTone" << std::endl;
}

void shiftOut(int dataPin, int clockPin, int bitOrder, int value)
{
  //bitOrder: which order to shift out the bits; either MSBFIRST or LSBFIRST.
}

int shiftIn(int dataPin, int clockPin, int bitOrder)
{
  //bitOrder: which order to shift out the bits; either MSBFIRST or LSBFIRST.
  return -9;
}

unsigned long pulseIn(uint8_t pin, int value)
{
  return -9;
}

unsigned long pulseIn(uint8_t pin, int value, unsigned long timeout)
{
  return -9;
}

//------ Time ------------------------------

unsigned long millis()
{
//  increment_counter(1);
  m_elapsed.lock();
  unsigned long e = micros_elapsed;
  m_elapsed.unlock();
  return (int)e / 1000;
}

// Return microseconds rounded down
// to the nearest multiple of 4
unsigned long
micros()
{
  m_elapsed.lock();
  unsigned long e = micros_elapsed;
  m_elapsed.unlock();
  int rem = e % 4;
  if (rem == 0)
    return e;
  return e - rem;
}

void delay(uint32_t ms)
{
  increment_counter(ms * 1000);
}

void delayMicroseconds(uint32_t us)
{
  increment_counter(us);
}

int map(int x, int fromLow, int fromHigh, int toLow, int toHigh)
{
//  increment_counter(100);
  int y;
  y = (float)(x - fromLow) / (fromHigh - fromLow) * (toHigh - toLow) + toLow;
  return (y);
}


//------ Random Numbers --------------------
void randomSeed(int seed)
{
//  increment_counter(100);
  srand(seed);
}

long random(long upperLimit)
{
//  increment_counter(100);
  long x = RAND_MAX / upperLimit;
  x = long(rand() / x);
  return (x);
}

long random(long lowerLimit, long upperLimit)
{
//  increment_counter(100);
  long interval, temp = 0;
  if (lowerLimit < upperLimit)
  {
    interval = upperLimit - lowerLimit;
    temp = lowerLimit + random(interval);
  }
  return (temp);
}


//------ Bits and Bytes --------------------
/*
unsigned char lowByte(int x)
{
//  increment_counter(100);
  return (x & 0xff);
}

unsigned char highByte(int x)
{
//  increment_counter(100);
  unsigned char y;
  x = x & 0xff00;
  y = x >> 8;
  return (y);
}

int bitRead(int x, int n)
{
//  increment_counter(100);
  int bit;
  //printf("bitRead: x=%d n=%d PORTB=%d\n",x,n,PORTB);
  bit = x >> n;
  bit = bit & 0x0001;
  //printf("bitRead: x=%d n=%d PORTB=%d bit=%d\n",x,n,PORTB,bit);
  return (bit);
}

void  bitSet(unsigned int *x, int n)
{
//  increment_counter(100);
  int res, mask;

  mask = 1 << n;
  *x  = *x | mask;
}

void bitClear(unsigned int *x, int n)
{
//  increment_counter(100);
  int res, mask;

  mask = 1 << n;
  *x  = *x & ~mask;
}

void bitWrite(unsigned int *x, int n, int b)
{
//  increment_counter(100);
  //printf("bitWrite: %d %d %d PORTB=%d\n",*x,n,b,PORTB);
  if (b == 0)bitClear(x, n);
  if (b == 1)bitSet(x, n);

}

int bit(int n)
{
  int res;
  res = 1 << n;
  return (res);
}
*/

//------ External Interrupts ---------------


// void attachInterrupt(int ir, void(*func)(), int mode)
// {
// //  increment_counter(100);
//   uint8_t pin, ok = S_NOK;

//   // ok = checkRange(S_OK, "interrupt", ir);

//   if (ok == S_OK)
//   {

//     pin                 = inrpt[ir];

//     if (mode == LOW)
//       std::cout << "attach interrupt low" << std::endl;
//     if (mode == RISING)
//       std::cout << "attach interrupt rising" << std::endl;
//     if (mode == FALLING)
//       std::cout << "attach interrupt falling" << std::endl;
//     if (mode == CHANGE)
//       std::cout << "attach interrupt change" << std::endl;

//     interruptMode[ir] = mode;
//     attached[ir]      = S_YES;
//     interrupt[ir]     = func;

//     //interrupt[ir](); ????
//     g_attachedPin[pin]  = S_YES;
//     g_interruptType[pin] = mode;

//     digitalMode[pin]  = mode;
//   }
//   else
//   {
//     errorLog("attachInterruptERROR", ir);
//   }
// }


// //---------------------------------------------------
// void detachInterrupt(int ir)
// {
// //  increment_counter(100);
//   int ok = S_NOK, pin;


//   // ok = checkRange(S_OK, "interrupt", ir);
//   if (ok == S_OK)
//   {
//     //interrupt[ir]     = NULL;
//     attached[ir]      = S_NO;
//     pin               = inrpt[ir];
//     g_attachedPin[pin] = S_NO;
//     digitalMode[pin]  = INPUT;
//   }
// }
//------ Interrupts ------------------------


