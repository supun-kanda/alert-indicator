#include "FS.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

#define D5 14
#define D6 12
#define D7 13

#define RELAY_SIGNAL D5
#define INDICATOR D6
#define PUSH_BUTTON D7

//const char* ssid = "MBR1200B-6bf";
//const char* password = "dyvcwkta007rulz";

const char *ssid = "Kadzzzzzzz";
const char *password = "(kandambi)";

//const char *ssid = "Dialog 4G";
//const char *password = "TFGHHTYA79G";

DynamicJsonDocument doc(1024);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

const char *AWS_endpoint = "abx9e94fmlpan-ats.iot.us-west-2.amazonaws.com"; //MQTT broker ip

WiFiClientSecure espClient;

int alert_state = 0;
bool indicator_status = false, initializer = true;
unsigned long t = 0, ref = 0;
String alert_type, alert_message;

///////////////// Message CallBack /////////////////////////
void callback(char *topic, byte *payload, unsigned int length)
{

  deserializeJson(doc, payload);
  alert_type = doc["type"].as<String>();
  alert_message = doc["message"].as<String>();

  if ((String)topic == "cm-alerts")
  {
    if (alert_type == "ERROR")
      alert_state = 2;
    else if (alert_type == "FIXED")
      alert_state = 1;
    initializer = true;
  }
}
////////////////////////////////////////////////////////////

PubSubClient client(AWS_endpoint, 8883, callback, espClient); //set  MQTT port number to 8883 as per standard
long lastMsg = 0;
char msg[50];
int value = 0;

/////////////////  Load a File   ///////////////////////////
File loadFile(char *filename)
{
  return SPIFFS.open(filename, "r");
}
////////////////////////////////////////////////////////////

/////////////////   Set-Up Wifi   //////////////////////////
void setup_wifi()
{
  // We start by connecting to a WiFi network
  espClient.setBufferSizes(512, 512);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(50);
  }

  timeClient.begin();
  while (!timeClient.update())
  {
    timeClient.forceUpdate();
  }

  espClient.setX509Time(timeClient.getEpochTime());
}
////////////////////////////////////////////////////////////

/////////////////////   Alert   ////////////////////////////
void alerts(int status)
{
  digitalWrite(INDICATOR, flasher_value(status));
  if (status == 2)
    digitalWrite(RELAY_SIGNAL, status);
  else
    digitalWrite(RELAY_SIGNAL, LOW);
}
////////////////////////////////////////////////////////////

///////////////////   Flasher   ////////////////////////////
bool flasher_value(int status)
{
  if (status == 1)
    return true;
  if (!status)
    return false;

  t = millis();
  if (initializer)
  {
    ref = t;
    initializer = false;
  }
  if (t - ref > 500)
  { 
    indicator_status = !indicator_status;
    ref = t;
  }
  return indicator_status;
}
////////////////////////////////////////////////////////////

/////////////////  Re-Connect  /////////////////////////////
void reconnect()
{
  // Loop until reconnected
  while (!client.connected())
  {
    // Attempt to connect
    if (client.connect("ESPthing"))
    {
      alert_state = 1;

      // publish connected message
      client.publish("device-health", "{\"message\":\"device-1-connected\"}");
      // ... start subscribing
      client.subscribe("cm-alerts");
    }
    else
    {
      alerts(0)

      char buf[256];
      espClient.getLastSSLError(buf, 256);

      // Wait 5 seconds before retrying
      delay(5000);
    }
    initializer = true;
  }
}
////////////////////////////////////////////////////////////

/////////////////////   Set Up   ///////////////////////////
void setup()
{
  pinMode(D5, OUTPUT); // RELAY_SIGNAL
  pinMode(D6, OUTPUT); //INDICATOR
  pinMode(D7, INPUT);  //PUSH_BUTTON

  alerts(0);

  setup_wifi();
  delay(1000);

  if (!SPIFFS.begin())
    return;

  // Load certificate files
  File cert = loadFile("/cert.der");
  File private_key = loadFile("/private.der");
  File ca = loadFile("/ca.der");

  espClient.loadCertificate(cert);
  espClient.loadPrivateKey(private_key);
  espClient.loadCACert(ca);
}
////////////////////////////////////////////////////////////

//////////////////////    Loop    //////////////////////////
void loop()
{
  if (!client.connected())
  {
    alert_state = 0;
    initializer = true;
    reconnect();
  }
  client.loop();

  if (digitalRead(PUSH_BUTTON) || alert_state == 2)
    alerts(2);
  else if (alert_state)
    alerts(1);
  else
    alerts(0);

  delay(20);
}
////////////////////////////////////////////////////////////
