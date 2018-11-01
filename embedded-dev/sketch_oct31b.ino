/*
 * B U Q Q B O I S embedded
 * 
 * WiFi server with basic GPIO related to our project
 */

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

const char* ssid     = "esptest";
const char* password = "buckeyes1";

const int relay_io_map[] = {2, 4, 5, 18}; //the I/Os being used for relay control
const int acs_io_map[] = {32, 33, 34, 35}; //the I/Os being used for ACS723 input

WebServer server(80);

/*
 * setup()
 */
 
void setup() {
  Serial.begin(115200); // begin serial link at 115K baud
  
  // set up pins appropriately
  for (int i = 0; i < 4; i++) {
    pinMode(relay_io_map[i], OUTPUT);
    pinMode(acs_io_map[i], INPUT);
  }

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}


/*
 * loop()
 */
void loop() {
  server.handleClient();
}

void read_acs(int *res) {
  for (int i = 0; i < 4; i++) {
    *res = analogRead(acs_io_map[i]);
    res++;
  }
}

void handleRoot() {
  char temp[1600];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  int acs_res[] = {0,0,0,0};

  read_acs(acs_res);

  snprintf(temp, 1600,

           "<html><head>\
    <title>ESP32 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>B U Q Q B O I S</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <button onclick=\"window.location.href='/'\">refresh</button>\
    <table><tr><td>Analog 0</td><td>%d</td></tr>\
    <tr><td>Analog 1</td><td>%d</td></tr>\
    <tr><td>Analog 2</td><td>%d</td></tr>\
    <tr><td>Analog 3</td><td>%d</td></tr>\
    <tr><td>Relay 0</td><td><button onclick=\"window.location.href='/r0'\">Toggle</button></td></tr>\
    <tr><td>Relay 1</td><td><button onclick=\"window.location.href='/r1'\">Toggle</button></td></tr>\
    <tr><td>Relay 2</td><td><button onclick=\"window.location.href='/r2'\">Toggle</button></td></tr>\
    <tr><td>Relay 3</td><td><button onclick=\"window.location.href='/r3'\">Toggle</button></td></tr></table>\
  </body>\
</html>",

           hr, min % 60, sec % 60, acs_res[0], acs_res[1], acs_res[2], acs_res[3]
          );
  server.send(200, "text/html", temp);
}

void handleNotFound() {
  String path = server.uri();
  int found = 0;

  for (int i = 0 ; i < 4 ; i++){
    if (path.endsWith("/r"+String(i))) {
      found = 1;
      handleRoot();
      digitalWrite(relay_io_map[i], !digitalRead(relay_io_map[i]));
      return;
    }
  }
  
  if (!found) {
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
  
    for (uint8_t i = 0; i < server.args(); i++) {
      message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    
    server.send(404, "text/plain", message);
  }
  
}
