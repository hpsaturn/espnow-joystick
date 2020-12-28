#include <analogWrite.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <pb_encode.h>
#include <pb_decode.h>
#include "simple.pb.h"

const char *ssid = "ROBOTAP";
const char *password = "77777777";

WiFiServer server(80);

WiFiUDP Udp1;

/* Now we could transmit the message over network, store it in a file or
* wrap it to a pigeon's leg.
*/

/* But because we are lazy, we will just decode it immediately. */

uint8_t buffer[128];
size_t message_length;
bool status;
 

size_t encodeMessage(int msg) {
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

    /* Fill in the lucky number */
    message.lucky_number = msg;

    /* Now we are ready to encode the message! */
    status = pb_encode(&stream, SimpleMessage_fields, &message);
    message_length = stream.bytes_written;

    Serial.print("message length: "); 
    Serial.println(message_length);

    /* Then just check for any errors.. */
    if (!status) {
        printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
        while(1);
    }

    return message_length;
}

void decodeMessage() {

    /* This is the buffer where we will store our message. */
    SimpleMessage message = SimpleMessage_init_zero;

    /* Create a stream that reads from the buffer. */
    pb_istream_t stream = pb_istream_from_buffer(buffer, message_length);

    /* Now we are ready to decode the message. */
    status = pb_decode(&stream, SimpleMessage_fields, &message);

    /* Check for errors... */
    if (!status) {
        printf("Decoding failed: %s\n", PB_GET_ERROR(&stream));
        while(1);
    }

    /* Print the data contained in the message. */
    printf("Your lucky number was %d!\n", (int)message.lucky_number);

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
    Serial.printf("[Vtx:%04d Vty:%04d Wt:%04d ]",Vtx,Vty,Wt);
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
    Serial.printf("[b0:%04d b1:%04d b2:%04d b3:%04d]\n",speed_buff[0],speed_buff[1],speed_buff[2],speed_buff[3]);

    for (int i = 0; i < 4; i++)
    {
        speed_buff[i] = (speed_buff[i] > 100) ? 100 : speed_buff[i];
        speed_buff[i] = (speed_buff[i] < -100) ? -100 : speed_buff[i];
        speed_sendbuff[i] = speed_buff[i];
    }
}

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

    encodeMessage(13);
    decodeMessage();
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
            // for (int i = 0; i < 8; i++)
            // {
            //     Serial.printf("%02X ", udodata[i]);
            // }
            // Serial.println();
            if (udodata[6] == 0x01)
            {
                Serial.printf("[%04d %04d %04d] [",udodata[3] - 100, udodata[4] - 100, udodata[5] - 100);
                int ledout = map(udodata[4], 0, 192, 0, 255);
                Serial.printf("led: %03d] ",ledout);
                analogWrite(19,ledout);
                Setspeed(udodata[3] - 100, udodata[4] - 100, udodata[5] - 100);
            }
            else
            {
                // Serial.printf("udodata[6] != 0x01 -> %d %d %d\n",0,0,0);
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
