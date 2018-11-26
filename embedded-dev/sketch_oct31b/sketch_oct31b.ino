/*
 * B U Q Q B O I S embedded
 * 
 * WiFi server with basic GPIO related to our project
 */

#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <driver/adc.h>

const char* ssid     = "BOX OF ROCKS-2G";  //"esptest";
const char* password = "doubleunit022";  //"buckeyes1";

const char* mqttServer = "m15.cloudmqtt.com";
const int mqttPort = 11322;
const char* mqttUser = "akyumnii";
const char* mqttPassword = "Z2HnUN3RumXD";

const float ACS_CURRENT_RANGE = 5; //5 amp max current meas
int ZERO_PT[] = {1810, 1810, 1810, 1810};

const int relay_io_map[] = {16, 17, 18, 19, 4, 2, 23, 22}; //the I/Os being used for relay control
int relay_states[] = {0,0,0,0};
const int acs_io_map[] = {34, 32, 35, 33}; //the I/Os being used for ACS723 input

WebServer server(80);
WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;
/*
 * setup()
 */
 
void setup() {
  Serial.begin(115200); // begin serial link at 115K baud
  analogSetWidth(12);
  analogSetCycles(255);
  
  // set up pins appropriately
  for (int i = 0; i < 8; i++) {
    pinMode(relay_io_map[i], OUTPUT);
    digitalWrite(relay_io_map[i],0);
    
    //pinMode(acs_io_map[i], INPUT);
    //adcAttachPin(acs_io_map[i]);
  }

  for (int i = 0; i < 4; i++) {
    pinMode(acs_io_map[i], INPUT);
    //adcAttachPin(acs_io_map[i]);
  }

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }

  client.setServer(mqttServer, mqttPort);
 
  while (!client.connected()) 
  {
    Serial.println("Connecting to MQTT..."); 
    if (client.connect("ESP32Client", mqttUser, mqttPassword )) 
    { 
      Serial.println("connected");   
    } 
    else 
    {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000); 
    }
}
  client.setCallback(callback);

  for (int j = 0 ; j < 4 ; j++) {
    setRelay(j,0);
    setRelayState(j,0);
  }

  // now get ZERO_PT
  int zsum = 0;
  for (int j = 0 ; j < 4 ; j++) {
    for (int k = 0 ; k < 100 ; k++) {
      zsum += analogRead(acs_io_map[j]);
    }
    ZERO_PT[j] = zsum / 100;
    zsum = 0;
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
  server.on("/test.svg", drawGraph);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
      //digitalWrite(ledPin, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      //digitalWrite(ledPin, LOW);
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setRelay(int relayIndex, int state) {
  int i = relayIndex;
  Serial.println("Toggling GPIOs " +  String(relay_io_map[2*i]) + ", " + String(relay_io_map[2*i+1]));

  digitalWrite(relay_io_map[2*relayIndex], !(state && relay_states[relayIndex]));
  digitalWrite(relay_io_map[2*relayIndex+1], (state && relay_states[relayIndex]));
  delay(250);
  digitalWrite(relay_io_map[2*relayIndex], 0);
  digitalWrite(relay_io_map[2*relayIndex+1], 0);
}

void setRelayState(int relayIndex, int state) {
  relay_states[relayIndex] =  state;
}

/*
 * loop()
 */
void loop() 
{
  if (!client.connected()) 
  {
    reconnect();
  }
  client.loop();
  long now = millis();
    
  if (now - lastMsg > 5000) 
  {
    lastMsg = now;
    int acs_res[] = {0,0,0,0};
    read_acs(acs_res);

    int reads[] = {0, 0, 0, 0};
    for (int i = 0; i < 4; i++) 
    {
      reads[i] = getCurrent(acs_io_map[i], i);
      Serial.print("Outlet: ");
      Serial.print(i);
      Serial.println(reads[i]);
      char topic[32];
      char payload[32];
      snprintf(topic, 32, "esp32/outlet/%d/data", i);
      snprintf(payload, 32, "%d", rawToPower(reads[i], i));
      client.publish(topic, payload);
    }
  }
  server.handleClient();
}

void read_acs(int *res) {
  for (int i = 0; i < 4; i++) {
    *res = analogRead(acs_io_map[i]);
    res++;
  }
}

int adcAverage(int pin) {
  int reading = 0;
  for (int i = 0 ; i < 50 ; i++) {
    reading += analogRead(pin);
  }
  return reading / 50;
}

float getCurrent (int pin, int pinIdx) {
  int maxDiff = 0;
  int reading = 0;
  int reads[200];
  for (int i = 0 ; i < 200 ; i++) {
    reading = analogRead(pin);
    reads[i] = reading;
    int abRead = abs(reading - ZERO_PT[pinIdx]);
    if (abRead > maxDiff) {
      maxDiff = abRead;
    }
    delayMicroseconds(500);
  }

  return getRMS(reads, 200, pinIdx);
}

int rawToPower(int reading, int pin) {
  return pin != 2 ? int(reading * 12.5 * 120.0 / 4096.0) : int(reading * 1.40 * 12.5*3 * 120.0 / 4096.0);
}

void handleRoot() {
  char temp[3200];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  int acs_res[] = {0,0,0,0};

  read_acs(acs_res);

  int reads[] = {0, 0, 0, 0};
  for (int i = 0; i < 4; i++) {
    reads[i] = getCurrent(acs_io_map[i], i);
  }
  snprintf(temp, 3200,


           "<html><head>\
    <title>Smart Outlet</title>\
    <style>\
      </style>\
  </head>\
  <script>window.history.pushState('', '', '/');</script>\
  <body>\
<nav class='navbar navbar-default'>\
  <div class='container-fluid'>\
    <div class'navbar-header'>\
      <a class='navbar-brand' href='#'>Team 5 Smart Outlet</a>\
    </div>\
    <ul class='nav navbar-nav pull-right'>\
    <li><a href='/'>Refresh</a></li>\
    </ul>\
  </div>\
</nav>\
<div class='container'>\
  <div class='row'>\
    <div class='outbox col-xs-6'>\
    <h2>Line 0</h2>\
    <h3>(%d) %d W</h3>\
    <button class='btn btn-dark' onclick=\"window.location.href='/a0?pin=0'\">More</button>\
    <button class='btn btn-dark' onclick=\"window.location.href='/r0'\">Toggle</button>\
    </div>\
    <div class='outbox col-xs-6'>\
    <h2>Line 1</h2>\
    <h3>(%d) %d W</h3>\
    <button class='btn btn-dark' onclick=\"window.location.href='/a1?pin=1'\">More</button>\
    <button class='btn btn-dark' onclick=\"window.location.href='/r1'\">Toggle</button>\
    </div>\
  </div>\
  <div class='row'>\
    <div class='outbox col-xs-6'>\
    <h2>Line 2</h2>\
    <h3>(%d) %d W</h3>\
    <button class='btn btn-dark' onclick=\"window.location.href='/a2?pin=2'\">More</button>\
    <button class='btn btn-dark' onclick=\"window.location.href='/r2'\">Toggle</button>\
    </div>\
    <div class='outbox col-xs-6'>\
    <h2>Line 3</h2>\
    <h3>(%d) %d W</h3>\
    <button class='btn btn-dark' onclick=\"window.location.href='/a3?pin=3'\">More</button>\
    <button class='btn btn-dark' onclick=\"window.location.href='/r3'\">Toggle</button>\
    </div>\
  </div>\
</div>\
    <p>Uptime: %02d:%02d:%02d</p>\
  </body>\
  <script src='https://code.jquery.com/jquery-3.3.1.min.js'></script>\
<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css' integrity='sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u' crossorigin='anonymous'>\
<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap-theme.min.css' integrity='sha384-rHyoN1iRsVXV4nD0JutlnGaslCJuC7uwjduW9SVrLvRYooPp2bWYgmgJQIXwl/Sp' crossorigin='anonymous'>\
<script src='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js' integrity='sha384-Tc5IQib027qvyjSMfHjOMaLkfuWVxZxUPnCJA7l2mCWNIpG9mGCD8wGNIcPD7Txa' crossorigin='anonymous'></script>\
</html>",

            reads[0], rawToPower(reads[0], 0),reads[1], rawToPower(reads[1], 1),reads[2], rawToPower(reads[2], 2),reads[3], rawToPower(reads[3], 3), hr, min % 60, sec % 60
          );
  server.send(200, "text/html", temp);
}


void handleAnalogReport(int pinIdx, int pin) {
  char temp[4600];

  snprintf(temp, 1600,

           "<html><head>\
           <meta http-equiv='refresh' content='5'/>\
    <title>Smart Outlet</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #880000; }\
    </style>\
  </head>\
  <body>\
    <h1>Analog Report for pin %d</h1>\
    <img src=\"/test.svg?pin=%d\" />\
  </body>\
<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css' integrity='sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u' crossorigin='anonymous'>\
<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap-theme.min.css' integrity='sha384-rHyoN1iRsVXV4nD0JutlnGaslCJuC7uwjduW9SVrLvRYooPp2bWYgmgJQIXwl/Sp' crossorigin='anonymous'>\
<script src='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js' integrity='sha384-Tc5IQib027qvyjSMfHjOMaLkfuWVxZxUPnCJA7l2mCWNIpG9mGCD8wGNIcPD7Txa' crossorigin='anonymous'></script>\
</html>",

           pin, pinIdx);
  server.send(200, "text/html", temp);
}


void handleNotFound() {
  String path = server.uri();
  int found = 0;

  for (int i = 0 ; i < 4 ; i++){
    if (path.endsWith("/r"+String(i))) {
      found = 1;
      handleRoot();
      setRelayState(i, !relay_states[i]);
      setRelay(i, 1);
      return;
    }
  }

  for (int i = 0 ; i < 4 ; i++){
    if (path.endsWith("/a"+String(i))) {
      found = 1;
      handleAnalogReport(i, acs_io_map[i]);
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


void drawGraph() {
  String out = "";
  int reads[200];
  int maxDiff = 0;
  int pin = acs_io_map[server.arg("pin").toInt()];
  
  char temp[150];
  out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"800\" height=\"500\">\n";
  out += "<rect width=\"800\" height=\"500\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
  out += "<g stroke=\"black\">\n";
  for (int x = 0; x < 200; x++) {
    int rd = (analogRead(pin));
    reads[x] = rd;
    int diff = abs(rd - ZERO_PT[pin]);
    if (diff > maxDiff) {
      maxDiff = diff;
    }
    delayMicroseconds(500);
  }

  for (int x = 0; x < 199; x++) {
     sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x*4, int(reads[x]*500/4096.0), x*4 + 4, int(reads[x+1]*500/4096.0));
     out += temp;
  }

  Serial.println("chart1");
  
  // Calc RMS
  int rms = getRMS(reads, 200, pin);
  sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-dasharray=\"4\" stroke=\"rgb(0, 255, 0)\" stroke-width=\"1\" />\n", 0, int(((ZERO_PT[pin]-rms)/4096.0) * 500), 800, int(((ZERO_PT[pin]-rms)/4096.0) * 500));
  out += temp;
  sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-dasharray=\"4\" stroke=\"rgb(0, 255, 0)\" stroke-width=\"1\" />\n", 0, int(((ZERO_PT[pin]+rms)/4096.0) * 500), 800, int(((ZERO_PT[pin]+rms)/4096.0) * 500));
  out += temp;

  Serial.println("chart2");

  // draw max line
  int maxDiff1 = int((ZERO_PT[pin]/4096.0) * 500) - int((maxDiff/4096.0) * 500);
  int maxDiff2 = int((ZERO_PT[pin]/4096.0) * 500) + int((maxDiff/4096.0) * 500);
  sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke=\"rgb(0, 0, 255)\" stroke-width=\"1\" />\n", 0, int((ZERO_PT[pin]/4096.0) * 500), 800, int((ZERO_PT[pin]/4096.0) * 500));
  out += temp;

  sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke=\"rgb(255, 0, 0)\" stroke-width=\"1\" />\n", 0, maxDiff1, 800, maxDiff1);
  out += temp;
  
  sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke=\"rgb(255, 0, 0)\" stroke-width=\"1\" />\n", 0, maxDiff2, 800, maxDiff2);
  out += temp;
  
  out += "</g>\n</svg>\n";

  Serial.println("/chart");
  
  server.send(200, "image/svg+xml", out);
}

// rms of abs(arr[x] - ZERO_PT)
int getRMS(int *arr, int len, int pin) {
  int tot = 0;
  for (int i = 0 ; i < len ; i++) {
    tot += sq(ZERO_PT[pin] - arr[i]);
  }
  return sqrt(tot/len);
}
