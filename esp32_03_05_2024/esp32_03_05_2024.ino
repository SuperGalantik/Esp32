/**
  Author: Gianluca Galanti 4C-IN
*/

#include <ArduinoJson.h>

#include <Wire.h>
#include <BH1750.h>

#include <DHT.h>
#include <DHT_U.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>

#define DHTPIN 13

#define SDAPIN 21
#define SCLPIN 22

#define LASERPIN 15

char *ssid = "dlink";
char *password = ""; 

IPAddress ip;

DHT dhtSensor(DHTPIN, DHT22);

BH1750 bhSensor;

WiFiClient client;
WiFiServer server(10000);

char* post_url = "http://192.168.1.3:10000/postListener";

unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;

float temp[3];
float hum[3];
float heat[3];
float light[3];

JsonDocument doc;

String dataToSend;

void setup() 
{
  Serial.begin(9600);

  delay(500);
  wifiConfig();

  delay(500);
  dhtSensor.begin();

  delay(500);
  Wire.begin();

  delay(500);
  bhSensor.begin();

  // delay(500);
  // pinMode(LASERPIN, OUTPUT);

  delay(500);
  clientConfiguration();

  doc["device_id"] = "esp32";

  xTaskCreatePinnedToCore (
  server_loop,     // Function to implement the task
  "server_loop",   // Name of the task
  1000,      // Stack size in bytes
  NULL,      // Task input parameter
  0,         // Priority of the task
  NULL,      // Task handle.
  0          // Core where the task should run
  );

  readHumAndTemp();
  readLightLevel();
}

/* ---------------------------- main loop ---------------------------------*/

void loop() 
{
  readHumAndTemp();
  readLightLevel();
  delay(30*60*1000);
}

/* ---------------------------- Server loop ---------------------------------*/

void server_loop(void* pvParameters)
{
  WiFiClient getClient = server.available();   

  if(getClient)
  {
    currentTime = millis();
    previousTime = currentTime;
    String currentLine = "";

    while (client.connected() && currentTime - previousTime <= timeoutTime) 
    {  
      currentTime = millis();
      if (getClient.available()) 
      {             
        char c = getClient.read();
        Serial.write(c);
        if (c == '\n') 
        {                    
          if (currentLine.length() == 0) 
          {
            getClient.println("HTTP/1.1 200 OK");
            getClient.println("Content-type:application/json");
            getClient.println("Connection: close");
            //parse the json data before sending
            parse_json_data();
            //send the data
            getClient.print(dataToSend);
            //print another blank line to end connection
            getClient.println();
            getClient.println();
            break;
          } 
          else 
            currentLine = "";
        }
        else if (c != '\r') 
          currentLine += c;
      }
    }
    getClient.stop();
  }
}

/* ---------------------------- JsonParser ---------------------------------*/

void parse_json_data()
{
  int i;

  for(i=0; i<3; i++)
  {
    doc["temperature"][i] = temp[i];
    doc["humidity"][i] = hum[i];
    doc["heat_index"][i] = heat[i];
    doc["light"][i] = light[i];
  }

  serializeJson(doc, dataToSend);
}

/* ---------------------------- Wifi client configuration ---------------------------------*/

void wifiConfig()
{
  WiFi.begin(ssid);
  
  while(WiFi.status()!= WL_CONNECTED)
  {
    Serial.println("Not connected");
    delay(5000);
  }

  Serial.println(WiFi.localIP());
  Serial.println();
}

/* ---------------------------- send data as client ---------------------------------*/

void clientConfiguration()
{
  HTTPClient http;
  int httpResponseCode;
  
  Serial.println(post_url);
  http.begin(client, post_url);

  http.addHeader("Content-type", "application/json");

  do
  {
    httpResponseCode = http.POST(dataToSend);
    http.end();
    delay(250);
  } while(httpResponseCode!=200);
  lastTime = millis();
}

/* ---------------------------- read humidity, temperature and heat index ---------------------------------*/

void readHumAndTemp()
{
  int i;
  for(i=0; i<3; i++)
  {
    temp[i] = dhtSensor.readTemperature();
    delay(2000);
    hum[i] = dhtSensor.readHumidity();
    delay(2000);
    heat[i] = dhtSensor.computeHeatIndex();
  }
}

/* ---------------------------- read light level ---------------------------------*/

void readLightLevel()
{
  int i;
  for(i=0; i<3; i++)
    light[i] bhSensor.readLightLevel();
}
  
