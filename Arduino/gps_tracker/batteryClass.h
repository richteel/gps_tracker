#ifndef BATTERY_H
#define BATTERY_H

#include "Defines.h"
#include "StructsAndEnums.h"

class batteryClass {

public:
  float batteryVoltage;
  float batteryPercent;
  bool usbPresent = false;
  BatteryChargeStates batteryChargeState;

  void begin(uint8_t batteryPin, uint8_t usbPin) {
    analogReadResolution(12);
    _batteryValueOldestIdx = 0;
    _batteryPin = batteryPin;
    _usbPin = usbPin;
    pinMode(_usbPin, INPUT);

    int batteryVoltageRaw = analogRead(_batteryPin);
    // Initialize the rolling avaerage array
    for (int i = 0; i < _batteryValuesLen; i++) {
      _batteryValues[i] = batteryVoltageRaw;
    }
  }

  void updateBattery() {
    int batteryVoltageRaw = analogRead(_batteryPin);
    int sumBatteryVoltageRaw = 0;
    int averageBatteryVoltage = 0;

    usbPresent = digitalRead(_usbPin);

    _batteryValues[_batteryValueOldestIdx] = batteryVoltageRaw;
    _batteryValueOldestIdx++;
    if (_batteryValueOldestIdx >= _batteryValuesLen) {
      _batteryValueOldestIdx = 0;
    }

    for (int i = 0; i < _batteryValuesLen; i++) {
      sumBatteryVoltageRaw += _batteryValues[i];
    }
    averageBatteryVoltage = sumBatteryVoltageRaw / _batteryValuesLen;

    batteryVoltage = 2.0f * (((float)averageBatteryVoltage * 3.3f) / 4096.0f);
    batteryPercent = ((batteryVoltage - minVoltage) / (maxVoltage - minVoltage)) * 100.0f;

    batteryChargeState = BatteryChargeStates::Discharging;
    if (usbPresent) {
      batteryChargeState = BatteryChargeStates::Charging;
      if(batteryPercent >= 100.0) {
        batteryChargeState = BatteryChargeStates::Charged;
        batteryPercent = 100.0;
      }
    }
  }

private:
  static const int _batteryValuesLen = 20;
  int _batteryValues[_batteryValuesLen];
  int _batteryValueOldestIdx;
  static constexpr float minVoltage = 3.35;
  static constexpr float maxVoltage = 4.2;

  uint8_t _batteryPin;
  uint8_t _usbPin;
};


#endif  // BATTERY_H