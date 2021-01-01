#include <WiFi.h>
#include <WiFiUdp.h>
#include <analogWrite.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include <ESP32MotorControl.h>
#include "simple.pb.h"

uint32_t count = 0;
uint8_t IIC_ReState = I2C_ERROR_NO_BEGIN;

const char *ssid = "ROBOTAP";
const char *password = "77777777";

WiFiServer server(80);
WiFiUDP udp;
uint8_t buffer[128];
SimpleMessage message = SimpleMessage_init_zero;

#define LEFT_MOTOR   0
#define RIGHT_MOTOR  1

#define MIN1  27
#define MIN2  25
#define MIN3  22
#define MIN4  21

ESP32MotorControl mc = ESP32MotorControl();

bool decodeMessage(uint16_t message_length) {
    /* This is the buffer where we will store our message. */

    /* Create a stream that reads from the buffer. */
    pb_istream_t stream = pb_istream_from_buffer(buffer, message_length);

    /* Now we are ready to decode the message. */
    bool status = pb_decode(&stream, SimpleMessage_fields, &message);

    /* Check for errors... */
    if (!status) {
        printf("Decoding failed: %s\n", PB_GET_ERROR(&stream));
        return false;
    }

    return true;
}

void setSpeed(int16_t Vtx, int16_t Vty, int16_t Wt) {
    Wt = (Wt > 100) ? 100 : Wt;
    Wt = (Wt < -100) ? -100 : Wt;

    Vtx = (Vtx > 100) ? 100 : Vtx;
    Vtx = (Vtx < -100) ? -100 : Vtx;

    Vty = (Vty > 100) ? 100 : Vty;
    Vty = (Vty < -100) ? -100 : Vty;

    // Vtx = (Wt != 0) ? Vtx * (100 - abs(Wt)) / 100 : Vtx;
    // Vty = (Wt != 0) ? Vty * (100 - abs(Wt)) / 100 : Vty;


    int speed = map(abs(Vty), 0, 100, 0, 160);
    int turn = abs(Wt);
    
    Serial.printf("[Vtx:%04d Vty:%04d Wt:%04d ]\n", Vtx, Vty, Wt);

    int dir = (Wt>0) ? 0 : 1; // 0 left, 1 right

    if (Vty > 10) {
        if (dir == 0) {  // turn left
            mc.motorForward(LEFT_MOTOR, speed);
            mc.motorForward(RIGHT_MOTOR, speed - turn);
        } else {  // turn right
            mc.motorForward(LEFT_MOTOR, speed - turn);
            mc.motorForward(RIGHT_MOTOR, speed);
        }
    } else if (Vty < -10) {
        if (dir == 0) {  // turn left
            mc.motorReverse(LEFT_MOTOR, speed);
            mc.motorReverse(RIGHT_MOTOR, speed - turn);
        } else {
            mc.motorReverse(LEFT_MOTOR, speed - turn);
            mc.motorReverse(RIGHT_MOTOR, speed);
        }
    } 
    else if (Vty < 10 && Vty > -10 && Wt !=0 ) {
        if (dir == 0) {  // turn left
            mc.motorReverse(RIGHT_MOTOR, turn);
            mc.motorForward(LEFT_MOTOR, turn);
        } else {
            mc.motorForward(RIGHT_MOTOR, turn);
            mc.motorReverse(LEFT_MOTOR, turn);
        }
    }
    else {
        mc.motorsStop();
    }

    // analogWrite(19, speed);

    // Serial.printf("led: %03d] ", ledout);
    // mc.motorForward(LEFT_MOTOR, ledout);
    // mc.motorForward(RIGHT_MOTOR, ledout);

    // speed_buff[0] = Vty - Vtx - Wt;
    // speed_buff[1] = Vty + Vtx + Wt;
    // speed_buff[3] = Vty - Vtx + Wt;
    // speed_buff[2] = Vty + Vtx - Wt;

    // for (int i = 0; i < 4; i++) {
    //     speed_buff[i] = (speed_buff[i] > 100) ? 100 : speed_buff[i];
    //     speed_buff[i] = (speed_buff[i] < -100) ? -100 : speed_buff[i];
    //     speed_sendbuff[i] = speed_buff[i];
    // }
    // Serial.printf("[b0:%04d b1:%04d b2:%04d b3:%04d]\n", speed_buff[0], speed_buff[1], speed_buff[2], speed_buff[3]);
}

void setup() {
    Serial.begin(115200);
    uint64_t chipid = ESP.getEfuseMac();
    String str = ssid + String((uint32_t)(chipid >> 32), HEX);
    //Set device in STA mode to begin with
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1),
                      IPAddress(192, 168, 4, 1),
                      IPAddress(255, 255, 255, 0));

    WiFi.softAP(str.c_str(), password);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    server.begin();

    udp.begin(1003);

    analogWriteResolution(19, 12);   // builtin LED for TTGO-T7 v1.3 (see docs directory)

    mc.attachMotors(MIN1,MIN2,MIN3,MIN4);
}

void loop() {
    int udplength = udp.parsePacket();
    if (udplength) {
        char udodata[udplength];
        udp.read(udodata, udplength);
        IPAddress udp_client = udp.remoteIP();
        if ((udodata[0] == 0xAA) && (udodata[1] == 0x55) && (udodata[udplength - 1] == 0xee)) {
            for (int i = 0; i < udplength-4; i++) {
                buffer[i] = udodata[i+3];
            }
            
            decodeMessage(udplength-4);
            
            if (message.ck == 0x01) {
                // Serial.printf("[%04d %04d %04d] [", message.ax - 100, message.ay - 100, message.az - 100);
                // Serial.printf("[%04d %04d %04d] [", message.ax - 100, message.ay - 100, message.az - 100);
                setSpeed(message.ax - 100, message.ay - 100, message.az - 100);
            } else {
                setSpeed(0, 0, 0);
            }
        } else {
            setSpeed(0, 0, 0);
        }
    }
    else {
        delay(10);
    }
}
