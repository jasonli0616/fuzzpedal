#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>

// MCP4131
const byte ADDRESS = 0x00;
const int FUZZ_CS = 5;
const int VOLUME_CS = 15;
int fuzzValue = 0; // 0 to 128
int volumeValue = 0; // 0 to 128

// Web interface
const String ssid = "Fuzz Pedal";
const String password = "12345678";
AsyncWebServer server(80); // web server on port 80

const char index_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE html>
  <html lang="en">
  <body>

    <p>Volume</p>
    <input type="range" name="volume" id="volume" min="0" max="128">
    <p>Fuzz</p>
    <input type="range" name="fuzz" id="fuzz" min="0" max="128">

    <script>
      const urlParams = new URLSearchParams(window.location.search);

      let volumeSlider = document.getElementById("volume");
      let fuzzSlider = document.getElementById("fuzz");

      if (urlParams.has("volume") && urlParams.has("fuzz")) {
        volumeSlider.value = urlParams.get("volume");
        fuzzSlider.value = urlParams.get("fuzz");
      } else {
        volumeSlider.value = 0;
        fuzzSlider.value = 0;
      }

      let volume = volumeSlider.value;
      let fuzz = fuzzSlider.value;

      volumeSlider.addEventListener("change", (event) => {
        volume = event.target.value;
        updateValue();
      });

      fuzzSlider.addEventListener("change", (event) => {
        fuzz = event.target.value;
        updateValue();
      });


      function updateValue() {
        let xhr = new XMLHttpRequest();
        xhr.open("GET", `/?volume=${volume}&fuzz=${fuzz}`, true);
        xhr.send();
      }
    </script>
      
  </body>
  </html>
)rawliteral";



void digitalPotentiometerWrite(int value, int cs) {
  digitalWrite(cs, LOW);
  SPI.transfer(ADDRESS);
  SPI.transfer(value);
  digitalWrite(cs, HIGH);
}


void setup() {
  Serial.begin(9600);

  // Initialize SPI
  pinMode(FUZZ_CS, OUTPUT);
  pinMode(VOLUME_CS, OUTPUT);
  SPI.begin();

  // Connect to wifi
  WiFi.softAP(ssid, password);
  Serial.println(WiFi.softAPIP());
  MDNS.begin("guitarpedal"); // access webpage using guitarpedal.local
  MDNS.addService("http", "tcp", 80);
  server.begin();


  server.on("/", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    

    if (request->hasParam("volume")) {
      inputMessage = request->getParam("volume")->value();
      volumeValue = inputMessage.toInt();
      digitalPotentiometerWrite(volumeValue, VOLUME_CS);
      Serial.println("Volume: ");
      Serial.println(volumeValue);
    }
    if (request->hasParam("fuzz")) {
      inputMessage = request->getParam("fuzz")->value();
      fuzzValue = inputMessage.toInt();
      digitalPotentiometerWrite(fuzzValue, FUZZ_CS);
      Serial.println("Fuzz");
      Serial.println(fuzzValue);
    }
    request->send_P(200, "text/html", index_html);
  });

}

void loop() {


}
