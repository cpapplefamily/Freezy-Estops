#ifndef SONARSENSOR_H
#define SONARSENSOR_H

#include <Arduino.h>

class SonarSensor
{
public:
    // timeout for pulseIn (us)
    SonarSensor(uint8_t trigPin, uint8_t echoPin, unsigned long timeout = 30000UL);

    void begin();

    // blocking single ping (microseconds)
    unsigned long pingMicroseconds();

    // blocking distance in cm (-1 on timeout)
    float pingCM();

    // non-blocking style helper:
    // return true if distance <= threshold_cm for at least hold_ms milliseconds
    // enforces a minimum-off window (minOffMs) between true events if configured
    bool belowThresholdFor(unsigned long threshold_cm, unsigned long hold_ms);

    // default hold_ms = 1000ms
    bool belowThresholdFor(unsigned long threshold_cm);

    // set minimum off time (ms) required after a true event before a new true is allowed
    void setMinOffTime(unsigned long minOffMs);

#if defined(ESP32)
    void startBackground(unsigned long intervalMs);
    void stopBackground();
    float getLastDistance();
#endif

private:
    uint8_t _trigPin;
    uint8_t _echoPin;
    unsigned long _timeout;

    // tracking for hold detection
    bool _belowActive = false;
    unsigned long _belowStartMillis = 0;

    // minimum off-time (refractory) after a triggered event
    unsigned long _minOffMs = 0;
    bool _inCooldown = false;
    unsigned long _offStartMillis = 0;   // when sensor first reported cleared while in cooldown

#if defined(ESP32)
    // background task support
    volatile float _lastDistance = -1.0f;
    volatile bool _bgRunning = false;
    unsigned long _bgIntervalMs = 100;
    TaskHandle_t _bgTaskHandle = NULL;
    static void _bgTask(void *pvParameters);
    unsigned long _bgIntervalMs_saved = 100;
#endif
};

#endif // SONARSENSOR_H