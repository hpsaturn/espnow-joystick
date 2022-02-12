#include <WiFi.h>
#include <esp_now.h>
#include <analogWrite.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include <ESP32MotorControl.h>
#include "comm.pb.h"

#define BUILTINLED  19

uint32_t count = 0;
uint8_t IIC_ReState = I2C_ERROR_NO_BEGIN;

uint8_t buffer[128];
JoystickMessage jm = JoystickMessage_init_zero;

#define MRIGHT 1
#define MLEFT  0

#define MIN1  27
#define MIN2  25
#define MIN3  22
#define MIN4  21

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

bool decodeMessage(uint16_t message_length) {
    /* This is the buffer where we will store our message. */

    /* Create a stream that reads from the buffer. */
    pb_istream_t stream = pb_istream_from_buffer(buffer, message_length);

    /* Now we are ready to decode the message. */
    bool status = pb_decode(&stream, JoystickMessage_fields, &jm);

    /* Check for errors... */
    if (!status) {
        printf("Decoding failed: %s\n", PB_GET_ERROR(&stream));
        return false;
    }
    return true;
}

void formatMacAddress(const uint8_t *macAddr, char *buffer, int maxLength) {
    snprintf(buffer, maxLength, "%02x:%02x:%02x:%02x:%02x:%02x", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
}

void receiveCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen) {
    // only allow a maximum of 250 characters in the message + a null terminating byte
    char udodata[ESP_NOW_MAX_DATA_LEN + 1];
    int msgLen = min(ESP_NOW_MAX_DATA_LEN, dataLen);
    strncpy(udodata, (const char *)data, msgLen);
    // make sure we are null terminated
    udodata[msgLen] = 0;
    // format the mac address
    char macStr[18];
    formatMacAddress(macAddr, macStr, 18);
    // debug log the message to the serial port
    // Serial.printf("Received message from: %s - %s\n", macStr, buffer);
    // what are our instructions
    
    // char udodata[dataLen];
    // udp.read(udodata, udplength);
    // IPAddress udp_client = udp.remoteIP();
    if ((udodata[0] == 0xAA) && (udodata[1] == 0x55) && (udodata[msgLen - 1] == 0xee)) {
        for (int i = 0; i < msgLen - 4; i++) {
            buffer[i] = udodata[i + 3];
        }
        decodeMessage(msgLen - 4);
        if (jm.ck == 0x01) {
            setSpeed(jm.ax - 100, jm.ay - 100, jm.az - 100);
        } else {
            setSpeed(0, 0, 0);
        }
    } else {
        setSpeed(0, 0, 0);
    }
    
}

// callback when data is sent
void sentCallback(const uint8_t *macAddr, esp_now_send_status_t status) {
    char macStr[18];
    formatMacAddress(macAddr, macStr, 18);
    // Serial.print("Last Packet Sent to: ");
    // Serial.println(macStr);
    // Serial.print("Last Packet Send Status: ");
    // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
    Serial.begin(115200);
    delay(100);
    WiFi.mode(WIFI_STA);
    // startup ESP Now
    Serial.println("ESPNow Init");
    Serial.println(WiFi.macAddress());
    // shut down wifi
    WiFi.disconnect();

    if (esp_now_init() == ESP_OK) {
        Serial.println("ESPNow Init Success");
        esp_now_register_recv_cb(receiveCallback);
        esp_now_register_send_cb(sentCallback);
    } else {
        Serial.println("ESPNow Init Failed");
        delay(3000);
        ESP.restart();
    }

    analogWriteResolution(BUILTINLED, 12);   // builtin LED for TTGO-T7 v1.3 (see docs directory)

    mc.attachMotors(MIN1,MIN2,MIN3,MIN4);
}

void loop() {
}
