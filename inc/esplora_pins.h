#ifndef ESPLORA_PINS_H_
#define ESPLORA_PINS_H_

#include <inttypes.h>

#define NUM_DIGITAL_PINS  30
#define NUM_ANALOG_INPUTS 12

#define TX_RX_LED_INIT  DDRD |= (1<<5), DDRB |= (1<<0)
#define TXLED0      PORTD |= (1<<5)
#define TXLED1      PORTD &= ~(1<<5)
#define RXLED0      PORTB |= (1<<0)
#define RXLED1      PORTB &= ~(1<<0)

static const uint8_t SDA = 2;
static const uint8_t SCL = 3;

// Map SPI port to 'new' pins D14..D17
static const uint8_t SS   = 17;
static const uint8_t MOSI = 16;
static const uint8_t MISO = 14;
static const uint8_t SCK  = 15;

// Mapping of analog pins as digital I/O
// A6-A11 share with digital pins
static const uint8_t A0 = 18;
static const uint8_t A1 = 19;
static const uint8_t A2 = 20;
static const uint8_t A3 = 21;
static const uint8_t A4 = 22;
static const uint8_t A5 = 23;
static const uint8_t A6 = 24; // D4
static const uint8_t A7 = 25; // D6
static const uint8_t A8 = 26; // D8
static const uint8_t A9 = 27; // D9
static const uint8_t A10 = 28;  // D10
static const uint8_t A11 = 29;  // D12

#endif