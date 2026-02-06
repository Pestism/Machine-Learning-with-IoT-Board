#include <Wire.h>
#include "dsplp_io.h"



// Function for reading temperature data from the TMP102 sensor 
float temperature() {
    int16_t raw = 0;

    Wire.beginTransmission(TMP_ADDRESS);
    Wire.write(TMP_REG_TEMP);                 // temperature register = 0x00
    uint8_t err = Wire.endTransmission();     // stop condition
    if (err) {
        Serial.printf("[ERROR] endTransmission: %u\n", err);
        return NAN;
    }

    if (Wire.requestFrom(TMP_ADDRESS, (uint8_t)2) == 2) {
        uint8_t msb = Wire.read();
        uint8_t lsb = Wire.read();

        raw = ((int16_t)msb << 8) | lsb;      // 16-bit value, left-justified 12-bit temp
        raw >>= 4;                            // arithmetic shift: now signed 12-bit value

        // Each LSB = 0.0625 °C
        return (float)raw * 0.0625f;
    } else {
        Serial.printf("[ERROR] Read error: wrong number of bytes.\n");
        return NAN;
    }
}


// Function for controlling the LEDs via the shift register
// Each bit in 'leds' corresponds to one LED (0 = off, 1 = on).
void diodes(uint8_t leds) {
    // Shift out 8 bits, LSB first
    for (int led = 0; led < 8; led++) {
        // Prepare for a clock pulse on the shift clock
        digitalWrite(LED_SHCP_IO, LOW);

        // Write the current bit to the data line
        // (1 << led) creates a mask to isolate one bit at a time
        if (leds & (1 << led)) {
            digitalWrite(LED_SDA_IO, HIGH);
        } else {
            digitalWrite(LED_SDA_IO, LOW);
        }

        delayMicroseconds(1);    // Small setup time before clock

        // Rising edge on the shift clock: the bit is shifted into the register
        digitalWrite(LED_SHCP_IO, HIGH);
        delayMicroseconds(1);    // Small hold time after clock
    }

    // Optional: set lines low after shifting
    digitalWrite(LED_SHCP_IO, LOW);
    digitalWrite(LED_SDA_IO, LOW);

    // Latch the shifted data into the output register:
    // Rising edge on STCP transfers the internal register to the outputs
    digitalWrite(LED_STCP_IO, HIGH);
    delayMicroseconds(1);
    digitalWrite(LED_STCP_IO, LOW);
}


// SETUP 
void setup() {
    
    // Setup serial 
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    
    // Setup communication with temperature sensor 
    Wire.begin(TMP_SDA_IO, TMP_SCL_IO, 100000);  
    
    // Configure the LED shift register pins as outputs
    pinMode(LED_SDA_IO, OUTPUT);
    pinMode(LED_SHCP_IO, OUTPUT);
    pinMode(LED_STCP_IO, OUTPUT);

    // Make sure all LEDs are turned off at start
    diodes(0);
}

float old_mu = 25.5632;
float sigma2 = 0.1735;
float new_mu = old_mu;
float lambda = 1.0/30.0;
float T;
float crit_val = 8.8075;
// MAIN LOOP
void loop() {

    float x = temperature();
    Serial.println(x);

    T = ((x - old_mu)*(x - old_mu)) / (sigma2);

    if (T < crit_val){
        new_mu = old_mu + lambda * ( x - old_mu );
        old_mu = new_mu;
    }
    else {
        float sum = 0.0;
        diodes(0b00000001);
        for(int i = 0; i<10; i++ ){
            float y = temperature();
            sum = sum + y;
            delay(1000);
        }
        old_mu = sum/10.0;
        diodes(0b00000000);
    }

    Serial.print("Test Statistic: ");
    Serial.println(T);
    delay(1000);
}
