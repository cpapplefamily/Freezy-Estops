#include "battery.h"

static float voltageReadings[NUM_SAMPLES];
static int readingIndex = 0;
static float lastSmoothedVoltage = 0;

void initBattery() {
  // Configure ADC
  analogReadResolution(12);  // 12-bit resolution
  analogSetAttenuation(ADC_11db);  // Full range: 0-3.3V

  // Initialize voltage readings array
  for (int i = 0; i < NUM_SAMPLES; i++) {
    voltageReadings[i] = 0;
  }
}

float readBatteryVoltage() {
  // Read ADC value
  int adcValue = analogRead(BATTERY_PIN);

  // Convert ADC to voltage at divider output
  float adcVoltage = (adcValue / ADC_RESOLUTION) * ADC_REFERENCE;

  // Calculate actual battery voltage
  float batteryVoltage = adcVoltage * VOLTAGE_DIVIDER_RATIO;

  // Check for large voltage change (battery connected/disconnected)
  float voltageDifference = abs(batteryVoltage - lastSmoothedVoltage);
  if (voltageDifference > VOLTAGE_CHANGE_THRESHOLD) {
    // Reset smoothing buffer to current voltage
    for (int i = 0; i < NUM_SAMPLES; i++) {
      voltageReadings[i] = batteryVoltage;
    }
    readingIndex = 0;
    lastSmoothedVoltage = batteryVoltage;
    return batteryVoltage;
  }

  // Smooth the reading
  voltageReadings[readingIndex] = batteryVoltage;
  readingIndex = (readingIndex + 1) % NUM_SAMPLES;

  float sum = 0;
  for (int i = 0; i < NUM_SAMPLES; i++) {
    sum += voltageReadings[i];
  }

  lastSmoothedVoltage = sum / NUM_SAMPLES;
  return lastSmoothedVoltage;
}

float calculatePercentage(float voltage) {
  // Linear interpolation between empty and full
  float percentage = ((voltage - BATTERY_EMPTY) / (BATTERY_FULL - BATTERY_EMPTY)) * 100.0;

  // Constrain to 0-100%
  if (percentage > 100) percentage = 100;
  if (percentage < 0) percentage = 0;

  return percentage;
}
