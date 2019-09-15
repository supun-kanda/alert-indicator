#include "FS.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

#define LED_BUILTIN 2

//const char* ssid = "MBR1200B-6bf";
//const char* password = "dyvcwkta007rulz";

DynamicJsonDocument doc(1024);
String alert_type;
String alert_message;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

const char *AWS_endpoint = "abx9e94fmlpan-ats.iot.us-west-2.amazonaws.com"; //MQTT broker ip

void callback(char *topic, byte *payload, unsigned int length)
{

  deserializeJson(doc, payload);
  alert_type = doc["type"].as<String>();
  alert_message = doc["message"].as<String>();

  if ((String)topic == "cm-alerts" && alert_type == "ERROR")
  {
    digitalWrite(LED_BUILTIN, LOW); // Turn the LED on
    Serial.print("\nError Message: ");
    Serial.println(alert_message);
    Serial.println("Turning Light ON");
  }
  else
  {
    digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off
    Serial.println("Turning Light OFF");
  }
}

WiFiClientSecure espClient;
PubSubClient client(AWS_endpoint, 8883, callback, espClient); //set  MQTT port number to 8883 as per standard
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi()
{

  delay(10);
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

void reconnect()
{
  // Loop until reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESPthing"))
    {

      Serial.println("connected");
      // publish connected message
      client.publish("device-health", "{\"message\":\"device-1-connected\"}");
      // ... start subscribing
      client.subscribe("cm-alerts");
    }
    else
    {
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
  }
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);    // Initialize the BUILTIN_LED pin as an output
  digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off at the begining

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

  // Load certificate file
  File cert = SPIFFS.open("/cert.der", "r");
  if (!cert)
  {
    Serial.println("Failed to open cert file");
  }
  else
    Serial.println("Success to open cert file");

  delay(1000);

  if (espClient.loadCertificate(cert))
    Serial.println("cert loaded");
  else
    Serial.println("cert not loaded");

  // Load private key file
  File private_key = SPIFFS.open("/private.der", "r");
  if (!private_key)
  {
    Serial.println("Failed to open private cert file");
  }
  else
    Serial.println("Success to open private cert file");

  delay(1000);

  if (espClient.loadPrivateKey(private_key))
    Serial.println("private key loaded");
  else
    Serial.println("private key not loaded");

  // Load CA file
  File ca = SPIFFS.open("/ca.der", "r");
  if (!ca)
  {
    Serial.println("Failed to open ca ");
  }
  else
    Serial.println("Success to open ca");

  delay(1000);

  if (espClient.loadCACert(ca))
    Serial.println("ca loaded");
  else
    Serial.println("ca failed");

  Serial.print("Heap: ");
  Serial.println(ESP.getFreeHeap());
}

void loop()
{

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
}
