#include <M5StickC.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <pb_decode.h>
#include <pb_encode.h>

#include "EEPROM.h"
#include "simple.pb.h"

#define EEPROM_SIZE 64

TFT_eSprite Disbuff = TFT_eSprite(&M5.Lcd);
extern const unsigned char connect_on[800];
extern const unsigned char connect_off[800];

#define SYSNUM 3

uint64_t realTime[4], time_count = 0;
bool k_ready = false;
uint32_t key_count = 0;

IPAddress local_IP(192, 168, 4, 100 + SYSNUM);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);    //optional
IPAddress secondaryDNS(8, 8, 4, 4);  //optional

const char *ssid = "ROBOTAP";
const char *password = "77777777";

WiFiUDP udp;

char APName[20];
String WfifAPBuff[16];
uint32_t count_bn_a = 0, choose = 0;
String ssidname;

uint8_t adc_value[5] = {0};
uint16_t AngleBuff[4];
uint32_t count = 0;

uint8_t buffer[128];
size_t message_length;
bool status;

bool sendMessage(uint8_t ax, uint8_t ay, uint8_t az, uint8_t ck) {
    /* This is the buffer where we will store our message. */
    SimpleMessage message = SimpleMessage_init_zero;

    /* Allocate space on the stack to store the message data.
    *
    * Nanopb generates simple struct definitions for all the messages.
    * - check out the contents of simple.pb.h!
    * It is a good idea to always initialize your structures
    * so that you do not have garbage data from RAM in there.
    */

    /* Create a stream that will write to our buffer. */
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    /* Fill the proto values (joystick angle)*/
    message.ax = ax;
    message.ay = ay;
    message.az = az;
    message.ck = ck;

    /* Now we are ready to encode the message! */
    status = pb_encode(&stream, SimpleMessage_fields, &message);
    message_length = stream.bytes_written;
    /* Then just check for any errors.. */

    if (!status) {
        printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
        return false;
    }

    /* Now we are ready to send it */

    uint8_t sendBuff[message_length + 4];

    sendBuff[0] = 0xAA;  // UDP header?
    sendBuff[1] = 0x55;
    sendBuff[2] = SYSNUM;  // UDP type?

    for (int i = 0; i < message_length; i++) {
        sendBuff[i + 3] = buffer[i];  // proto message
    }

    sendBuff[message_length + 3] = 0xee;  // UDP end?

    if (WiFi.status() == WL_CONNECTED) {
        udp.beginPacket(IPAddress(192, 168, 4, 1), 1000 + SYSNUM);
        udp.write(sendBuff, message_length + 4);
        udp.endPacket();
        return true;
    }

    return false;
}

uint8_t I2CRead8bit(uint8_t Addr) {
    Wire.beginTransmission(0x38);
    Wire.write(Addr);
    Wire.endTransmission();
    Wire.requestFrom(0x38, 1);
    return Wire.read();
}

uint16_t I2CRead16bit(uint8_t Addr) {
    uint16_t ReData = 0;
    Wire.beginTransmission(0x38);
    Wire.write(Addr);
    Wire.endTransmission();
    Wire.requestFrom(0x38, 2);
    for (int i = 0; i < 2; i++) {
        ReData <<= 8;
        ReData |= Wire.read();
    }
    return ReData;
}

