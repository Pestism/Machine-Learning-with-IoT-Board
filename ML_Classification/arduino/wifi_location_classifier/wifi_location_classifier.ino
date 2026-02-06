#include <Arduino.h>
#include <WiFi.h>
#include "dsplp_io.h"  
#include <stdio.h>   
#include <math.h>   


// -------------------------------------------------------------
// Number of features (MAC addresses) used by the classifier
// -------------------------------------------------------------
#define NUM_FEATURES 3   


// -------------------------------------------------------------
// MAC addresses used as features (as 64-bit packed values)  // <-- Change this
// -------------------------------------------------------------
const uint64_t FEAT_MACS[NUM_FEATURES] = {
    0xEC750C67E56EULL,
    0xFE750C67E56EULL,
    0x3C6AD27F1E0FULL
};

// -------------------------------------------------------------
// Logistic regression parameters (learned in MATLAB)  // <-- Change this
// -------------------------------------------------------------
// One parameter per feature. Length must match NUM_FEATURES.
const float theta[NUM_FEATURES] = {
    0.5666f,
    0.5122f,
    -0.8579f
};


// -------------------------------------------------------------
// LED shift-register helper
// -------------------------------------------------------------
void diodes(uint8_t leds) {
    for (int led = 0; led < 8; led++) {
        digitalWrite(LED_SHCP_IO, LOW);
        (leds & (1 << led)) ? digitalWrite(LED_SDA_IO, HIGH)
                           : digitalWrite(LED_SDA_IO, LOW);
        delayMicroseconds(1);
        digitalWrite(LED_SHCP_IO, HIGH);
        delayMicroseconds(1);
    }

    digitalWrite(LED_SHCP_IO, LOW);
    digitalWrite(LED_SDA_IO, LOW);
    digitalWrite(LED_STCP_IO, HIGH);
    delayMicroseconds(1);
    digitalWrite(LED_SHCP_IO, HIGH);
    digitalWrite(LED_STCP_IO, LOW);
}


// -------------------------------------------------------------
// Convert a MAC string "AA:BB:CC:DD:EE:FF" to a 64-bit integer
// -------------------------------------------------------------
uint64_t parse_mac_hex(const String& macStr) {
    unsigned int b0, b1, b2, b3, b4, b5;

    // %x reads a hex integer, ':' are literal separators
    sscanf(macStr.c_str(), "%x:%x:%x:%x:%x:%x",
           &b0, &b1, &b2, &b3, &b4, &b5);

    // Pack the 6 bytes into a 64-bit integer
    uint64_t mac = 0;
    mac |= ((uint64_t)(b0 & 0xFF) << 40);
    mac |= ((uint64_t)(b1 & 0xFF) << 32);
    mac |= ((uint64_t)(b2 & 0xFF) << 24);
    mac |= ((uint64_t)(b3 & 0xFF) << 16);
    mac |= ((uint64_t)(b4 & 0xFF) <<  8);
    mac |= ((uint64_t)(b5 & 0xFF));

    return mac;
}


// -------------------------------------------------------------
// Find which feature index a MAC address belongs to
// -------------------------------------------------------------
static int feat_index_from_mac(uint64_t mac) {
    for (int i = 0; i < NUM_FEATURES; ++i) {
        if (mac == FEAT_MACS[i]) {
            return i;   // Match found
        }
    }
    return -1;          // Not a feature MAC
}


// -------------------------------------------------------------
// Build feature vector from WiFi scan
// -------------------------------------------------------------
static bool build_feature_vector(float* x) {

    bool found = true;   // Will stay true only if ALL features are seen

    // Mark all features as "not found yet" using NaN
    for (int k = 0; k < NUM_FEATURES; ++k) {
        x[k] = NAN;
    }

    int n = WiFi.scanNetworks();

    for (int i = 0; i < n; ++i) {
        uint64_t mac = parse_mac_hex(WiFi.BSSIDstr(i));
        int idx = feat_index_from_mac(mac);

        // If this MAC is one of our feature MACs
        if (idx >= 0 && idx < NUM_FEATURES) {
            x[idx] = (float)WiFi.RSSI(i);
        }
    }

    WiFi.scanDelete();

    // Final decision: if ANY x[k] is still NaN → missing MAC → error
    for (int k = 0; k < NUM_FEATURES; ++k) {
        if (isnan(x[k])) {
            found = false;
            break;
        }
    }

    return found;   // true = OK, false = ERROR
}

//helper function to do the dot product

float dot(const float *row, const float *col, size_t n) {
    float product = 0.0f;
    for (size_t i = 0; i < n; ++i) {
        product += row[i] * col[i];
    }
    return product;
}


// -------------------------------------------------------------
// Logistic regression classifier
// -------------------------------------------------------------
// Returns:
//   0 -> error (not all MACs found)
//   1 -> Location 1
//   2 -> Location 2
//
static uint8_t my_classifier(void) {

    float x[NUM_FEATURES];   // Feature vector (RSSI values)

    // Build RSSI feature vector
    if (!build_feature_vector(x)) {
        return 0;  // Error: not all feature MACs detected
    }
    float z = dot(x,theta, NUM_FEATURES);
    float sign = 1.0f / (1.0f + expf(-z));
    if (sign >= 0.5f){
        return 1;
    }
    else{
        return 2;
    }
}


// -------------------------------------------------------------
// Arduino setup
// -------------------------------------------------------------
void setup() {
    Serial.begin(115200);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    pinMode(LED_SDA_IO,  OUTPUT);
    pinMode(LED_SHCP_IO, OUTPUT);
    pinMode(LED_STCP_IO, OUTPUT);

    diodes(0b00000000);  // Turn off all LEDs
}


// -------------------------------------------------------------
// Arduino main loop
// -------------------------------------------------------------
void loop() {

    uint8_t led_nr = my_classifier();

    // Show classification result on LEDs
    diodes(1 << led_nr);

    Serial.print("Classifier output: ");
    Serial.println(led_nr);

    delay(1000);   // Run classifier once per second
}
