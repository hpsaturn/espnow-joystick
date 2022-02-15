/************************************************************************
# This file is part of the ESPNow Joystick library.
# https://github.com/hpsaturn/espnow-joystick
# Copyright (c) 2022, @hpsaturn, Antonio Vanegas
# https://hpsaturn.com, All rights reserved.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
*************************************************************************/

#ifndef ESPNOW_JOYSTICK_H
#define ESPNOW_JOYSTICK_H

#include <WiFi.h>
#include <esp_now.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include "comm.pb.h"

#define CSL_VERSION "0.0.7"
#define CSL_REVISION 072

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
    virtual ~EspNowJoystickCallbacks(){};
    virtual void onJoystickMsg(JoystickMessage jm);
    virtual void onError();
};

class EspNowTelemetryCallbacks {
   public:
    virtual ~EspNowTelemetryCallbacks(){};
    virtual void onTelemetryMsg(TelemetryMessage jm);
    virtual void onError();
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_OTAHANDLER)
extern EspNowJoystick joystick;
#endif

#endif
