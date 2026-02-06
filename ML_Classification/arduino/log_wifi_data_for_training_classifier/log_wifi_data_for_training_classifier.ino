#include "WiFi.h"

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
}

void loop() {

  if (Serial.available() > 0) {

    char c = Serial.read();

    if (c == 'A') {

      for (int k = 0; k < 5; k++) {

        int n = WiFi.scanNetworks();

        for (int i = 0; i < n; i++) {
          Serial.print(WiFi.BSSIDstr(i));
          Serial.print(",");
          Serial.println(WiFi.RSSI(i));
          delay(10);
        }

        WiFi.scanDelete();
         delay(1000);
      }

      Serial.println("SS"); // Stop signal   
    }
  }
}
