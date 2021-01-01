#include "ConfigApp.hpp"

void ConfigApp::init(const char app_name[]) {
    if (strlen(app_name)==0){
        uint64_t chipid = ESP.getEfuseMac();
        String str = "_PREF" + String((uint32_t)(chipid >> 32), HEX);
    }
    _app_name = new char[strlen(app_name) + 1];
    strcpy(_app_name, app_name);
    // override with debug INFO level (>=3)
    #ifdef CORE_DEBUG_LEVEL
    if (CORE_DEBUG_LEVEL>=3) devmode = true;  
    #endif
    if (devmode) Serial.println("-->[CONFIG] debug is enable.");
    reload();
}

void ConfigApp::reload() {
    preferences.begin(_app_name, false);
    preferences.end();
}

void ConfigApp::saveString(String key, String value){
    preferences.begin(_app_name, false);
    preferences.putString(key.c_str(), value.c_str());
    preferences.end();
}

String ConfigApp::loadString(String key){
    preferences.begin(_app_name, false);
    String out = preferences.getString(key.c_str(), "");
    preferences.end();
    return out;
}

void ConfigApp::saveInt(String key, int value){
    preferences.begin(_app_name, false);
    preferences.putInt(key.c_str(), value);
    preferences.end();
}

void ConfigApp::saveBool(String key, bool value){
    preferences.begin(_app_name, false);
    preferences.putBool(key.c_str(), value);
    preferences.end();
}

void ConfigApp::clear() {
    preferences.begin(_app_name, false);
    preferences.clear();
    preferences.end();
}

void ConfigApp::setDebugMode(bool enable){
    devmode = enable;
}

void ConfigApp::DEBUG(const char *text, const char *textb) {
    if (devmode) {
        _debugPort.print(text);
        if (textb) {
            _debugPort.print(" ");
            _debugPort.print(textb);
        }
        _debugPort.println();
    }
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_CFGHANDLER)
ConfigApp cfg;
#endif
