#include <Arduino.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <Preferences.h>
#include <Wire.h>

#include "main.h"

Params params;

#include "WifiScanner.h"
#include "Radio.h"
#include "Servos.h"

#ifdef WIFICLIENTMULTI_H_
WiFiMulti wifiMulti;
#endif

Preferences prefs;

const char *host = "perro";

#ifndef WIFICLIENTMULTI_H_
// Replace with your network credentials
// const char *ssid = "BITCOIN", *password = "Am3r1c4n01";
// IPAddress local_IP(192, 168, 7, 222);
// IPAddress gateway(192, 168, 7, 254);
// IPAddress subnet(255, 255, 255, 0);
// IPAddress primaryDNS(8, 8, 8, 8);    // optional
// IPAddress secondaryDNS(4, 2, 2, 2);  // optional

// const char* ssid = "mario-hotspot-open";
const char *ssid = "MiniMakers", *password = "Sayaab01";

#endif

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Decode HTTP GET value
String valueString = String(5);
int pos1 = 0;
int pos2 = 0;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup()
{
  while (!Serial)
    delay(100);
  delay(1000);
  Serial.begin(115200);
  Wire.begin();

  if (!prefs.begin("perro", false))
  {
    Serial.println("Error abriendo preferencias");
    delay(5000);
    ESP.restart();
  }
  if (prefs.getBytes("perro", &params, sizeof(params)) == 0)
  {
    prefs.putBytes("perro", &params, sizeof(params));
  }

  xTaskCreate(vTaskRadio, "taskRadio", 4096, &params, 0, NULL);
  xTaskCreate(vTaskServos, "taskServos", 4096, &params, 0, NULL);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to WiFi");
  // WiFi.mode(WIFI_AP_STA);
  WiFi.mode(WIFI_STA);
  delay(1000);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  WiFi.setSleep(false);
  esp_wifi_set_ps(WIFI_PS_NONE);

  WiFiScanner();

#ifdef WIFICLIENTMULTI_H_
  wifiMulti.addAP("MiniMakers", "Sayaab01");
  wifiMulti.addAP("BITCOIN", "Am3r1c4n01");
  wifiMulti.addAP("DURAZNO1_2.4", "Am3r1c4n01");
  wifiMulti.addAP("mario-hotspot-open");
  wifiMulti.addAP("mario-hotspot", "Am3r1c4n01");
#else
#ifdef password
  WiFi.begin(ssid, password);
#else
  WiFi.begin(ssid);
#endif

#ifdef local_IP &&gateway &&subnet &&primaryDNS &&secondaryDNS
  Serial.println("Usando direccion fija");
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
  {
    Serial.println("STA Failed to configure");
  }
#endif

  Serial.print("Connecting to: ");
  Serial.println(ssid);
#endif
  pinMode(GPIO_NUM_2, OUTPUT);
  digitalWrite(GPIO_NUM_2, LOW);
  int retries = 0;
#ifdef WIFICLIENTMULTI_H_
  while (wifiMulti.run(10000) != WL_CONNECTED)
  {
#else
  while (WiFi.status() != WL_CONNECTED)
  {
    // while (WiFi.waitForConnectResult() != WL_CONNECTED) {
#endif
    delay(500);
    Serial.print(".");
    digitalWrite(GPIO_NUM_2, !digitalRead(GPIO_NUM_2));
    if (++retries > 60)
    {
      digitalWrite(GPIO_NUM_2, LOW);
      delay(10);
      WiFi.disconnect();
      delay(5000);
      ESP.restart();
    }
  }
  digitalWrite(GPIO_NUM_2, HIGH);
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Network: ");
  Serial.println(WiFi.SSID());

  if (!MDNS.begin(host))
  { // http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1)
    {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

  server.begin();

  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  ArduinoOTA.setHostname(host);

  // No authentication by default
  ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
      .onStart([]()
               {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else  // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type); })
      .onEnd([]()
             { Serial.println("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total)
                  { Serial.printf("Progress: %u%%\r\n", (progress / (total / 100))); })
      .onError([](ota_error_t error)
               {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed"); });

  ArduinoOTA.begin();
}

void loop()
{
#ifdef WIFICLIENTMULTI_H_
  if (wifiMulti.run() != WL_CONNECTED)
  {
#else
  if (WiFi.status() != WL_CONNECTED)
  {
#endif
    Serial.println("WiFi not connected!");
    delay(1000);
  }

  ArduinoOTA.handle();

  WiFiClient client = server.available(); // Listen for incoming clients

  if (client)
  { // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client."); // print a message out in the serial port
    String currentLine = "";       // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime)
    { // loop while the client's connected
      currentTime = millis();
      if (client.available())
      {                         // if there's bytes to read from the client,
        char c = client.read(); // read a byte, then
        // Serial.write(c);        // print it out the serial monitor
        header += c;
        if (c == '\n')
        { // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0)
          {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>body { text-align: center; font-family: \"Trebuchet MS\", Arial; margin-left:auto; margin-right:auto;}");
            client.println(".slider { width: 300px; }</style>");
            client.println("<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script>");

            client.println("<script>");
            client.println("$.ajaxSetup({timeout:1000});");
            client.println("function servo(pos, servoNum) { ");
            client.println("  $('#servoPos'+servoNum).html(pos);");
            client.println("  $.get(\"/s\"+servoNum+\"?value=\" + pos + \"&\"); {Connection: close};");
            client.println("}");
            client.println("</script>");

            // Web Page
            client.println("</head><body><h1>ESP32 with Servo</h1>");
            for (int s = 1; s <= 3; s++)
            {
              char buffer[512];
              int pos;
              switch (s)
              {
              case 1:
                // pos = perro.pos1;
                break;
              case 2:
                // pos = perro.pos2;
                break;
              case 3:
                // pos = perro.pos3;
                break;
              }
              sprintf(buffer, "<p>Position Servo %d: <span id=\"servoPos%d\">%d</span></p>", s, s, pos);
              client.println(buffer);
              sprintf(buffer, "<input type=\"range\" min=\"0\" max=\"180\" class=\"slider%d\" id=\"servoSlider%d\" onchange=\"servo(this.value, %d)\" value=\"%d\"/>", s, s, s, pos);
              client.println(buffer);
            }

            client.println("</body></html>");

            // GET /?value=180& HTTP/1.1
            if (header.indexOf("GET /s1?value=") >= 0)
            {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1 + 1, pos2);

              // Rotate the servo
              // perro.pos1 = valueString.toInt();
              prefs.putBytes("perro", &params, sizeof(params));
              // pwmController.setChannelPWM(0, pwmServo1.pwmForAngle(map(perro.pos1, 0, 180, -90, 90)));
              // Serial.println(valueString);
            }

            if (header.indexOf("GET /s2?value=") >= 0)
            {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1 + 1, pos2);

              // Rotate the servo
              // perro.pos2 = valueString.toInt();
              prefs.putBytes("perro", &params, sizeof(params));
              // pwmController.setChannelPWM(1, pwmServo2.pwmForAngle(map(perro.pos2, 0, 180, -90, 90)));
              // Serial.println(valueString);
            }

            if (header.indexOf("GET /s3?value=") >= 0)
            {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1 + 1, pos2);

              // Rotate the servo
              // perro.pos3 = valueString.toInt();
              prefs.putBytes("perro", &params, sizeof(params));
              // pwmController.setChannelPWM(2, pwmServo3.pwmForAngle(map(perro.pos3, 0, 180, -90, 90)));
              // Serial.println(valueString);
            }

            // The HTTP response ends with another blank line
            // client.println();
            // Break out of the while loop
            break;
          }
          else
          { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        }
        else if (c != '\r')
        {                   // if you got anything else but a carriage return character,
          currentLine += c; // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
