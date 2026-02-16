#ifndef BATTERY_H
#define BATTERY_H

#include <Arduino.h>

// Pin definitions
#define BATTERY_PIN 1  // GPIO01

// Voltage divider constants
#define R1 68000.0  // 68kΩ
#define R2 22000.0  // 22kΩ
#define VOLTAGE_DIVIDER_RATIO ((R1 + R2) / R2)  // ~4.09

// ADC settings
#define ADC_RESOLUTION 4095.0  // 12-bit ADC
#define ADC_REFERENCE 3.3      // ESP32 ADC reference voltage

// Battery voltage thresholds (Lead-Acid)
// Adjust these for your battery type
#define BATTERY_FULL 12.6   // 100% charge
#define BATTERY_EMPTY 10.5  // 0% charge (don't discharge below this!)

// For LiFePO4 batteries, use:
// #define BATTERY_FULL 14.6
// #define BATTERY_EMPTY 10.0
// And change R1 to 82kΩ

// Smoothing
#define NUM_SAMPLES 20
#define VOLTAGE_CHANGE_THRESHOLD 6.0  // Bypass smoothing if change exceeds this

void initBattery();
float readBatteryVoltage();
float calculatePercentage(float voltage);

#endif
