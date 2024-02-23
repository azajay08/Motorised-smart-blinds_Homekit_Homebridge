/*
* Motorised Blinds - ESP32
* Source code to control a DC Motor with an encoder using an ESP32
* It will then connect to a Homebridge server to be controlled by
* Apple HomeKit
*/

#include "config.h"
#include <WiFi.h>
#include <WebServer.h>
#include <PIDController.h>

PIDController pos_pid;
WebServer server(81);
volatile long int encoder_pos = 0;
unsigned int integerValue = 0;  // Max value is 65535
int motor_speed = 0;
char incomingByte;
const int buttonClockwise = 32;
const int buttonCounterClockwise = 33;
const int encoderPin = 21;
const int interruptPin = 22;
const int IN1 = 27;
const int IN2 = 26;
const int minPos = 0; // Starting position
const int maxPos = 14000; // Maximum position
bool setpoint_set = false; // Flag to indicate if setpoint is set

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

void setup() {
  Serial.begin(115200);
  pinMode(interruptPin, INPUT);
  pinMode(encoderPin, INPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(buttonClockwise, INPUT_PULLUP);
  pinMode(buttonCounterClockwise, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), encoder, RISING);

  pos_pid.begin();
  pos_pid.tune(100, 0.5, 10);
  pos_pid.limit(-255, 255);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.on("/setpoint", handleSetpoint);
  server.on("/position", handlePosition);
  server.begin();

}

void loop() {
  server.handleClient();

  if (!setpoint_set) {
    pos_pid.setpoint(encoder_pos); // Set current setpoint
  }

  motor_speed = pos_pid.compute(encoder_pos);

  if (digitalRead(buttonClockwise) == LOW) {
    setpoint_set = false;
    motor_clockwise(255); // Full speed clockwise
  } else if (digitalRead(buttonCounterClockwise) == LOW) {
    setpoint_set = false;
    motor_counter_clockwise(255); // Full speed counter-clockwise
  } else {
    // If neither button is pressed, use PID controller value
    if (motor_speed > 0) {
      motor_counter_clockwise(motor_speed);
    } else {
      motor_clockwise(abs(motor_speed));
    }
  }

  delay(5);
}

void handleSetpoint() {
  Serial.println("setpoint");
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    return;
  }

  if (!server.hasArg("value")) {
    server.send(400, "text/plain", "Bad Request: Missing value");
    return;
  }

  int percent = server.arg("value").toInt();
  int setpoint = map(percent, 0, 100, minPos, maxPos);
  pos_pid.setpoint(setpoint);
  setpoint_set = true;

  server.send(200, "text/plain", "Setpoint updated");
}

void handlePosition() {

  int position = 0;
  if (encoder_pos < minPos) {
	  position = 0;
  } else if (encoder_pos > maxPos) {
	  position = 100;
  }
  else {
	  position = map(encoder_pos, minPos, maxPos, 0, 100);
  }
  String jsonResponse = "{\"position\": " + String(position) + "}";
  server.send(200, "application/json", jsonResponse);
}

void encoder() {
  if (digitalRead(encoderPin) == HIGH) {
    encoder_pos++;
  } else {
    encoder_pos--;
  }
}

void motor_clockwise(int power) {
  if (power > 100) {
    digitalWrite(IN2, HIGH);
    digitalWrite(IN1, LOW);
  } else {
    digitalWrite(IN2, LOW);
    digitalWrite(IN1, LOW);
  }
}

void motor_counter_clockwise(int power) {
  if (power > 100) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
  } else {
    digitalWrite(IN2, LOW);
    digitalWrite(IN1, LOW);
  }
}

