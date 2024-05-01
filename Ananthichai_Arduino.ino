#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFiMulti.h>

#define PIR_PIN D7  // PIR sensor pin
#define BUILTIN_LED 2  // Built-in LED pin on the AX-WiFi board
#define API_KEY "7ZNQA98AQ1SUFEI1"
#define CHANNEL_ID "2531274"
#define LED D6
#define BUZZER_PIN D8

const char* ssid = "iot-lab";
const char* password = "computing";
const char* server = "api.thingspeak.com";
const int httpPort = 80;

ESP8266WiFiMulti WiFiMulti;
WiFiClient client;
ESP8266WebServer webServer(80); // Create a web server object listening on port 80

bool ledState = false; // Flag to track LED state

void setup() {
  WiFiMulti.addAP("iot-lab","computing");
  while (WiFiMulti.run() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  
  pinMode(PIR_PIN, INPUT);
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  Serial.begin(115200);
  
  // Connect to WiFi
  Serial.println();
  Serial.println("Connecting to WiFi...");
  WiFi.begin("iot-lab", "computing");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");

  // Print local IP address
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  String html = "<h1>Hello world</h1>"
                "<h3>This our IoT Sensor project</h3>"
                "<h3>==================================</h3>"
                "<h2>Group member</h2>"
                "<h4>1.) Ananthichai saehui 6530613030 DE.</h4>"
                "<h4>1.) Nadthawitra ketkeaw 6530613006 DE.</h4>"
                "<button id='onButton' onclick='onButtonClicked()' ";
  
  if(ledState) {
    html += "disabled"; // Disable ON button if LED is already ON
  }

  html += ">ON</button>"
          "<button id='offButton' onclick='offButtonClicked()'>OFF</button>"
          "<script>"
          "function onButtonClicked() {"
          "  fetch('/led/on');"
          "  fetch('/update/1');"
          "  document.getElementById('onButton').disabled = true;"
          "  document.getElementById('offButton').disabled = false;"
          "}"
          "function offButtonClicked() {"
          "  fetch('/led/off');"
          "  fetch('/update/0');"
          "  document.getElementById('onButton').disabled = false;"
          "  document.getElementById('offButton').disabled = true;"
          "}"
          "</script>";

  webServer.on("/", [html](){
    webServer.send(200, "text/html", html);
  });

  webServer.on("/led/on", [](){
    digitalWrite(LED, HIGH);
    digitalWrite(BUILTIN_LED, LOW);
    analogWrite(BUZZER_PIN, 8);
    ledState = true; // Set LED state to ON
    webServer.send(200, "text/html", "LED and buzzer turned on");
  });

  webServer.on("/led/off", [](){
    digitalWrite(LED, LOW);
    digitalWrite(BUILTIN_LED, HIGH);
    analogWrite(BUZZER_PIN, 0);
    ledState = false; // Set LED state to OFF
    webServer.send(200, "text/html", "LED and buzzer turned off");
  });

  // Start web server
  webServer.begin();
}

void loop() {
  webServer.handleClient(); // Handle incoming client requests
  
  int motion = digitalRead(PIR_PIN);

  if (motion == HIGH) {
    sendDataToThingSpeak(1);
    digitalWrite(LED, HIGH);
    digitalWrite(BUILTIN_LED, LOW); // Turn on built-in LED if motion is detected
    analogWrite(BUZZER_PIN, 8);
    Serial.println("Motion Detected!");
  } else {
    sendDataToThingSpeak(0);
    digitalWrite(LED, ledState ? HIGH : LOW); // Keep LED ON if ledState is true
    digitalWrite(BUILTIN_LED, ledState ? LOW : HIGH); // Keep built-in LED ON if ledState is true
    analogWrite(BUZZER_PIN, ledState ? 8 : 0); // Keep buzzer ON if ledState is true
    Serial.println("No Motion Detected");
  }
  
  delay(500); // Delay for stability
}

void sendDataToThingSpeak(int motion) {
  if (client.connect(server, httpPort)) {
    String url = "/update?api_key=";
    url += API_KEY;
    url += "&field1=";
    url += String(motion);
    
    Serial.print("Requesting URL: ");
    Serial.println(url);
    
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + server + "\r\n" +
                 "Connection: close\r\n\r\n");
    
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 10) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }
    
    while (client.available()) {
      String line = client.readStringUntil('\r');
      //Serial.print(line);
    }
    
    Serial.println();
    Serial.println("closing connection");
  } else {
    Serial.println("Connection failed!");
  }
}
