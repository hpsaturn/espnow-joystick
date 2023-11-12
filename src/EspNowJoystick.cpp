#include <EspNowJoystick.hpp>

JoystickMessage _jm = JoystickMessage_init_zero;
TelemetryMessage _tm = TelemetryMessage_init_zero;

/// general buffer for msg sender
uint8_t send_buffer[256];

/// general buffer for receive msgs
uint8_t recv_buffer[256];

/// receivers map (id,macaddr)
std::map<uint32_t, std::string> amp;

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

uint32_t getReceiverId(const uint8_t *macAddr){
    return macAddr[0]+macAddr[1]+macAddr[2]+macAddr[3]+macAddr[4]+macAddr[5];
}

void formatMacAddress(const uint8_t *macAddr, char *buffer, int maxLength) {
    snprintf(buffer, maxLength, "%02x:%02x:%02x:%02x:%02x:%02x", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
}

std::string EspNowJoystick::getFormattedMacAddr(const uint8_t *macAddress){
    char macStr[18];
    formatMacAddress(macAddress, macStr, 18);
    return std::string(macStr);
}

void printMacAddress(const uint8_t * macAddress){
    char macStr[18];
    formatMacAddress(macAddress, macStr, 18);
    Serial.println(macStr);
}

void printBuffer(uint8_t *buffer, uint32_t length) {
    for (uint32_t i = 0; i < length; i++) {
        Serial.printf("%02X", buffer[i]);
    }
    Serial.println();
}

bool EspNowJoystick::sendJoystickMsg(JoystickMessage jm) {
    return sendMessage(encodeJoystickMsg(jm));
}

bool EspNowJoystick::sendJoystickMsg(JoystickMessage jm, const uint8_t* mac){
    memcpy(&targetAddress, mac, 6);
    return sendMessage(encodeJoystickMsg(jm), mac);
}

size_t EspNowJoystick::encodeJoystickMsg(JoystickMessage jm) {
    pb_ostream_t stream = pb_ostream_from_buffer(send_buffer, sizeof(send_buffer));
    bool status = pb_encode(&stream, JoystickMessage_fields, &jm);
    size_t message_length = stream.bytes_written;
    if (!status) {
        printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
        return 0;
    }
    return message_length;
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

#ifdef ARDUINO_ARCH_ESP32
void joystickRecvCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen) {
#else
void joystickRecvCallback(uint8_t *macAddr, uint8_t *data, uint8_t dataLen) {
#endif
    #ifdef ARDUINO_ARCH_ESP32
    int msgLen = min(ESP_NOW_MAX_DATA_LEN, dataLen);
    #else
    int msgLen = dataLen;
    #endif
    memcpy(recv_buffer, data, msgLen); 
    joystickDecodeMessage(msgLen);
    if (joystick.devmode) printMacAddress(macAddr);
}

// callback when data is sent. Not necessary for now. 
void joystickSendCallback(const uint8_t *macAddr, int status) {
    // if (!joystick.devmode) return;
    // printMacAddress(macAddr); 
    // Serial.print("Last Packet Send Status: ");
    // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

bool EspNowJoystick::sendTelemetryMsg(TelemetryMessage tm) {
    return sendMessage(encodeTelemetryMsg(tm));
}

bool EspNowJoystick::sendTelemetryMsg(TelemetryMessage tm, const uint8_t* mac){
    return sendMessage(encodeTelemetryMsg(tm), mac);
}

size_t EspNowJoystick::encodeTelemetryMsg(TelemetryMessage tm) {
    pb_ostream_t stream = pb_ostream_from_buffer(send_buffer, sizeof(send_buffer));
    bool status = pb_encode(&stream, TelemetryMessage_fields, &tm);
    size_t message_length = stream.bytes_written;
    if (!status) {
        printf("Encoding Telemetry msg failed: %s\n", PB_GET_ERROR(&stream));
        return 0;
    }
    return message_length;
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

bool checkReceiver(const uint8_t *macAddr) {
  std::map<uint32_t, std::string>::const_iterator iter;
  iter = amp.find(getReceiverId(macAddr));
  if (iter != amp.end()) return true;
  return false;
}

std::vector<uint32_t> EspNowJoystick::getReceivers() {
  std::vector<uint32_t> vints;
  for (auto const &imap : amp)
    vints.push_back(imap.first);
  return vints;
}

/// returns the MAC address of receiver with this id
const uint8_t * EspNowJoystick::getReceiverMacAddr(uint32_t receiverId) {
  std::map<uint32_t, std::string>::const_iterator iter;
  iter = amp.find(receiverId);
  if (iter != amp.end()) return (const uint8_t *)(iter->second.c_str());
  return nullptr;
}

void EspNowJoystick::printReceivers() {
  for (const auto& ka: amp) {
    char macStr[18];
    formatMacAddress((const uint8_t *)ka.second.c_str(), macStr, 18);
    Serial.printf("receiverId: %i [%s]\r\n",ka.first,macStr);
  }
}

void saveReceiver(const uint8_t *macAddr) {
  if (!checkReceiver(macAddr)) {
    uint32_t id = getReceiverId(macAddr);
    Serial.printf("[%02d] New receiverId: %i with MAC: ", amp.size()+1, id);
    printMacAddress(macAddr);
    amp.insert(std::make_pair(id, std::string((const char *)macAddr)));
  }
}

#ifdef ARDUINO_ARCH_ESP32
void telemetryRecvCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen) {
#else
void telemetryRecvCallback(uint8_t *macAddr, uint8_t *data, uint8_t dataLen) {
#endif
    saveReceiver(macAddr);
    // if (memcmp(joystick.targetAddress, macAddr, 6) != 0) return;
    #ifdef ARDUINO_ARCH_ESP32
    int msgLen = min(ESP_NOW_MAX_DATA_LEN, dataLen);
    #else
    int msgLen = dataLen;
    #endif
    memcpy(recv_buffer, data, msgLen); 
    telemetryDecodeMessage(msgLen);
    if(joystick.devmode) printMacAddress(macAddr);
}

void telemetrySendCallback(const uint8_t *macAddr, int status) {
}

bool EspNowJoystick::sendMessage(uint32_t msglen) {
    return sendMessage(msglen, targetAddress);    
}

bool EspNowJoystick::sendMessage(uint32_t msglen, const uint8_t *mac) {
    #ifdef ARDUINO_ARCH_ESP32
    esp_now_peer_info_t peerInfo = {};
    memcpy(&peerInfo.peer_addr, mac, 6);
    if (!esp_now_is_peer_exist(mac)) {
        esp_now_add_peer(&peerInfo);
    }
    esp_err_t result = esp_now_send(mac, send_buffer, msglen);
    #else // ESP8266
    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
    esp_now_add_peer((uint8 *)mac, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
    int result = esp_now_send((uint8 *) mac, (uint8_t *) send_buffer, msglen);
    #endif
    if (result == ESP_OK) {
        if (joystick.devmode) {
            Serial.println("Broadcast message success");
            printMacAddress(mac);
            Serial.printf("Send message size: %i\n", msglen);
            printBuffer(send_buffer, msglen);
        }
        return true;
    
    #ifdef ARDUINO_ARCH_ESP32
    } else if (result == ESP_ERR_ESPNOW_NOT_INIT) {
        reportError("ESPNOW not Init.");
    } else if (result == ESP_ERR_ESPNOW_ARG) {
        reportError("Invalid Argument");
    } else if (result == ESP_ERR_ESPNOW_INTERNAL) {
        reportError("Internal Error");
    } else if (result == ESP_ERR_ESPNOW_NO_MEM) {
        reportError("ESP_ERR_ESPNOW_NO_MEM");
    } else if (result == ESP_ERR_ESPNOW_NOT_FOUND) {
        reportError("Peer not found.");
    #endif
    } else {
        reportError("Unknown error");
    }
    return false;
}

void EspNowJoystick::reportError(const char *msg) {
    if (devmode) Serial.println(msg);
    if (joystick._pEspNowTelemetryCallbacks != nullptr) {
        joystick._pEspNowTelemetryCallbacks->onError(msg);
    }
    if (joystick._pEspNowJoystickCallbacks != nullptr) {
        joystick._pEspNowJoystickCallbacks->onError(msg);
    }
}

JoystickMessage EspNowJoystick::newJoystickMsg() {
    JoystickMessage jm = JoystickMessage_init_zero;
    return jm;
}

TelemetryMessage EspNowJoystick::newTelemetryMsg() {
    TelemetryMessage tm = TelemetryMessage_init_zero;
    return tm;
}

String EspNowJoystick::getDeviceId() { 
    uint32_t chipId = 0;
    #ifdef ARDUINO_ARCH_ESP32
    for (int i = 0; i < 17; i = i + 8) chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    #else
    for (int i = 0; i < 17; i = i + 8) chipId |= ((ESP.getChipId() >> (40 - i)) & 0xff) << i;
    #endif
    return String(chipId, HEX);
}

bool EspNowJoystick::init(bool debug) {
    devmode = debug;
    WiFi.mode(WIFI_STA);
    // startup ESP Now
    Serial.println("ESPNow Init");
    Serial.println(WiFi.macAddress());
    // shutdown wifi
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
