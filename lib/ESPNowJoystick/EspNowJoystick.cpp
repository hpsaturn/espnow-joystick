#include <EspNowJoystick.hpp>

JoystickMessage _jm = JoystickMessage_init_zero;
TelemetryMessage _tm = TelemetryMessage_init_zero;
uint8_t buffer[128];

EspNowJoystick::EspNowJoystick() {
    _pEspNowJoystickCallbacks = nullptr;
    _pEspNowTelemetryCallbacks = nullptr;
    _ESP_ID = getDeviceId(); 
}

void EspNowJoystick::setJoystickCallbacks(EspNowJoystickCallbacks* pCallbacks) {
    _pEspNowJoystickCallbacks = pCallbacks;
}

void EspNowJoystick::setTelemetryCallbacks(EspNowTelemetryCallbacks* pCallbacks) {
    _pEspNowTelemetryCallbacks = pCallbacks;
}

void formatMacAddress(const uint8_t *macAddr, char *buffer, int maxLength) {
    snprintf(buffer, maxLength, "%02x:%02x:%02x:%02x:%02x:%02x", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
}

bool EspNowJoystick::sendJoystickMsg(JoystickMessage jm) {

    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    bool status = pb_encode(&stream, JoystickMessage_fields, &jm);
    size_t message_length = stream.bytes_written;

    if (!status) {
        printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
        return false;
    }

    uint8_t sendBuff[message_length + 4];

    sendBuff[0] = 0xAA;  // UDP header?
    sendBuff[1] = 0x55;
    sendBuff[2] = SYSNUM;  // UDP type?

    for (int i = 0; i < message_length; i++) {
        sendBuff[i + 3] = buffer[i];  // proto message
    }

    sendBuff[message_length + 3] = 0xee;  // UDP end?

    // this will broadcast a message to everyone in range
    uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    esp_now_peer_info_t peerInfo = {};
    memcpy(&peerInfo.peer_addr, broadcastAddress, 6);
    if (!esp_now_is_peer_exist(broadcastAddress)) {
        esp_now_add_peer(&peerInfo);
    }
    esp_err_t result = esp_now_send(broadcastAddress, sendBuff, message_length + 4);
    // and this will send a message to a specific device
    /*uint8_t peerAddress[] = {0x3C, 0x71, 0xBF, 0x47, 0xA5, 0xC0};
    esp_now_peer_info_t peerInfo = {};
    memcpy(&peerInfo.peer_addr, peerAddress, 6);
    if (!esp_now_is_peer_exist(peerAddress))
    {
      esp_now_add_peer(&peerInfo);
    }
    esp_err_t result = esp_now_send(peerAddress, (const uint8_t *)message.c_str(), message.length());*/
    if (result == ESP_OK) {
        // Serial.println("Broadcast message success");
        return true;
    } else if (result == ESP_ERR_ESPNOW_NOT_INIT) {
        Serial.println("ESPNOW not Init.");
    } else if (result == ESP_ERR_ESPNOW_ARG) {
        Serial.println("Invalid Argument");
    } else if (result == ESP_ERR_ESPNOW_INTERNAL) {
        Serial.println("Internal Error");
    } else if (result == ESP_ERR_ESPNOW_NO_MEM) {
        Serial.println("ESP_ERR_ESPNOW_NO_MEM");
    } else if (result == ESP_ERR_ESPNOW_NOT_FOUND) {
        Serial.println("Peer not found.");
    } else {
        Serial.println("Unknown error");
    }
    return false;
}

bool joystickDecodeMessage(uint16_t message_length) {
    /* This is the buffer where we will store our message. */

    /* Create a stream that reads from the buffer. */
    pb_istream_t stream = pb_istream_from_buffer(buffer, message_length);

    /* Now we are ready to decode the message. */
    bool status = pb_decode(&stream, JoystickMessage_fields, &_jm);

    /* Check for errors... */
    if (!status) {
        printf("Decoding joystick data failed: %s\n", PB_GET_ERROR(&stream));
        return false;
    }
    if (joystick._pEspNowJoystickCallbacks != nullptr) {
        joystick._pEspNowJoystickCallbacks->onJoystickMsg(_jm);
    }
    // joystick.getInstance()->onJoystickMsg(jm);
    return true;
}

void joystickRecvCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen) {
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
        joystickDecodeMessage(msgLen - 4);
    } else {
        // TODO
    }
}

// callback when data is sent
void joystickSendCallback(const uint8_t *macAddr, esp_now_send_status_t status) {
    if (!joystick.devmode) return;
    char macStr[18];
    formatMacAddress(macAddr, macStr, 18);
    Serial.print("Last Packet Sent to: ");
    Serial.println(macStr);
    Serial.print("Last Packet Send Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

bool telemetryDecodeMessage(TelemetryMessage *tm, uint16_t message_length) {
    /* This is the buffer where we will store our message. */

    /* Create a stream that reads from the buffer. */
    pb_istream_t stream = pb_istream_from_buffer(buffer, message_length);

    /* Now we are ready to decode the message. */
    bool status = pb_decode(&stream, TelemetryMessage_fields, &tm);

    /* Check for errors... */
    if (!status) {
        printf("Decoding telemetry data failed: %s\n", PB_GET_ERROR(&stream));
        return false;
    }
    return true;
}

void telemetryRecvCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen) {
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
        TelemetryMessage tm = TelemetryMessage_init_zero;
        telemetryDecodeMessage(&tm,msgLen - 4);
    } else {
        // TODO
    }
}

void telemetrySendCallback(const uint8_t *macAddr, esp_now_send_status_t status) {
}

String EspNowJoystick::getDeviceId() { 
    uint32_t chipId = 0;
    for (int i = 0; i < 17; i = i + 8) chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    return String(chipId, HEX);
}

bool EspNowJoystick::init(bool debug) {
    devmode = debug;
    WiFi.mode(WIFI_STA);
    // startup ESP Now
    Serial.println("ESPNow Init");
    Serial.println(WiFi.macAddress());
    // shut down wifi
    WiFi.disconnect();

    if (esp_now_init() == ESP_OK) {
        Serial.println("ESPNow Init Success");
        if(_pEspNowJoystickCallbacks != nullptr) {
            esp_now_register_recv_cb(joystickRecvCallback);
            esp_now_register_send_cb(telemetrySendCallback);
            return true;
        }
        else if(_pEspNowTelemetryCallbacks != nullptr) {
            esp_now_register_recv_cb(telemetryRecvCallback);
            esp_now_register_send_cb(joystickSendCallback);
            return true;
        }
        else {
            Serial.println("No callbacks registered");
            return false;
        }
    } else {
        Serial.println("ESPNow Init Failed");
        delay(100);
        return false;
    }
}
