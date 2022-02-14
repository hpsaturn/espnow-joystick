#ifndef ESPNOW_JOYSTICK_H
#define ESPNOW_JOYSTICK_H

#include <WiFi.h>
#include <esp_now.h>
#include <pb_encode.h>
#include <pb_decode.h>
#include "comm.pb.h"

#define SYSNUM 3

class EspNowJoystickCallbacks;
class EspNowTelemetryCallbacks;
class EspNowJoystick {
    public:
        bool devmode = false;
        JoystickMessage jm = JoystickMessage_init_zero;
        TelemetryMessage tm = TelemetryMessage_init_zero;
        EspNowJoystick();
        bool init(bool debug = false);
        void setJoystickCallbacks(EspNowJoystickCallbacks* pCallbacks);
        void setTelemetryCallbacks(EspNowTelemetryCallbacks* pCallbacks);
        JoystickMessage newJoystickMsg();
        bool sendJoystickMsg(JoystickMessage jm);
        TelemetryMessage newTelemetryMsg();
        bool sendTelemetryMsg(TelemetryMessage tm);
        EspNowJoystick* getInstance();
        EspNowJoystickCallbacks* _pEspNowJoystickCallbacks = nullptr;
        EspNowTelemetryCallbacks* _pEspNowTelemetryCallbacks = nullptr;
    private: 
        String _ESP_ID;
        String getDeviceId();
        bool sendMessage(uint32_t message_length);
        
};

class EspNowJoystickCallbacks {
public:
    virtual ~EspNowJoystickCallbacks() {};
    virtual void onJoystickMsg(JoystickMessage jm);
    virtual void onError();
};

class EspNowTelemetryCallbacks {
public:
    virtual ~EspNowTelemetryCallbacks() {};
    virtual void onTelemetryMsg(TelemetryMessage jm);
    virtual void onError();
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_OTAHANDLER)
extern EspNowJoystick joystick;
#endif

#endif
