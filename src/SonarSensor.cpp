#include "SonarSensor.h"

SonarSensor::SonarSensor(uint8_t trigPin, uint8_t echoPin, unsigned long timeout)
    : _trigPin(trigPin), _echoPin(echoPin), _timeout(timeout) {}

void SonarSensor::begin()
{
    pinMode(_trigPin, OUTPUT);
    digitalWrite(_trigPin, LOW);
    pinMode(_echoPin, INPUT);
}

unsigned long SonarSensor::pingMicroseconds()
{
    // Ensure trigger is low
    digitalWrite(_trigPin, LOW);
    delayMicroseconds(2);

    // Send 10us pulse to trigger
    digitalWrite(_trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(_trigPin, LOW);

    // Read echo pulse width (microseconds)
    unsigned long duration = pulseIn(_echoPin, HIGH, _timeout);
    return duration; // 0 if timeout
}

float SonarSensor::pingCM()
{
    unsigned long duration = pingMicroseconds();
    if (duration == 0)
        return -1.0f;
    // speed of sound ~343 m/s -> 0.0343 cm/us; divide by 2 for round trip
    float distanceCm = (duration * 0.0343f) / 2.0f;
    return distanceCm;
}

bool SonarSensor::belowThresholdFor(float threshold_cm, unsigned long hold_ms)
{
    float d = pingCM();
    unsigned long now = millis();

    if (d < 0)
    {
        // treat timeout as not-below threshold; reset state
        _belowActive = false;
        _belowStartMillis = 0;
        return false;
    }

    if (d <= threshold_cm)
    {
        if (!_belowActive)
        {
            _belowActive = true;
            _belowStartMillis = now;
            return false;
        }
        else
        {
            if ((now - _belowStartMillis) >= hold_ms)
            {
                // keep returning true once threshold has been held
                return true;
            }
            return false;
        }
    }
    else
    {
        // above threshold: reset
        _belowActive = false;
        _belowStartMillis = 0;
        return false;
    }
}

bool SonarSensor::belowThresholdFor(float threshold_cm)
{
    return belowThresholdFor(threshold_cm, 1000UL);
}

#if defined(ESP32)
// Background task function (runs in its own FreeRTOS task)
void SonarSensor::_bgTask(void *pvParameters)
{
    SonarSensor *sensor = reinterpret_cast<SonarSensor *>(pvParameters);
    if (!sensor)
        vTaskDelete(NULL);

    while (sensor->_bgRunning)
    {
        unsigned long dur = sensor->pingMicroseconds();
        float d = -1.0f;
        if (dur != 0)
        {
            d = (dur * 0.0343f) / 2.0f;
        }
        // atomic write to volatile
        sensor->_lastDistance = d;

        // sleep for interval
        vTaskDelay(pdMS_TO_TICKS(sensor->_bgIntervalMs));
    }

    sensor->_bgTaskHandle = NULL;
    vTaskDelete(NULL);
}

void SonarSensor::startBackground(unsigned long intervalMs)
{
    if (_bgRunning)
        return;
    _bgIntervalMs = intervalMs;
    _bgRunning = true;
    _lastDistance = -1.0f;
    // create task pinned to core 1 to avoid interfering with WiFi (core 0)
    xTaskCreatePinnedToCore(SonarSensor::_bgTask, "sonar_bg", 2048, this, 1, &_bgTaskHandle, 1);
}

void SonarSensor::stopBackground()
{
    if (!_bgRunning)
        return;
    _bgRunning = false;
    // wait for task to clear handle (optional small delay)
    unsigned long start = millis();
    while (_bgTaskHandle != NULL && (millis() - start) < 500)
    {
        delay(10);
    }
}

float SonarSensor::getLastDistance()
{
    return _lastDistance;
}
#endif
