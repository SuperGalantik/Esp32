/**
  Author: Gianluca Galanti 4C-IN
  Version: 6/05/2024
*/

#include <ESPAsyncWebServer.h>

#include <AsyncTCP.h>

#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>

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

char *ssid = "EoloRouterEvo";
char *password = "Pr?43!8ve"; 

IPAddress ip;

DHT dhtSensor(DHTPIN, DHT22);

BH1750 bhSensor;

WiFiClient wifi;

WiFiClient getClient;

IPAddress local_ip(192,168,1,9);
IPAddress gateway(192,168,1,1);
IPAddress snm(255,255,255,0);

String server_address = "192.168.1.3";
int port = 10000;

HttpClient client = HttpClient(wifi, server_address, port);

WiFiServer server(10000);

TaskHandle_t server_loop_task;

unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;

float temp[3];
float hum[3];
float heat[3];
float light[3];

JsonDocument doc;

String dataToSend;
String server_payload;

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

  doc["device_id"] = "esp32";

  xTaskCreatePinnedToCore(server_loop, "server_loop", 4000, NULL, 2, &server_loop_task, 0);

  Serial.println("Reading levels in setup");
  readHumAndTemp();
  readLightLevel();
  parse_json_data();
}

/* ---------------------------- main loop ---------------------------------*/

void loop() 
{
  Serial.println("Reading levels in main loop");
  readHumAndTemp();
  readLightLevel();
  parse_json_data();
  post_data();
  delay(10000);
}

/* ---------------------------- Server loop ---------------------------------*/

void server_loop(void* pvParameters)
{
  Serial.println("second loop");
  server.begin(10000);

  while(1)
  {
    getClient = server.available();  

    if(getClient)
    {
      currentTime = millis();
      previousTime = currentTime;
      String currentLine = "";

      while (wifi.connected() && currentTime - previousTime <= timeoutTime) 
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
    vTaskDelay(1);
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

/* ---------------------------- Wifi wifi configuration ---------------------------------*/

void wifiConfig()
{
  WiFi.config(local_ip, gateway, snm);
  WiFi.begin(ssid, password);
  
  while(WiFi.status()!= WL_CONNECTED)
  {
    Serial.println("Not connected");
    delay(5000);
  }

  Serial.println(WiFi.localIP());
}

/* ---------------------------- send data as wifi ---------------------------------*/

void post_data()
{
  int httpResponseCode;
  
  //Serial.println(dataToSend);

  String contentType = "application/json";
  String payload = dataToSend;

  Serial.println(payload);
  //Serial.print("Tipo di payload: ");
  //Serial.println(typeof(payload));

  client.post("/postListener", contentType, payload);

  httpResponseCode = client.responseStatusCode();
  String response = client.responseBody();

  Serial.print("HTTP POST response code: ");
  Serial.println(httpResponseCode);

  //do
  //{
  //} while(httpResponseCode!=200); to implement later
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
    light[i] = bhSensor.readLightLevel();
}
  
