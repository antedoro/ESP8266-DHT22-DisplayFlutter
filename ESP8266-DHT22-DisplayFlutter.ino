#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSansBold9pt7b.h> 
#include <Fonts/FreeMono9pt7b.h> 

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

#define DHTPIN 14     
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);

float t = 0.0;
float h = 0.0;
AsyncWebServer server(80);

const long interval = 10000;
unsigned long previousMillis = 0;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html { font-family: Arial; text-align: center; margin: 0px auto; }
    h2 { font-size: 2.5rem; }
    p { font-size: 2.5rem; }
    .units { font-size: 1.2rem; }
    .dht-labels { font-size: 1.5rem; vertical-align: middle; padding-bottom: 15px; }
  </style>
</head>
<body>
  <h2>ESP8266 DHT Server</h2>
  <p><i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span><sup class="units">&deg;C</sup>
  </p>
  <p><i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span><sup class="units">%</sup>
  </p>
</body>
<script>
setInterval(function () {
  fetch('/temperature').then(response => response.text()).then(data => {
    document.getElementById("temperature").innerHTML = data;
  });
  fetch('/humidity').then(response => response.text()).then(data => {
    document.getElementById("humidity").innerHTML = data;
  });
}, 10000);
</script>
</html>)rawliteral";

// Replaces placeholder with DHT values
String processor(const String& var){
  if(var == "TEMPERATURE") return String(t);
  else if(var == "HUMIDITY") return String(h);
  return String();
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  // Web server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(t).c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(h).c_str());
  });
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    String jsonResponse = "{\"temperature\":" + String(t) + ", \"humidity\":" + String(h) + "}";
    request->send(200, "application/json", jsonResponse);
  });

  server.begin();
}

void updateDisplay(float temperature, float humidity) {
  display.clearDisplay();
  display.setFont(&FreeSansBold9pt7b);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0, 25);
  display.print("Temp: ");
  display.print(temperature);
  display.print(" C");

  display.setCursor(0, 50);
  display.print("Hum:   ");
  display.print(humidity);
  display.print(" %");
  
  display.display();
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    float newT = dht.readTemperature();
    float newH = dht.readHumidity();

    if (!isnan(newT)) {
      t = newT;
      Serial.print("Temperature: "); Serial.println(t);
    }
    if (!isnan(newH)) {
      h = newH;
      Serial.print("Humidity: "); Serial.println(h);
    }

    updateDisplay(t, h);
  }
}
