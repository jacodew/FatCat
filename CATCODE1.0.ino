#include <Wire.h>
#include "RTClib.h"

RTC_DS3231 rtc;

#define STEP 3
#define DIR 2
#define BUZZER 9

int stepDelay = 400;        // microseconds per step (adjust speed)
int quarterTurnSteps = 50;  // 200 steps/revolution ÷ 4

// Feeding time (24-hour format)
int targetHour = 17;
int targetMinute = 48;

bool fedToday = false;      // prevents multiple triggers in same minute
unsigned long lastPrint = 0;

void setup() {
  pinMode(STEP, OUTPUT);
  pinMode(DIR, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  Serial.begin(9600);
  Wire.begin(); // SDA = A4, SCL = A5 on Nano

  if(!rtc.begin()){
    Serial.println("RTC not found");
    while(1); // stop here if RTC fails
  }

  // ONE-TIME RTC SETTING:
  // Set the RTC to the compile time once, then comment out this line in future uploads
 // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void loop() {
  DateTime now = rtc.now();

  // Print current time once per second
  if(millis() - lastPrint >= 1000){
    lastPrint = millis();
    Serial.print("Current Time: ");
    if(now.hour() < 10) Serial.print('0');
    Serial.print(now.hour());
    Serial.print(':');
    if(now.minute() < 10) Serial.print('0');
    Serial.print(now.minute());
    Serial.print(':');
    if(now.second() < 10) Serial.print('0');
    Serial.println(now.second());
  }

  // Check feeding time
  if(now.hour() == targetHour && now.minute() == targetMinute && !fedToday){
    Serial.println("Feeding time! Activating buzzer and motor...");

    // Buzzer alert: 1 second beep
    digitalWrite(BUZZER, HIGH);
    delay(1000);
    digitalWrite(BUZZER, LOW);

    // Move stepper ¼ turn forward
    digitalWrite(DIR, HIGH);
    moveSteps(quarterTurnSteps);

    delay(500); // short pause

    // Move stepper ¼ turn back
    digitalWrite(DIR, LOW);
    moveSteps(quarterTurnSteps);

    fedToday = true;
    Serial.println("Feeding complete!");
  }

  // Reset flag when minute changes so it triggers next day
  if(now.minute() != targetMinute){
    fedToday = false;
  }
}

// Moves the stepper a given number of steps
void moveSteps(int steps){
  for(int i = 0; i < steps; i++){
    digitalWrite(STEP, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(STEP, LOW);
    delayMicroseconds(stepDelay);
  }
}