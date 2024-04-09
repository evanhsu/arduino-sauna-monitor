/*
 * Sauna Monitor
 * For the Particle Photon 1
 */

 // Include Particle Device OS APIs
#include "Particle.h"

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);

// Run the application and system concurrently in separate threads
SYSTEM_THREAD(ENABLED);

bool success;
float tempReadingInside;
float tempReadingOutside;

// Show system, cloud connectivity, and application logs over USB
// View logs with CLI using 'particle serial monitor --follow'
SerialLogHandler logHandler(LOG_LEVEL_INFO);

bool publishData(float temperatureInside, float temperatureOutside) {
    char buf[256];
    JSONBufferWriter writer(buf, sizeof(buf));

    writer.beginObject();
    writer.name("temperatureInside").value(String::format("%.1f", temperatureInside));
    writer.name("temperatureOutside").value(String::format("%.1f", temperatureOutside));
    writer.endObject();

    writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;

    Log.info(writer.buffer());
    return Particle.publish("sensorReading", writer.buffer());
}

// setup() runs once, when the device is first turned on
void setup() {
    // Put initialization like pinMode and begin functions here
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
    // Publish event to cloud every 10 seconds
    tempReadingInside = 124.3;
    tempReadingOutside = 70.0;
    Log.info("Sending event to the cloud!");


    success = publishData(tempReadingInside, tempReadingOutside);

    if (success == true) {
        Log.info("Successfully published temperature event");
    }
    else {
        Log.error("Failed to publish temperature event");
    }

    delay(10 * 1000); // milliseconds and blocking - see docs for more info!
}
