#include <Arduino.h>
#include <analogWrite.h>
#include <WiFi.h>
#include <WiFiUdp.h>

const char *ssid = "M5AP";
const char *password = "77777777";

WiFiServer server(80);

WiFiUDP Udp1;

void setup()
{
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

    Udp1.begin(1003);

    analogWriteResolution(19, 12);
}

uint8_t SendBuff[9] = {0xAA, 0x55,
                       0x00,
                       0x00,
                       0x00,
                       0x00,
                       0x00,
                       0x00,
                       0xee};

int16_t speed_buff[4] = {0};
int8_t speed_sendbuff[4] = {0};
uint32_t count = 0;
uint8_t IIC_ReState = I2C_ERROR_NO_BEGIN;

void Setspeed(int16_t Vtx, int16_t Vty, int16_t Wt)
{
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

    for (int i = 0; i < 4; i++)
    {
        speed_buff[i] = (speed_buff[i] > 100) ? 100 : speed_buff[i];
        speed_buff[i] = (speed_buff[i] < -100) ? -100 : speed_buff[i];
        speed_sendbuff[i] = speed_buff[i];
    }
    Serial.printf("data: %s",(uint8_t *)speed_sendbuff);
}

void loop()
{
    int udplength = Udp1.parsePacket();
    if (udplength)
    {
        char udodata[udplength];
        Udp1.read(udodata, udplength);
        IPAddress udp_client = Udp1.remoteIP();
        if ((udodata[0] == 0xAA) && (udodata[1] == 0x55) && (udodata[7] == 0xee))
        {
            for (int i = 0; i < 8; i++)
            {
                Serial.printf("%02X ", udodata[i]);
            }
            Serial.println();
            if (udodata[6] == 0x01)
            {
                Serial.printf("udodata[6] == 0x01 -> %d %d %d\n",udodata[3] - 100, udodata[4] - 100, udodata[5] - 100);
                int ledout = map(udodata[4], 0, 190, 0, 255);
                Serial.printf("led set to %d\n",ledout);
                analogWrite(19,ledout);
                // Setspeed(udodata[3] - 100, udodata[4] - 100, udodata[5] - 100);
            }
            else
            {
                Serial.printf("udodata[6] != 0x01 -> %d %d %d\n",0,0,0);
            }
        }
        else
        {
            Serial.printf("udodata[6] != 0x01 -> %d %d %d\n", 0, 0, 0);
        }
    }
    count++;
    if (count > 100)
    {
        count = 0;

    }
}
