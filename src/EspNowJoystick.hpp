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

#include <Arduino.h>

#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#include <esp_now.h>
#else
#include <ESP8266WiFi.h>
#include <espnow.h>
#define ESP_OK 0
#endif

#include <pb_decode.h>
#include <pb_encode.h>
#include "comm.pb.h"

#include <map>
#include <vector>
#include <string>

#define CSL_VERSION "0.1.5"
#define CSL_REVISION 089

class EspNowJoystickCallbacks;
class EspNowTelemetryCallbacks;

class EspNowJoystick {
   public:

    /// basic constuctor. Please see the init command
    EspNowJoystick();

    /// main init. Perform it after callbacks set
    bool init(bool debug = false);
   
    /// joystick message constuctor
    JoystickMessage newJoystickMsg();
   
    /// receiver message constructor
    TelemetryMessage newTelemetryMsg();

    /// callback setter for the receiver to have joystick messages 
    void setJoystickCallbacks(EspNowJoystickCallbacks* pCallbacks);

    /// callback setter for the joystick to have receiver messages 
    void setTelemetryCallbacks(EspNowTelemetryCallbacks* pCallbacks);

    /// sender messages for the joystick
    bool sendJoystickMsg(JoystickMessage jm);
   
    /// sender messages for the joystick to specific target
    bool sendJoystickMsg(JoystickMessage jm, const uint8_t* mac);

    /// sender messages for the receiver 
    bool sendTelemetryMsg(TelemetryMessage tm);
    
    /// sender messages for the receiver to specific target
    bool sendTelemetryMsg(TelemetryMessage tm, const uint8_t* mac);

    /// print all receivers detected in the current session
    void printReceivers();

    /// utility for print formatted mac address 
    std::string getFormattedMacAddr(const uint8_t *mac);

    /// getter to retreive all receivers vector
    std::vector<uint32_t> getReceivers();

    /// get mac address from the receiverId
    const uint8_t * getReceiverMacAddr(uint32_t receiverId);

    /// get the current instance of this object
    EspNowJoystick* getInstance();

    // Fields:
    
    /// current joystick callback
    EspNowJoystickCallbacks* _pEspNowJoystickCallbacks = nullptr;

    /// current receiver callback
    EspNowTelemetryCallbacks* _pEspNowTelemetryCallbacks = nullptr;
    
    /// current mac target (default: broadcasting)
    uint8_t targetAddress [6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    
    /// devmode for improve verbose output
    bool devmode = false;
    

   private:
    
    String getDeviceId();

    size_t encodeJoystickMsg(JoystickMessage jm);
    
    size_t encodeTelemetryMsg(TelemetryMessage tm);

    void reportError(const char *msg);

    bool sendMessage(uint32_t msglen);

    bool sendMessage(uint32_t msglen, const uint8_t *mac);
    
    String _ESP_ID;
};

/// callback class for the receiver. See examples
class EspNowJoystickCallbacks {
   public:
    virtual ~EspNowJoystickCallbacks(){};
    virtual void onJoystickMsg(JoystickMessage jm);
    virtual void onError(const char *msg);
};

/// callback class for the joystick. See examples
class EspNowTelemetryCallbacks {
   public:
    virtual ~EspNowTelemetryCallbacks(){};
    virtual void onTelemetryMsg(TelemetryMessage jm);
    virtual void onError(const char *msg);
};

/// global instance for the receiver and the joystick
#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_OTAHANDLER)
extern EspNowJoystick joystick;
#endif

#endif
