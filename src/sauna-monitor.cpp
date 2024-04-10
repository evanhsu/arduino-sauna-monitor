/**
 * Sauna Monitor
 * For the Particle Photon 1
 *
 * View logs with CLI using 'particle serial monitor --follow'
 */

#include "Particle.h"
#include "DS18.h"

DS18 sensor(D1);

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);

// Run the application and system concurrently in separate threads
SYSTEM_THREAD(ENABLED);

bool success;
uint8_t addressInsideSensor[9];
uint8_t addressOutsideSensor[9];
float tempReadingInside;
float tempReadingOutside;

SerialLogHandler logHandler(LOG_LEVEL_INFO);

bool publishData(float temperatureInside, float temperatureOutside) {
    unsigned long timestampMs = Time.now() * 1000;
    char buf[256];
    JSONBufferWriter writer(buf, sizeof(buf));

    writer.beginObject();
    writer.name("temperatureInside").value(temperatureInside);
    writer.name("temperatureOutside").value(temperatureOutside);
    writer.name("timestampMs").value(timestampMs);
    writer.endObject();

    writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;

    Log.info(writer.buffer());
    return Particle.publish("sauna/temperature", writer.buffer());
}

float getInsideTemp() {
    if (sensor.read(addressInsideSensor)) {
        return sensor.fahrenheit();
    }
    else {
        return 0.0;
    }
}

float getOutsideTemp() {
    if (sensor.read(addressOutsideSensor)) {
        return sensor.fahrenheit();
    }
    else {
        return 0.0;
    }
}

// setup() runs once, when the device is first turned on
void setup() {
    // Read the first sensor on the 1-wire bus
    if (sensor.read()) {
        sensor.addr(addressInsideSensor); // Store the address of the INSIDE sensor (the last-read sensor);
    }
    else {
        Log.error("Couldn't find the inside temperature probe");
    }

    // Read the next sensor on the 1-wire bus
    if (sensor.read()) {
        sensor.addr(addressOutsideSensor); // Store the address of the OUTSIDE sensor (the last-read sensor);
    }
    else {
        Log.error("Couldn't find the outside temperature probe");
    }
}

void loop() {
    tempReadingInside = getInsideTemp();
    tempReadingOutside = getOutsideTemp();

    success = publishData(tempReadingInside, tempReadingOutside);

    if (success != true) {
        Log.error("Failed to publish temperature event");
    }

    delay(10 * 1000); // Wait for 10 seconds
}
