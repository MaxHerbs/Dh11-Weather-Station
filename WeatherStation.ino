#include <WiFi.h>
#include "DHT.h"
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"
#include <NTPClient.h>
#include <WiFi.h>
#include <TimeLib.h>

const char* ssid = "xxx";
const char* password = "xxx";

float currentTemp;
float lowTemp;
float highTemp;
float avgTemp;

float currentHumid;
float lowHumid;
float highHumid;
float avgHumid;

bool resetHour = false;

TaskHandle_t avg;
WiFiClient espClient;
AsyncWebServer server(80);
DHT dht(23, 11);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

void checkHigh() {
  if (currentTemp > highTemp) {
    highTemp = currentTemp;
  }

  if (currentHumid > highHumid) {
    highHumid = currentHumid;
  }

}
void checkLow() {
  if (currentTemp < lowTemp) {
    lowTemp = currentTemp;
  }

  if (currentHumid < lowHumid) {
    lowHumid = currentHumid;
  }
}


String HTML() {
  String fullAdd = "<!DOCTYPE html>\n<html>\n<head>\n<style>\nbody{\n\tfont-family: Arial, Helvetica, sans-serif;\n\tcolor: white;\n}\ndiv{\n\tmargin: auto;\t\n\twidth: 60%;\n\n\tbackground-color: orange;\n\tborder: 5px solid white;\n\tpadding: 10px;\n}\nsup{\n\tvertical-align: top;\n\tfont-weight: bold;\n\tfont-size: 20px;\n}\n.title{\n\tfont-weight:bold;\n\tfont-size: 30px;\n\tmargin: 30px;\n}\n.largeNumber{\n\tfont-size: 65px;\n\tfont-weight: bold;\n\tmargin: 30px;\n}\n.smallerNumbers{\n\tfont-size: 25px;\n\tmargin: 30px;\n}\n.smallerTitles{\n\tfont-size:30px;\n\tmargin: 30px;\n}\ntd{\n\ttext-align:center;\n}\n</style>\n</head>\n<body>\n\n<div><br>\n<span class=\"title\";>Temperature</span><img src=\"https://image.flaticon.com/icons/png/512/56/56295.png\" style=\"width:120px;height:120px;float:right;\">\n<br><br>\n<span class=\"largeNumber\";>";
  fullAdd += currentTemp;
  fullAdd += "</span><sup> C </sup><br><br><br>\n\n<table style=\"width:100%\";>\n  <tr>\n    <td class=\"smallerTitles\";>Low</td>\n    <td class=\"smallerTitles\";>Avg</td> \n    <td class=\"smallerTitles\";>High</td>\n  </tr>\n  <tr>\n    <td class=\"smallerNumbers\";>";
  fullAdd += lowTemp;
  fullAdd += " <sup>c</sup></td>\n    <td class=\"smallerNumbers\";>";
  fullAdd += avgTemp;
  fullAdd += " <sup>c</sup></td> \n    <td class=\"smallerNumbers\";>";
  fullAdd += highTemp;
  fullAdd += " <sup>c</sup></td>\n  </tr>\n</table>\n<br>\n</div>\n\n<div style=\"background-color:#0DC2EA\";><br>\n<span class=\"title\";>Humidity</span><img src=\"https://image.flaticon.com/icons/png/512/63/63123.png\" style=\"width:120px;height:120px;float:right;\">\n<br><br>\n<span class=\"largeNumber\";>";
  fullAdd += currentHumid;
  fullAdd += "</span><sup> % </sup><br><br><br>\n\n<table style=\"width:100%\";>\n  <tr>\n    <td class=\"smallerTitles\";>Low </td>\n    <td class=\"smallerTitles\";>Avg</td> \n    <td class=\"smallerTitles\";>High </td>\n  </tr>\n  <tr>\n    <td class=\"smallerNumbers\";>";
  fullAdd += lowHumid;
  fullAdd += " %</td>\n    <td class=\"smallerNumbers\";>";
  fullAdd += avgHumid;
  fullAdd += " %</td> \n    <td class=\"smallerNumbers\";>";
  fullAdd += highHumid;
  fullAdd += " %</td>\n  </tr>\n</table>\n<br>\n</div>\n\n</body>\n</html>";

  return fullAdd;
}



















void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  dht.begin();
  Serial.println(WiFi.localIP());
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/html", HTML());
  });

  server.begin();

  xTaskCreatePinnedToCore(
                    averageCalc,   /* Task function. */
                    "Avg",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &avg,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
    delay(500); 


  delay(1000);
  Serial.println(WiFi.localIP());
  currentTemp = dht.readTemperature();
  currentHumid = dht.readHumidity();

  lowTemp = currentTemp;
  highTemp = currentTemp;

  highHumid = currentHumid;
  lowHumid = currentHumid;
}

void loop() {
  timeClient.update();
  currentTemp = dht.readTemperature();
  currentHumid = dht.readHumidity();
  checkLow();
  checkHigh();

  int currentHour = timeClient.getHours();

  if (timeClient.getHours() == 23 && timeClient.getMinutes() == 55) {
    highTemp = currentTemp;
    lowTemp = currentTemp;

    highHumid = currentHumid;
    lowHumid = currentHumid;

    avgTemp = currentTemp;
    avgHumid = currentHumid;
  }
  delay(2000);
}

void averageCalc( void * pvParameters ){
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());
  delay(5000);
  
  avgTemp = currentTemp;
  avgHumid = currentHumid;
  delay(2000);
  
  for(;;){  
    avgTemp = (avgTemp + currentTemp)/2;
    avgHumid = (avgHumid + currentHumid)/2;
    delay(5000);
  }
}
