#include <WiFi.h>
#include <WiFiUdp.h>
#include <analogWrite.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include "simple.pb.h"

int16_t speed_buff[4] = {0};
int8_t speed_sendbuff[4] = {0};
uint32_t count = 0;
uint8_t IIC_ReState = I2C_ERROR_NO_BEGIN;

const char *ssid = "ROBOTAP";
const char *password = "77777777";

WiFiServer server(80);
WiFiUDP udp;
uint8_t buffer[128];
SimpleMessage message = SimpleMessage_init_zero;

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
    // Serial.printf("[Vtx:%04d Vty:%04d Wt:%04d ]", Vtx, Vty, Wt);
    Wt = (Wt > 100) ? 100 : Wt;
    Wt = (Wt < -100) ? -100 : Wt;

    Vtx = (Vtx > 100) ? 100 : Vtx;
    Vtx = (Vtx < -100) ? -100 : Vtx;
    Vty = (Vty > 100) ? 100 : Vty;
    Vty = (Vty < -100) ? -100 : Vty;

    Vtx = (Wt != 0) ? Vtx * (100 - abs(Wt)) / 100 : Vtx;
    Vty = (Wt != 0) ? Vty * (100 - abs(Wt)) / 100 : Vty;

    speed_buff[0] = Vty - Vtx - Wt;
    speed_buff[1] = Vty + Vtx + Wt;
    speed_buff[3] = Vty - Vtx + Wt;
    speed_buff[2] = Vty + Vtx - Wt;

    for (int i = 0; i < 4; i++) {
        speed_buff[i] = (speed_buff[i] > 100) ? 100 : speed_buff[i];
        speed_buff[i] = (speed_buff[i] < -100) ? -100 : speed_buff[i];
        speed_sendbuff[i] = speed_buff[i];
    }
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

    analogWriteResolution(19, 12);
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
                Serial.printf("[%04d %04d %04d] [", message.ax - 100, message.ay - 100, message.az - 100);
                int ledout = map(message.ay, 0, 192, 0, 255);
                Serial.printf("led: %03d] \n", ledout);
                analogWrite(19, ledout);
                setSpeed(message.ax - 100, message.ay - 100, message.az - 100);
            } else {
                setSpeed(0, 0, 0);
            }
        } else {
            setSpeed(0, 0, 0);
        }
    }
}
