/**
 * Sauna Monitor
 * For the Particle Photon 1
 *
 * View logs with CLI using 'particle serial monitor --follow'
 */

#include "Particle.h"
#include "DS18.h"
#include "neopixel.h"

#define LED_COUNT 60
#define LED_PIN D1
#define LED_TYPE WS2812B
#define LED_BRIGHTNESS 50 // 0 - 255

DS18 sensor(D0);

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, LED_TYPE);
uint32_t COLOR_0 = strip.Color(90, 90, 255);
uint32_t COLOR_1 = strip.Color(255, 255, 255);
uint32_t COLOR_2 = strip.Color(255, 252, 11);
uint32_t COLOR_3 = strip.Color(255, 140, 5);
uint32_t COLOR_4 = strip.Color(255, 10, 0);

uint temp_1 = 80;
uint temp_2 = 110;
uint temp_3 = 130;
uint temp_4 = 150;

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);

// Run the application and system concurrently in separate threads
SYSTEM_THREAD(ENABLED);

bool success;
uint8_t addressInsideSensor[9];
uint8_t addressOutsideSensor[9];

/**
 * Current (most recent) temperature inside the sauna
*/
float t_inside;

/**
 * Current (most recent) temperature outside the sauna (ambient)
*/
float t_outside;

/**
 * Difference between the temperature inside and outside the sauna
*/
float t_differential;

/**
 * Change in temperature inside the sauna since the last sample
*/
float deltaT_inside;

/**
 * Change in temperature outside the sauna since the last sample
*/
float deltaT_outside;

/**
 * Previous value of t_inside
*/
float t_insidePrevious = -100;

/**
 * Previous value of t_outside
*/
float t_outsidePrevious = -100;

/**
 * The value of t_inside that was most-recently broadcast.
 * Not every reading gets published - an event is only broadcast if the temperature changes by at
 * least 2 degrees since the last broadcast (even if there's only a small temperature change in between
 * individual temperature readings)
*/
float t_atLastBroadcast;

String phase = "OFF";

SerialLogHandler logHandler(LOG_LEVEL_INFO);

bool publishData(float temperatureInside, float temperatureOutside, String phase) {
    char timestampChar[10];
    char buf[256];
    JSONBufferWriter writer(buf, sizeof(buf) - 1);

    unsigned long timestamp = Time.now();
    sprintf(timestampChar, "%lu", timestamp);

    writer.beginObject();
    writer.name("temperatureInside").value(temperatureInside);
    writer.name("temperatureOutside").value(temperatureOutside);
    writer.name("phase").value(phase);
    writer.name("timestamp").value(timestampChar);
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

void colorWipe(uint32_t c, uint8_t wait) {
    for (uint16_t i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, c);
        strip.show();
        delay(wait);
    }
}

// void breath(uint32_t c, uint8_t wait) {

//     Log.info("Starting to breath");
//     for (uint8_t b = 0; b <= 255; b++) {
//         Log.info("ramping up");
//         // Set all pixels to the new brightness (simultaneously)
//         strip.setBrightness(b);
//         for (uint16_t i = 0; i < strip.numPixels(); i++) {
//             strip.setPixelColor(i, c);
//         }
//         strip.show();
//         delay(wait);
//     }

//     for (uint8_t j = 255; j >= 0; j--) {
//         Log.info("ramping down");
//         // Set all pixels to the new brightness (simultaneously)
//         strip.setBrightness(j);
//         for (uint16_t i = 0; i < strip.numPixels(); i++) {
//             strip.setPixelColor(i, c);
//         }
//         strip.show();
//         delay(wait);
//     }
//     Log.info("Done breathing");
// }

void setColorFromTemperature(uint8_t temperature) {
    if (temperature < temp_1) {
        colorWipe(COLOR_0, 50);
    }
    else if (temperature >= temp_1 && temperature < temp_2) {
        colorWipe(COLOR_1, 50);
    }
    else if (temperature >= temp_2 && temperature < temp_3) {
        colorWipe(COLOR_2, 50);
    }
    else if (temperature >= temp_3 && temperature < temp_4) {
        colorWipe(COLOR_3, 50);
    }
    else if (temperature >= temp_4) {
        colorWipe(COLOR_4, 50);
    }

}

String determinePhase() {
    if (t_differential < 10 && deltaT_inside <= 0) {
        return "OFF";
    }

    if (t_inside >= 150) {
        return "READY";
    }

    // Using 0.2 degrees F as a naive margin to prevent false positives
    if (deltaT_inside > 0.2 && deltaT_inside > (deltaT_outside + 0.2)) {
        return "HEATING";
    }

    // if (deltaT_inside <= 0 && deltaT_inside <= deltaT_outside) {}
    return "COOLING";
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

    strip.begin();
    strip.setBrightness(LED_BRIGHTNESS);
    strip.show(); // Initialize all pixels to 'off'
}

void loop() {
    t_inside = getInsideTemp();
    t_outside = getOutsideTemp();
    t_differential = t_inside - t_outside;
    deltaT_inside = t_inside - t_insidePrevious;
    deltaT_outside = t_outside - t_outsidePrevious;

    phase = determinePhase();

    if (phase == "OFF") {
        // Turn off the LEDs
        strip.setBrightness(0);
        strip.show();
    }
    else {
        strip.setBrightness(LED_BRIGHTNESS);
        setColorFromTemperature(t_inside);
    }

    if (abs(t_inside - t_atLastBroadcast) > 2.0) {
        Log.info("Temperature has changed. Publishing event.");
        success = publishData(t_inside, t_outside, phase);

        // TODO: should this only be updated if the "publish()" is successful?
        // That would create a sort of "retry" behavior, but could also result in a lot of queued events if
        // they're failing just because the wifi connection has temporarily dropped.
        t_atLastBroadcast = t_inside;

        if (success != true) {
            Log.error("Failed to publish temperature event");
        }
    }

    t_insidePrevious = t_inside;
    t_outsidePrevious = t_outside;
    delay(10 * 1000); // Wait for 10 seconds
}
