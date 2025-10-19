#include "SonarSensor.h"

SonarSensor::SonarSensor(uint8_t trigPin, uint8_t echoPin, unsigned long timeout)
    : _trigPin(trigPin), _echoPin(echoPin), _timeout(timeout), _minOffMs(0), _bgRunning(false), _bgIntervalMs(20), _lastDistance(-1.0f), _bgTaskHandle(NULL), _inCooldown(false), _belowActive(false), _offStartMillis(0), _belowStartMillis(0) {}

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
    return pulseIn(_echoPin, HIGH, _timeout); // 0 if timeout
}

float SonarSensor::pingCM()
{
    unsigned long duration = pingMicroseconds();
    if (duration == 0)
        return -1.0f;

    // speed of sound ~343 m/s -> 0.0343 cm/us; divide by 2 for round trip
    return (duration * 0.0343f) / 2.0f; // cm
}

void SonarSensor::setMinOffTime(unsigned long minOffMs)
{
    _minOffMs = minOffMs;
}

bool SonarSensor::belowThresholdFor(unsigned long threshold_cm, unsigned long hold_ms)
{
    float d = getLastDistance(); // Use background distance for consistency
    unsigned long now = millis();

    // treat timeout as "no object" (above threshold)
    bool isBelow = (d >= 0) && (d <= threshold_cm);

    // If in cooldown (we recently reported true) we must see a continuous "cleared" period
    // of _minOffMs before allowing a new true.
    if (_inCooldown)
    {
        if (!isBelow)
        {
            // sensor reports cleared while in cooldown; start or continue off timer
            if (_offStartMillis == 0)
            {
                _offStartMillis = now;
            }
            else if ((now - _offStartMillis) >= _minOffMs)
            {
                // cooled off sufficiently -> leave cooldown
                _inCooldown = false;
                _offStartMillis = 0;
                // continue processing detection as normal after cooldown cleared
            }
        }
        else
        {
            // object detected again while in cooldown -> reset off timer
            _offStartMillis = 0;
        }

        // while still in cooldown do not report new true
        if (_inCooldown)
        {
            // but keep internal belowActive state reset so hold timer starts fresh when allowed
            _belowActive = false;
            _belowStartMillis = 0;
            return false;
        }
    }

    // normal detection path when not in cooldown
    if (!isBelow)
    {
        // above threshold: reset below tracking
        _belowActive = false;
        _belowStartMillis = 0;
        return false;
    }

    // isBelow == true
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
            // We have sustained below threshold long enough -> report true and enter cooldown if configured
            if (_minOffMs > 0)
            {
                _inCooldown = true;
                _offStartMillis = 0; // will be set when cleared
            }
            // keep belowActive true (optional) and return true
            return true;
        }
        return false;
    }
}

bool SonarSensor::belowThresholdFor(unsigned long threshold_cm)
{
    return belowThresholdFor(threshold_cm, 40UL); // Align with ~2 pings at 20ms
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
        float d = (dur == 0) ? -1.0f : (dur * 0.0343f) / 2.0f;
        sensor->_lastDistance = d; // Atomic write to volatile
        vTaskDelay(pdMS_TO_TICKS(sensor->_bgIntervalMs)); // 20ms = 50Hz
    }

    sensor->_bgTaskHandle = NULL;
    vTaskDelete(NULL);
}

void SonarSensor::startBackground(unsigned long intervalMs)
{
    if (_bgRunning)
        return;
    _bgIntervalMs = (intervalMs < 20) ? 20 : intervalMs; // Enforce 20ms minimum
    _bgRunning = true;
    _lastDistance = -1.0f;
    // create task pinned to core 0
    xTaskCreatePinnedToCore(_bgTask, "sonar_bg", 2048, this, 3, &_bgTaskHandle, 0); // Priority 3, Core 0
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