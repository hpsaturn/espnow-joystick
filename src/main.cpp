#include <Arduino.h>
#include <analogWrite.h>

// object avoidance robot, two DC motors driven by a A4988 module
// speed is set with value of 0-255 but 255 is the slowest speed and 0 the fastest

int dirPin = 2;
int stepperPin = 27;
int en = 25;
int rst = 4;
const int trigPin = 35;
const int echoPin = 33;

uint32_t state = 0;
uint32_t oldSpeed = 0;



void setup() {
    Serial.begin(115200);
    pinMode(dirPin, OUTPUT);
    pinMode(stepperPin, OUTPUT);
    // pinMode(en, OUTPUT);   //enable, active low
    pinMode(rst, OUTPUT);  //rst, active low
    digitalWrite(rst, HIGH);

    // pinMode(trigPin, OUTPUT);
    // pinMode(echoPin, INPUT);

    // digitalWrite(en, LOW);
    digitalWrite(dirPin, HIGH);
    delay(1000);
    analogWriteResolution(en, 12);
    randomSeed(analogRead(A0));
    Serial.println("-> setup ready.");
}

//make some steps
void step(int stepsit) {
    //digitalWrite(dirPin,dir);
    //delay(50);
    for (int i = 0; i < stepsit; i++) {
        digitalWrite(stepperPin, HIGH);
        delayMicroseconds(800);
        digitalWrite(stepperPin, LOW);
        delayMicroseconds(800);
    }
}

uint32_t motors(int robot_direction, int robot_speed)  // this determines how many steps to what direction
{
    switch (robot_direction) {
        case 0:
            if (state != robot_direction) {
                digitalWrite(en, HIGH);
                robot_direction = state;
            }
            break;
        case 1:
            if (state != robot_direction || oldSpeed != robot_speed) {
                digitalWrite(rst, LOW);
                delayMicroseconds(5);
                digitalWrite(rst, HIGH);

                analogWrite(en, robot_speed);
                oldSpeed = robot_speed;

                step(2);
            }
            break;
        case 2:
            if (state != robot_direction || oldSpeed != robot_speed) {
                digitalWrite(rst, LOW);
                delayMicroseconds(5);
                digitalWrite(rst, HIGH);
                analogWrite(en, robot_speed);
                oldSpeed = robot_speed;
                //step(false, 2);
            }
            break;
        case 4:
            if (state != robot_direction || oldSpeed != robot_speed) {
                digitalWrite(rst, LOW);
                delayMicroseconds(5);
                digitalWrite(rst, HIGH);
                digitalWrite(en, LOW);
                analogWrite(en, robot_speed);
                oldSpeed = robot_speed;
                step(1);
            }
            break;
        case 5:
            if (state != robot_direction || oldSpeed != robot_speed) {
                digitalWrite(rst, LOW);
                delayMicroseconds(5);
                digitalWrite(rst, HIGH);
                digitalWrite(en, LOW);
                analogWrite(en, robot_speed);
                oldSpeed = robot_speed;
                step(3);
            }
            break;
    }

    return (robot_direction);
}

int ultra() {
    // just get distance:

    int result = 0;
    unsigned long kesto = 0;
    unsigned long matka = 0;
    for (int i = 0; i < 3; i++) {
        digitalWrite(trigPin, LOW);
        delayMicroseconds(2);
        digitalWrite(trigPin, HIGH);

        delayMicroseconds(10);

        digitalWrite(trigPin, LOW);
        delayMicroseconds(1);

        kesto = pulseIn(echoPin, HIGH);

        matka = matka + (kesto / 58.2);
        delay(10);
    }
    matka /= 3;
    delay(10);
    Serial.println(matka);
    result = matka;
    return (result);
}

void loop() {  // some simple object avoidance
    // int view = ultra();
    // view = min(view, 100);

    // if (view < 30) {
    //     if (view < 15) {
    //         state = motors(1, 100);  //backwards
    //         delay(800);
    //     }
    //     int way = random(4, 6);  //right or left
    //     while (ultra() < 30)
    //         state = motors(way, 70);
    //     delay(110);
    // }

    // view = map(view, 0, 100, 5, 150);
    // view = 255 - view;
    state = motors(2, 100);  //go forward, speed depends on distance ahead
    delay(10);
}