void setup() {
    M5.begin();
    Wire.begin(0, 26, 10000);
    EEPROM.begin(EEPROM_SIZE);

    M5.Lcd.setRotation(4);
    M5.Lcd.setSwapBytes(false);
    Disbuff.createSprite(80, 160);
    Disbuff.setSwapBytes(true);

    Disbuff.fillRect(0, 0, 80, 20, Disbuff.color565(50, 50, 50));
    Disbuff.setTextSize(2);
    Disbuff.setTextColor(GREEN);
    Disbuff.setCursor(55, 6);
    //Disbuff.printf("%03d",SYSNUM);

    Disbuff.pushImage(0, 0, 20, 20, (uint16_t *)connect_off);
    Disbuff.pushSprite(0, 0);

    uint8_t res = I2CRead8bit(0x32);
    Serial.printf("Res0 = %02X \r\n", res);

    M5.update();

    if ((EEPROM.read(0) != 0x56) || (M5.BtnA.read() == 1)) {
        WiFi.mode(WIFI_STA);
        int n = WiFi.scanNetworks();
        Disbuff.setTextSize(1);
        Disbuff.setTextColor(GREEN);
        Disbuff.fillRect(0, 0, 80, 20, Disbuff.color565(50, 50, 50));

        if (n == 0) {
            Disbuff.setCursor(5, 20);
            Disbuff.printf("no networks");

        } else {
            int count = 0;
            for (int i = 0; i < n; ++i) {
                if (WiFi.SSID(i).indexOf("ROBOTAP") != -1) {
                    if (count == 0) {
                        Disbuff.setTextColor(GREEN);
                    } else {
                        Disbuff.setTextColor(WHITE);
                    }
                    Disbuff.setCursor(5, 25 + count * 10);
                    String str = WiFi.SSID(i);
                    Disbuff.printf(str.c_str());
                    WfifAPBuff[count] = WiFi.SSID(i);
                    count++;
                }
            }
            Disbuff.pushSprite(0, 0);
            while (1) {
                if (M5.BtnA.read() == 1) {
                    if (count_bn_a >= 200) {
                        count_bn_a = 201;
                        EEPROM.writeUChar(0, 0x56);
                        EEPROM.writeString(1, WfifAPBuff[choose]);
                        ssidname = WfifAPBuff[choose];
                        break;
                    }
                    count_bn_a++;
                    Serial.printf("count_bn_a %d \n", count_bn_a);
                } else if ((M5.BtnA.isReleased()) && (count_bn_a != 0)) {
                    Serial.printf("count_bn_a %d", count_bn_a);
                    if (count_bn_a > 200) {
                    } else {
                        choose++;
                        if (choose >= count) {
                            choose = 0;
                        }
                        Disbuff.fillRect(0, 0, 80, 20, Disbuff.color565(50, 50, 50));
                        for (int i = 0; i < count; i++) {
                            Disbuff.setCursor(5, 25 + i * 10);
                            if (choose == i) {
                                Disbuff.setTextColor(GREEN);
                            } else {
                                Disbuff.setTextColor(WHITE);
                            }
                            Disbuff.printf(WfifAPBuff[i].c_str());
                        }
                        Disbuff.pushSprite(0, 0);
                    }
                    count_bn_a = 0;
                }
                delay(10);
                M5.update();
            }
            //EEPROM.writeString(1,WfifAPBuff[0]);
        }
    } else if (EEPROM.read(0) == 0x56) {
        ssidname = EEPROM.readString(1);
        EEPROM.readString(1, APName, 16);
    }

    Disbuff.fillRect(0, 20, 80, 140, BLACK);

    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
        Serial.println("STA Failed to configure");
    }

    WiFi.begin(ssidname.c_str(), password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    udp.begin(2000);

    Disbuff.pushImage(0, 0, 20, 20, (uint16_t *)connect_on);
    Disbuff.pushSprite(0, 0);
}

void drawValues(uint8_t ax, uint8_t ay, uint8_t az) {
    Disbuff.fillRect(0, 30, 80, 130, BLACK);
    Disbuff.setCursor(10, 30);
    Disbuff.printf("AX: %04d", ax);
    Disbuff.setCursor(10, 45);
    Disbuff.printf("AY: %04d", ay);
    Disbuff.setCursor(10, 60);
    Disbuff.printf("AZ: %04d", az);
}

void loop() {
    for (int i = 0; i < 4; i++) {
        AngleBuff[i] = I2CRead16bit(0x50 + i * 2);
    }

    delay(10);

    if (WiFi.status() != WL_CONNECTED) {
        Disbuff.pushImage(0, 0, 20, 20, (uint16_t *)connect_off);
        Disbuff.pushSprite(0, 0);
        if (count++ > 1000) {
            WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);
            count = 0;
        }

    } else {
        Disbuff.pushImage(0, 0, 20, 20, (uint16_t *)connect_on);
        Disbuff.pushSprite(0, 0);

        uint8_t ax = map(AngleBuff[0], 0, 4000, 0, 200);
        uint8_t ay = map(AngleBuff[1], 0, 4000, 0, 200);
        uint8_t az = map(AngleBuff[2], 0, 4000, 0, 200);
        uint8_t ck = 0x00;

        if ((ax > 110) || (ax < 90) ||
            (ay > 110) || (ay < 90) ||
            (az > 110) || (az < 90)) {
            ck = 0x01;
        }
        drawValues(ax, ay, az);
        sendMessage(ax, ay, az, ck);
    }
}