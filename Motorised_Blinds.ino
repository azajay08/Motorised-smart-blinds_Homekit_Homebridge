/*
* Motorised Blinds - ESP32
* Source code to control a DC Motor with an encoder using an ESP32
* It will then connect to a Homebridge server to be controlled by
* Apple HomeKit
*/

#include "config.h"
#include <PIDController.h>

PIDController pos_pid; 
volatile long int encoder_pos = 0;
unsigned int integerValue=0;  // Max value is 65535
int motor_speed = 255;
char incomingByte;

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
