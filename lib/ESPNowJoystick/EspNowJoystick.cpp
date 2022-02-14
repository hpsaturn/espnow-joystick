#include <EspNowJoystick.hpp>

JoystickMessage _jm = JoystickMessage_init_zero;
TelemetryMessage _tm = TelemetryMessage_init_zero;

uint8_t send_buffer[256];
uint8_t recv_buffer[256];

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

void printBuffer(uint8_t *buffer, uint32_t length) {
    for (uint32_t i = 0; i < length; i++) {
        Serial.printf("%02X", buffer[i]);
    }
    Serial.println();
}

void printMacAddress(const uint8_t *macAddress) {
    char macStr[18];
    formatMacAddress(macAddress, macStr, 18);
    Serial.println(macStr);
}

bool EspNowJoystick::sendJoystickMsg(JoystickMessage jm) {
    pb_ostream_t stream = pb_ostream_from_buffer(send_buffer, sizeof(send_buffer));
    bool status = pb_encode(&stream, JoystickMessage_fields, &jm);
    size_t message_length = stream.bytes_written;
    if (!status) {
        printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
        return false;
    }
    return sendMessage(message_length);
}

bool joystickDecodeMessage(uint16_t message_length) {
    pb_istream_t stream = pb_istream_from_buffer(recv_buffer, message_length);
    bool status = pb_decode(&stream, JoystickMessage_fields, &_jm);
    if (!status) {
        printf("Decoding joystick msg failed: %s\n", PB_GET_ERROR(&stream));
        return false;
    }
    if (joystick._pEspNowJoystickCallbacks != nullptr) {
        joystick._pEspNowJoystickCallbacks->onJoystickMsg(_jm);
    }
    return true;
}

void joystickRecvCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen) {
    int msgLen = min(ESP_NOW_MAX_DATA_LEN, dataLen);
    memcpy(recv_buffer, data, msgLen); 
    joystickDecodeMessage(msgLen);
}

// callback when data is sent
void joystickSendCallback(const uint8_t *macAddr, esp_now_send_status_t status) {
    if (!joystick.devmode) return;
    printMacAddress(macAddr); 
    Serial.print("Last Packet Send Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

bool EspNowJoystick::sendTelemetryMsg(TelemetryMessage tm) {
    pb_ostream_t stream = pb_ostream_from_buffer(send_buffer, sizeof(send_buffer));
    bool status = pb_encode(&stream, TelemetryMessage_fields, &tm);
    size_t message_length = stream.bytes_written;
    if (!status) {
        printf("Encoding Telemetry msg failed: %s\n", PB_GET_ERROR(&stream));
        return false;
    }
    return sendMessage(message_length);
}

bool telemetryDecodeMessage(uint16_t message_length) {
    pb_istream_t stream = pb_istream_from_buffer(recv_buffer, message_length);
    bool status = pb_decode(&stream, TelemetryMessage_fields, &_tm);
    if (!status) {
        printf("Decoding telemetry msg failed: %s\n", PB_GET_ERROR(&stream));
        return false;
    }
    if (joystick._pEspNowTelemetryCallbacks != nullptr) {
        joystick._pEspNowTelemetryCallbacks->onTelemetryMsg(_tm);
    }
    return true;
}

void telemetryRecvCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen) {
    int msgLen = min(ESP_NOW_MAX_DATA_LEN, dataLen);
    memcpy(recv_buffer, data, msgLen); 
    telemetryDecodeMessage(msgLen);
}

void telemetrySendCallback(const uint8_t *macAddr, esp_now_send_status_t status) {
}

bool EspNowJoystick::sendMessage(uint32_t msglen) {
    // this will broadcast a message to everyone in range
    uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    esp_now_peer_info_t peerInfo = {};
    memcpy(&peerInfo.peer_addr, broadcastAddress, 6);
    if (!esp_now_is_peer_exist(broadcastAddress)) {
        esp_now_add_peer(&peerInfo);
    }
    esp_err_t result = esp_now_send(broadcastAddress, send_buffer, msglen);
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
        if (joystick.devmode) {
            Serial.println("Broadcast message success");
            Serial.printf("Send message size: %i\n", msglen);
            printBuffer(send_buffer, msglen);
        }
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

JoystickMessage EspNowJoystick::newJoystickMsg() {
    return jm;
}

TelemetryMessage EspNowJoystick::newTelemetryMsg() {
    return tm;
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
    delay(100);

    if (esp_now_init() == ESP_OK) {
        Serial.println("ESPNow Init Success");
        if(_pEspNowJoystickCallbacks != nullptr) {
            esp_now_register_recv_cb(joystickRecvCallback);
            // esp_now_register_send_cb(telemetrySendCallback);
            return true;
        }
        else if(_pEspNowTelemetryCallbacks != nullptr) {
            esp_now_register_recv_cb(telemetryRecvCallback);
            // esp_now_register_send_cb(joystickSendCallback);
            return true;
        }
        else {
            Serial.println("No callbacks registered");
            return false;
        }
    } else {
        Serial.println("ESPNow Init Failed");
        delay(100);
        ESP.restart();
        return false;
    }
}
