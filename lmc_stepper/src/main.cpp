#include <Arduino.h>

const int in1 =  8;
const int in2 =  9;
const int in3 = 10;
const int in4 = 11;

// example input: d:1,s:3,t:1000#
// d = direction -1/1. 0 = stop
// s = speed 1 fastest = delay in ms between steps
// t = time of move in ms

// d:-1,s:1,t:15000#
// d:-1,s:5,t:500#
// d:0#

const unsigned char steps[] = {0x01,0x03,0x02,0x06,0x04,0x0c,0x08,0x09};
typedef struct {
  int stepIndex;
  int direction;
  unsigned long startMs;
  int timeMs;
  int tickCount;
  int speedDelay;
} axis;

axis xAxis = {0, 0, 0, 0, 0, 0};

const byte numChars = 32;
char receivedChars[numChars]; // an array to store the received data
int inputIndex = 0;

void setup() {
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  Serial.begin(9600);
  Serial.println("<Arduino is ready>");
}

void logAxis(axis *a) {
  Serial.print("stepIndex: ");
  Serial.print(a->stepIndex);
  Serial.print(", direction: ");
  Serial.print(a->direction);
  Serial.print(", startMs: ");
  Serial.print(a->startMs);
  Serial.print(", timeMs: ");
  Serial.print(a->timeMs);
  Serial.print(", tickCount: ");
  Serial.print(a->tickCount);
  Serial.print(", speedDelay: ");
  Serial.println(a->speedDelay);
}

void checkAxis(axis *aPtr) {
  if (aPtr->direction != 0) {
    if (aPtr->startMs != 0 && millis() > aPtr->startMs + aPtr->timeMs) {
      // ending move
      *aPtr = {0, 0, 0, 0, 0, 0};
    } else {
      if (aPtr->startMs == 0) {
        // starting move
        //Serial.println("Starting");
        aPtr->startMs = millis();
        aPtr->tickCount = aPtr->speedDelay;
      }
      if (aPtr->tickCount >= aPtr->speedDelay) {
        // do the step
        digitalWrite(in1, steps[aPtr->stepIndex] & 0x01);
        digitalWrite(in2, steps[aPtr->stepIndex] & 0x02 ? HIGH : LOW);
        digitalWrite(in3, steps[aPtr->stepIndex] & 0x04 ? HIGH : LOW);
        digitalWrite(in4, steps[aPtr->stepIndex] & 0x08 ? HIGH : LOW);
        aPtr->stepIndex += (aPtr->direction > 0) ? 1 : -1;
        if (aPtr->stepIndex < 0) aPtr->stepIndex = 7;
        if (aPtr->stepIndex >= 8) aPtr->stepIndex = 0;
        aPtr->tickCount = 0;
        //logAxis(aPtr);
      } else {
        aPtr->tickCount++;
      }
    }
  }
}

void parseMessageItem(char *item, axis *axis) {
  if (item[1] == ':') {
    int value = atoi(item+2);
    switch (item[0])
    {
      case 'd':
        Serial.print("Setting direction to: ");
        Serial.println(value);
        axis->direction = value;
        break;
      case 's':
        Serial.print("Setting speed to: ");
        Serial.println(value);
        axis->speedDelay = value;
        break;
      case 't':
        Serial.print("Setting time to: ");
        Serial.println(value);
        axis->timeMs = value;
        break;
      default:
        Serial.print("Unexpected message part type. Expected d (direction)/s (speed)/t (time) but I got: ");
        Serial.println(item[0]);
        break;
    }
  } else {
    Serial.print("Can't parse message part. Second character should be ':' but I got: ");
    Serial.println(item);
  }
}

void parseMessage() {
  receivedChars[inputIndex] = '\0';
  Serial.print("Received: ");
  Serial.println(receivedChars);
  char *token;
  token = strtok(receivedChars, ",");
  while( token != NULL ) {
      parseMessageItem(token, &xAxis);
      token = strtok(NULL, ",");
  }
  inputIndex = 0;
  xAxis.startMs = 0;
//  xAxis.direction = 1;
//  xAxis.speedDelay = 1;
//  xAxis.timeMs = 2000;
}

void checkSerialInput() {
  char rc;
  while (Serial.available() > 0 && inputIndex < sizeof(receivedChars)) {
    rc = Serial.read();
    receivedChars[inputIndex] = rc;
    if (rc == '#') {
      parseMessage();
    } else {
      inputIndex++;
    }
  }
}

void loop() {
  checkSerialInput();
  checkAxis(&xAxis);
  delay(1);
}