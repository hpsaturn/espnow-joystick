#include <EspNowJoystick.hpp>
#include <analogWrite.h>
#include <ESP32MotorControl.h>

#define BUILTINLED  19

uint32_t count = 0;

#define MRIGHT 1
#define MLEFT  0

#define MIN1  27
#define MIN2  25
#define MIN3  22
#define MIN4  21

EspNowJoystick joystick;
TelemetryMessage tm;

ESP32MotorControl mc = ESP32MotorControl();
static uint_least64_t debugCount = 0;

void setSpeed(int16_t Vtx, int16_t Vty, int16_t Wt) {
    Wt = (Wt > 100) ? 100 : Wt;
    Wt = (Wt < -100) ? -100 : Wt;

    Vtx = (Vtx > 100) ? 100 : Vtx;
    Vtx = (Vtx < -100) ? -100 : Vtx;

    Vty = (Vty > 100) ? 100 : Vty;
    Vty = (Vty < -100) ? -100 : Vty;

    int speed = map(abs(Vty), 0, 100, 60, 200);
    int turn = map(abs(Wt), 0, 100, 0, 130);
    
    // Serial.printf("[Vtx:%04d Vty:%04d Wt:%04d ]\n", Vtx, Vty, Wt);
    Serial.println(debugCount++);

    int dir = (Wt>0) ? 0 : 1; // 0 left, 1 right

    if (Vty > 10) {
        if (dir == 0) {  // turn left
            mc.motorForward(MRIGHT, speed);
            mc.motorForward(MLEFT, speed - turn);
        } else {  // turn right
            mc.motorForward(MRIGHT, speed - turn);
            mc.motorForward(MLEFT, speed);
        }
        analogWrite(BUILTINLED, abs(Vty));

    } else if (Vty < -10) {
        if (dir == 0) {  // turn left
            mc.motorReverse(MRIGHT, speed);
            mc.motorReverse(MLEFT, speed - turn);
        } else {
            mc.motorReverse(MRIGHT, speed - turn);
            mc.motorReverse(MLEFT, speed);
        }
        analogWrite(BUILTINLED, abs(Vty));
    } 
    else if (Vty < 10 && Vty > -10 && Wt !=0 ) {
        speed=turn+60;
        if (dir == 0) {  // turn left
            mc.motorReverse(MLEFT, speed);
            mc.motorForward(MRIGHT, speed);
        } else {
            mc.motorForward(MLEFT, speed);
            mc.motorReverse(MRIGHT, speed);
        }
        analogWrite(BUILTINLED, abs(turn));
    }
    else {
        mc.motorsStop();
        analogWrite(BUILTINLED, 0);
    }

}

void sendHeartbeat() {
    static uint_least32_t timeStamp = 0;
    if (millis() - timeStamp > 500) {
        timeStamp = millis();
        tm.e1 = true;
        joystick.sendTelemetryMsg(tm);
    }
}

class MyJoystickCallback : public EspNowJoystickCallbacks {
    void onJoystickMsg(JoystickMessage jm){
        if (jm.ck == 0x01) {
            setSpeed(jm.ax - 100, jm.ay - 100, jm.az - 100);
        } else {
            setSpeed(0, 0, 0);
        }
        sendHeartbeat();
    };
    void onError(const char *msg){
        setSpeed(0, 0, 0);
        Serial.println("Error");
    };
};

void setup() {
    Serial.begin(115200);
    delay(100);

    joystick.setJoystickCallbacks(new MyJoystickCallback());
    tm = joystick.newTelemetryMsg();
    joystick.init();

    analogWriteResolution(BUILTINLED, 12);   // builtin LED for TTGO-T7 v1.3 (see docs directory)
    mc.attachMotors(MIN1,MIN2,MIN3,MIN4);
}

void loop() {}
