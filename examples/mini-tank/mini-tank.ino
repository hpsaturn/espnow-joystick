#include <EspNowJoystick.hpp>
#include <ESP32Servo.h>
#include <U8g2lib.h>
#include <analogWrite.h>
#include "GUI.h"

#define BUILTINLED 22

EspNowJoystick joystick;
TelemetryMessage tm;

Servo servoLeft;
Servo servoRight;
int servoLeftPin = 16;
int servoRightPin = 17;

bool running, fire;
uint32_t count = 0;

const int spanLeft = 20;
const int offsetLeft = 0;
const int degreesCenterL = 96;
const int degreesMinL = degreesCenterL - spanLeft + offsetLeft;
const int degreesMaxL = degreesCenterL + spanLeft + offsetLeft;

const int deathBand = 10;

const int spanRight = 20;
const int offsetRight = -3;
const int degreesCenterR = 101;
const int degreesMinR = degreesCenterR - spanRight + offsetRight;
const int degreesMaxR = degreesCenterR + spanRight + offsetRight;

int lastVty = 0;

void attachServoLeft() {
  if (!servoLeft.attached()) servoLeft.attach(servoLeftPin);
}

void attachServoRight() {
  if (!servoRight.attached()) servoRight.attach(servoRightPin);
}

void detachServos() {
  servoLeft.detach();
  servoRight.detach();
}

/**
 * @param Vtx Joystick left stick, X axis (Left/Right)
 * @param Vty Joystick left stick, Y axis (forward/backward)
 * @param Wt Joystick right stick, X asis (Left/Right) 
 * 
 * The next code to controlling the servos. You able to capture more data from the Joystick Object,
 * for now, for simplify, we only use two sticks but only two axis.
 * 
 * The current version was improved from
 * the code @acicuecalo for Arduino IDE:
 * https://github.com/acicuecalo/Robot_mini_tanque
*/
void setSpeed(int16_t Vtx, int16_t Vty, int16_t Wt) {
  Vtx = constrain(-Wt, -100, 100);
  Vty = constrain(Vty, -100, 100);
  //   Wt = constrain(Wt, -100, 100);

  int spdL;
  int spdR;

  if (abs(Vtx) < deathBand && abs(Vty) < deathBand) {
    Vtx = 0;
    Vty = 0;
  }
 
  // Mixer
  spdL = Vty + Vtx;   //motorDelanteroIzquierdo
  spdR = -Vty + Vtx;  //motorDelanteroDerecho

  // Servo output
  spdL = map(spdL, -100, 100, degreesMinL, degreesMaxL);
  spdR = map(spdR, -100, 100, degreesMinR, degreesMaxR);
   
  if (spdL != degreesCenterL) {
    attachServoLeft();
    servoLeft.write(spdL);
  } else {
    servoLeft.detach();
  }

  if (spdR != degreesCenterR) {
    attachServoRight();
    servoRight.write(spdR);
  } else {
    servoRight.detach();
  }
  // GUI Variables
  if(lastVty!=0) lastVty = Vty; 
  if(Vty!=0) lastVty = Vty;
  analogWrite(BUILTINLED, abs(Vty));
  // Debugging
  if (spdL !=degreesCenterL || spdR != degreesCenterR)
    Serial.printf("[spdR:%04d spdL:%04d]\r\n", spdR, spdL);
}

void sendHeartbeat() {
  static uint_least32_t timeStamp = 0;
  if (millis() - timeStamp > 500) {
    timeStamp = millis();
    tm.e1 = true;
    joystick.sendTelemetryMsg(tm);
  }
}

static uint_least32_t connectStamp = 0;

void checkRunning() {
  if (millis() - connectStamp > 100) {
    running = false;
    setSpeed(0, 0, 0);
  }
}

class MyJoystickCallback : public EspNowJoystickCallbacks {
  void onJoystickMsg(JoystickMessage jm) {
    // Serial.println("[Joystick]");
    connectStamp = millis();
    if (jm.ck == 0x02 && jm.bA == 1) {
      fire = true;
    }
    if (jm.ck == 0x01) {
      static uint_least32_t speedStamp = 0;
      if (millis() - speedStamp > 20) {
        speedStamp = millis();
        setSpeed(jm.ax - 100, jm.ay - 100, jm.az - 100);
        running = true;
      }
    } 
  };
  void onError(const char* msg) {
    setSpeed(0, 0, 0);
    Serial.println("Error");
  };
};

void setup() {
  Serial.begin(115200);
  delay(100);
  displayInit();
  showWelcome();
  joystick.setJoystickCallbacks(new MyJoystickCallback());
  tm = joystick.newTelemetryMsg();
  joystick.init();
  showWelcomeMessage("ESPNow ready");

  // Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  attachServoLeft();
  attachServoRight();
  showWelcomeMessage("Servos ready");
  delay(500);
  showWelcomeMessage("== SETUP READY ==");
}

void loop() {
  checkRunning();
  sendHeartbeat();
  displayEmoticon(lastVty);
  delay(20);
}
