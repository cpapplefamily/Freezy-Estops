#ifndef SONARSENSOR_H
#define SONARSENSOR_H

#include <Arduino.h>

class SonarSensor
{
public:
    SonarSensor(uint8_t trigPin, uint8_t echoPin, unsigned long timeout = 30000UL);
    void begin();
    // Returns distance in centimeters, or -1.0 on timeout/no-echo
    float pingCM();
    // Returns pulse duration in microseconds, or 0 on timeout
    unsigned long pingMicroseconds();
    // Returns true when measured distance stays below `threshold_cm` continuously
    // for at least `hold_ms` milliseconds. Should be called periodically (e.g., in loop()).
    bool belowThresholdFor(float threshold_cm, unsigned long hold_ms);
    // Convenience overload: hold time defaults to 1000 ms (1 second)
    bool belowThresholdFor(float threshold_cm);

#if defined(ESP32)
    // Start background measurement task (non-blocking for main loop).
    // intervalMs: measurement interval in milliseconds.
    void startBackground(unsigned long intervalMs = 200);
    void stopBackground();
    // Returns last measured distance (cm) from background task, or -1.0 if none.
    float getLastDistance();
#endif

private:
    uint8_t _trigPin;
    uint8_t _echoPin;
    unsigned long _timeout;
    // State for below-threshold detection
    bool _belowActive = false;
    unsigned long _belowStartMillis = 0;
#if defined(ESP32)
    // Background task state (ESP32 only)
    volatile float _lastDistance = -1.0f;
    volatile bool _bgRunning = false;
    TaskHandle_t _bgTaskHandle = NULL;
    unsigned long _bgIntervalMs = 200;
    static void _bgTask(void *pvParameters);
#endif
};

#endif // SONARSENSOR_H
