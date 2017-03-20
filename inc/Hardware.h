#ifndef HARDWARE_H_
#define HARDWARE_H_

#define SIM_SWITCH_1      1
#define SIM_SWITCH_2      2
#define SIM_SWITCH_3      3
#define SIM_SWITCH_4      4
#define SIM_SLIDER        7
#define SIM_LIGHT         8
#define SIM_TEMPERATURE   11
#define SIM_MIC           12
#define SIM_TINKERKIT_INA   13  // tinkerkit input
#define SIM_TINKERKIT_INB   14  // tinkerkit input
#define SIM_JOYSTICK_SW   15
#define SIM_JOYSTICK_X    16
#define SIM_JOYSTICK_Y    17
#define SIM_RED           5
#define SIM_GREEN         9
#define SIM_BLUE          10
#define SIM_ACCEL_X       23
#define SIM_ACCEL_Y       29
#define SIM_ACCEL_Z       24
#define SIM_BUZZER       6 

#define MAX_TOTAL_PINS 30

void check_suspend();
void check_shutdown();
void increment_counter(int us);

// in Main.cpp
void send_pin_update();
void send_led_update();

#endif