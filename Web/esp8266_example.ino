#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

// Replace with your actual WiFi settings
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Replace with the IP address of your computer running the Node.js server
// e.g., "192.168.1.100"
const char* serverAddress = "YOUR_LAPTOP_IP_ADDRESS";
const int serverPort = 3000;

unsigned long lastTime = 0;
// Timer set to 5 seconds (5000)
unsigned long timerDelay = 5000;

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Check WiFi connection status
  if(WiFi.status() == WL_CONNECTED){
    
    if ((millis() - lastTime) > timerDelay) {
      WiFiClient client;
      HTTPClient http;

      // TODO: Replace this simulated distance with your actual TDR sensor reading
      float currentDistance = 3.65 + (random(-10, 10) / 100.0); // Simulates 3.55 to 3.75

      // Construct the URL path 
      // Example: http://192.168.1.100:3000/api/update-distance?dist=3.65
      String serverPath = "http://" + String(serverAddress) + ":" + String(serverPort) + "/api/update-distance?dist=" + String(currentDistance);
      
      Serial.print("Sending to: ");
      Serial.println(serverPath);

      // Your Domain name with URL path or IP address with path
      http.begin(client, serverPath.c_str());
      
      // Send HTTP GET request
      int httpResponseCode = http.GET();
      
      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      
      // Free resources
      http.end();
      
      lastTime = millis();
    }
  }
  else {
    Serial.println("WiFi Disconnected");
  }
}
