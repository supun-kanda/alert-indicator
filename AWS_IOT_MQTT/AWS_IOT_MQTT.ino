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
bool push_button_status = false, indicator_status = false, initializer = true;
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
    {
      alert_state = 2;
      Serial.print("\nError Message: ");
      Serial.println(alert_message);
      Serial.println("Turning Light ON");
    }
    else if (alert_type == "FIXED")
    {
      alert_state = 1;
      Serial.println("Turning Light OFF");
    }
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
  File file = SPIFFS.open(filename, "r");
  if (file)
  {
    Serial.print("Successfully opened ");
    Serial.println(filename);
  }
  else
  {
    Serial.print("Failed to open ");
    Serial.println(filename);
  }
  return file;
}
////////////////////////////////////////////////////////////

/////////////////   Set-Up Wifi   //////////////////////////
void setup_wifi()
{
  // We start by connecting to a WiFi network
  espClient.setBufferSizes(512, 512);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

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
  if (t > ref)
  {
    indicator_status = !indicator_status;
    ref = ref + 500;
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
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESPthing"))
    {
      alert_state = 1;
      Serial.println("connected");

      // publish connected message
      client.publish("device-health", "{\"message\":\"device-1-connected\"}");
      // ... start subscribing
      client.subscribe("cm-alerts");
    }
    else
    {
      alert_state = 0;
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");

      char buf[256];
      espClient.getLastSSLError(buf, 256);
      Serial.print("WiFiClientSecure SSL error: ");
      Serial.println(buf);

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

  Serial.begin(115200);
  Serial.setDebugOutput(true);

  setup_wifi();
  delay(1000);

  if (!SPIFFS.begin())
  {
    Serial.println("Failed to mount file system");
    return;
  }

  Serial.print("Heap: ");
  Serial.println(ESP.getFreeHeap());

  // Load certificate files
  File cert = loadFile("/cert.der");
  File private_key = loadFile("/private.der");
  File ca = loadFile("/ca.der");

  if (espClient.loadCertificate(cert))
    Serial.println("cert loaded");
  else
    Serial.println("cert not loaded");

  if (espClient.loadPrivateKey(private_key))
    Serial.println("private key loaded");
  else
    Serial.println("private key not loaded");

  if (espClient.loadCACert(ca))
    Serial.println("ca loaded");
  else
    Serial.println("ca failed");

  Serial.print("Heap: ");
  Serial.println(ESP.getFreeHeap());
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

  push_button_status = (digitalRead(PUSH_BUTTON)) ? 1 : 0;

  if (push_button_status || alert_state == 2)
    alerts(2);
  else if (alert_state)
    alerts(1);
  else
    alerts(0);

  delay(20);
}
////////////////////////////////////////////////////////////
