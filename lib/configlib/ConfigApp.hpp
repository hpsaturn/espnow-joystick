#ifndef CanfigApp_hpp
#define CanfigApp_hpp

#include <Preferences.h>

class ConfigApp {
   public:
    
    void init(const char app_name[] = "");

    void saveString(String key, String value);

    String loadString(String key);

    void saveInt(String key, int value);

    void saveBool(String key, bool value);

    void reload();

    void clear();

    void setDebugMode(bool enable);

   private:
    ///preferences main key
    char* _app_name;
    ///ESP32 preferences abstraction
    Preferences preferences;
    ///last key saved (for callback)
    bool devmode;

    void DEBUG(const char* text, const char* textb = "");

    // @todo use DEBUG_ESP_PORT ?
#ifdef WM_DEBUG_PORT
    Stream& _debugPort = WM_DEBUG_PORT;
#else
    Stream& _debugPort = Serial;  // debug output stream ref
#endif
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_CFGHANDLER)
extern ConfigApp cfg;
#endif

#endif