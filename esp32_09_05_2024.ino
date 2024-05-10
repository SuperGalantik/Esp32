
/**
  Author: Gianluca Galanti 4C-IN
  Version: 09/05/2024
*/

#include <ESPAsyncWebServer.h>
#include <Math.h>
#include <AsyncTCP.h>

#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>
//#include <WiFi101.h>

#include <Wire.h>
#include <BH1750.h>

#include <DHT.h>
#include <DHT_U.h>

#include <WiFi.h>
#include <WiFiClient.h>

#define DHTPIN 13

#define SDAPIN 21
#define SCLPIN 22

#define LASERPIN 15

char *ssid = "dlink";
char *password = ""; 

IPAddress ip;

DHT dhtSensor(DHTPIN, DHT22);

BH1750 bhSensor;

WiFiClient wifi;

WiFiClient getClient;

IPAddress local_ip(10,0,3,5);
IPAddress gateway(10,0,3,1);
IPAddress snm(255,255,255,0);

String server_address = "10.25.0.15";
int port = 10000;

HttpClient client = HttpClient(wifi, server_address, port);

AsyncWebServer webServer(80);
void notFound(AsyncWebServerRequest *request) { request->send(404, "text/plain", "Not found"); }

TaskHandle_t humAndTempTask;
TaskHandle_t lightTask;

TaskHandle_t webServerHumAndTempTask;
TaskHandle_t webServerLightTask;

unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;

double temp[3];
double hum[3];
double heat[3];
double light[3];

JsonDocument doc;

String dataToSend;
String server_payload;

void setup() 
{
  Serial.begin(9600);

  doc["device_id"] = "esp32";

  wifiConfig();

  dhtSensor.begin();

  Wire.begin();

  bhSensor.begin();

  webServerSetup();

  //pinMode(LASERPIN, OUTPUT);
}

/* ---------------------------- main loop ---------------------------------*/

void loop() 
{
  Serial.println("Reading levels in main loop");
  readHumAndTemp();
  readLightLevel();
  parse_json_data();
  post_data();
  delay(15000);
}

/* ---------------------------- Server loop ---------------------------------*/

void webServerSetup()
{
  webServer.on("/get_actuals", HTTP_GET, [](AsyncWebServerRequest *request) 
  {
    readHumAndTemp();
    readLightLevel();
    parse_json_data();
    String payload = dataToSend;

    request->send(200, "application/json", payload); 
  });
  webServer.begin();
}

/*----------------------------- Read levels -----------------------*/

/*
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
*/

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
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid);
  
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

  String contentType = "application/json";
  String payload = dataToSend;

  Serial.println(payload);
  
  do
  {
    client.post("/postListener", contentType, payload);
    httpResponseCode = client.responseStatusCode();
    String response = client.responseBody();
  } while(httpResponseCode!=200);
  
  Serial.print("HTTP POST response code: ");
  Serial.println(httpResponseCode);
}

/* ---------------------------- read humidity, temperature and heat index ---------------------------------*/

void readHumAndTemp()
{
  int i;
  for(i=0; i<3; i++)
  {
    temp[i] = round(dhtSensor.readTemperature()*100.00)/100.00;
    delay(500);
    hum[i] = round(dhtSensor.readHumidity()*100.00)/100.00;
    delay(500);
    heat[i] = round(dhtSensor.computeHeatIndex()*100.00)/100.00;
    if(i!=2)
      delay(500);
  }
}

/* ---------------------------- read light level ---------------------------------*/

void readLightLevel()
{
  int i;
  for(i=0; i<3; i++)
    light[i] = round(bhSensor.readLightLevel()*100.00)/100.00;
}
