#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

// --- Wi-Fi & Web Settings ---
const char* ssid = "hehe";
const char* password = "Mystic123";

// CHANGE THIS to your laptop's IP address and the port!
// Example: "http://192.168.1.5:3000/api/update-distance"
const char* serverName = "http://172.18.37.146:3000/api/update-distance"; 

uint8_t buffer[2];
int byteIndex = 0;

// Timer to prevent spamming your web server
unsigned long lastTimeSent = 0;
// Sent to web every 2 seconds to match frontend polling
const unsigned long timerDelay = 2000; 

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("\nConnecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nConnected to WiFi!");
  Serial.println("--- TDR Receiver Ready ---");
}

void loop() {
  while (Serial.available() > 0) {
    uint8_t incomingByte = Serial.read();

    if (incomingByte == '\n') {
      if (byteIndex == 2) {
        
        uint16_t distance_cm = (buffer[0] << 8) | buffer[1];
        
        // Convert from centimeters to meters because the website uses meters!
        float distance_meters = distance_cm / 100.0;
        
        Serial.print("Fault Distance: ");
        Serial.print(distance_meters);
        Serial.println(" m");

        // Send to Website
        if ((millis() - lastTimeSent) > timerDelay) {
          if (WiFi.status() == WL_CONNECTED) {
            WiFiClient client;
            HTTPClient http;
            
            // Construct the exact URL the server expects:
            String serverPath = String(serverName) + "?dist=" + String(distance_meters);
            
            http.begin(client, serverPath.c_str());
            int httpResponseCode = http.GET();
            
            if (httpResponseCode > 0) {
              Serial.print("HTTP Response code: ");
              Serial.println(httpResponseCode);
            } else {
              Serial.print("HTTP Error code: ");
              Serial.println(httpResponseCode);
            }
            http.end(); 
            
            lastTimeSent = millis();
          }
        }
      }
      byteIndex = 0;
    } else {
      if (byteIndex < 2) {
        buffer[byteIndex] = incomingByte;
        byteIndex++;
      }
    }
  }
}
