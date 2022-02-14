#include <Arduino.h>
#include <esp_adc_cal.h>
#include "hal.hpp"

#define BATTERY_MIN_V 3.4
#define BATTERY_MAX_V 4.19
#define BATTCHARG_MIN_V 4.21
#define BATTCHARG_MAX_V 4.80

void setupBattADC();
void setupBattery();
float battGetVoltage();
uint8_t battCalcPercentage(float volts);
void battUpdateChargeStatus();
bool battIsCharging();

uint8_t _calcPercentage(float volts, float max, float min);
