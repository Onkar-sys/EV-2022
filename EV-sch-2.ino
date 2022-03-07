#define TINY_GSM_MODEM_SIM800              // MODEM NAME 
#include "EmonLib.h"                       // Include Emon Library

#include <SoftwareSerial.h>
#include <TinyGsmClient.h>                 //MQTT Arduino library for GPRS modules 
#include <PubSubClient.h>

SoftwareSerial SerialAT(2, 3);             // RX, TX

//Network details
//const char apn[]  = "internet";
const char apn[]  = "M2MISAFE";
const char user[] = "";
const char pass[] = "";

// MQTT details
const char* broker = "139.59.90.224";
const char* topicCurrent1 = "EV-2/SCH/C1";     // Topic for current 1 value
//const char* topicCurrent1 = "EV-3/C1";
const char* topicRelay1 = "EV-2/SCH/R1";
//const char* topicRelay1 = "EV-3/R1";// Topic for current 1 value

const char* mqttUser = "whizkey";          //MQTT Username
const char* mqttPassword = "whizkey";      //MQTT Password for authentication

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
PubSubClient mqtt(client);

#define CURRENT1_CAL 41                   //current calibartion 
     
#define VOLT 230                          
String receive;

//Create emon instances(objects)
EnergyMonitor emon1;

// assigning current variable
float current1;

bool test;
bool modemLoop;

void setup()
{
  //Baud Rate
  Serial.begin(9600);
  SerialAT.begin(9600);

  //assigning PIN number for relay R1 and relay R2
  pinMode(13, OUTPUT);                      //FOR RELAY 1
  
  digitalWrite(13, LOW);                   // making RELAY LOW AT START 
  emon1.current(7, CURRENT1_CAL);          // assigning PIN 7 FOR CURRENT SENSOR 1
  

  Serial.println("System start.");
  modem.restart();
  Serial.println("Modem: " + modem.getModemInfo());
  Serial.println("Searching for telco provider.");
  if (!modem.waitForNetwork())
  {
    Serial.println("fail");
    while (true);
  }
  Serial.println("Connected to telco.");
  Serial.println("Signal Quality: " + String(modem.getSignalQuality()));

  Serial.println("Connecting to GPRS network.");
  if (!modem.gprsConnect(apn, user, pass))
  {
    Serial.println("fail");
    while (true);
  }
  Serial.println("Connected to GPRS: " + String(apn));

  mqtt.setServer(broker, 1883);
  mqtt.setCallback(mqttCallback);
  Serial.println("Connecting to MQTT Broker: " + String(broker));
  while (mqttConnect() == false) continue;
  Serial.println();
}

void loop()
{

  Serial.println("Working properly");
  emon1.calcVI(20, 500);            // 20,2000 //
  current1 = emon1.Irms;
 
  if (current1 > 0.9)
  {
    current1 = current1;
  }
  else
  {
    current1 = 0;
  }
  
  String message1 =  String(current1);


  // message+=(char)Serial.read();
  mqtt.publish(topicCurrent1, message1.c_str());

  if (mqtt.connected())
  {
    Serial.println("In the mqtt loop");
    mqtt.loop();
  }
  else{
    TryReconnection();
    }
   
  
}

boolean mqttConnect()
{
  if (!mqtt.connect("test_client_1", mqttUser, mqttPassword))
  {
    Serial.print(".");
    return false;
  }
  Serial.println("Connected to broker.");
  mqtt.subscribe(topicRelay1);
  return mqtt.connected();
}

void mqttCallback(char* topic, byte* payload, unsigned int len)
{
  Serial.print("Message receive: ");
  Serial.write(payload, len);
  receive = payload;
  Serial.println(receive);
  if (receive[1] == '1')
  { if (receive[3] == '1')
    {
      digitalWrite(13, HIGH);       // Making Relay 1 High
      Serial.println("Relay 1 ON");

    }
    else if (receive[3] == '0')
    {
      digitalWrite(13, LOW);         // Making Relay 1 Low
      Serial.println("Relay 1 OFF");
    }
  }  
}



void TryReconnection(){
  uno:
Serial.println("System start.");
  modem.restart();
  Serial.println("Modem: " + modem.getModemInfo());
  Serial.println("Searching for telco provider.");
  if (!modem.waitForNetwork())
  {
    Serial.println("fail");
    goto uno;
    while (true);
  }
  Serial.println("Connected to telco.");
  Serial.println("Signal Quality: " + String(modem.getSignalQuality()));

  Serial.println("Connecting to GPRS network.");
  if (!modem.gprsConnect(apn, user, pass))
  {
    Serial.println("fail");
    goto uno;
    while (true);
  }
  Serial.println("Connected to GPRS: " + String(apn));

  mqtt.setServer(broker, 1883);
  mqtt.setCallback(mqttCallback);
  Serial.println("Connecting to MQTT Broker: " + String(broker));
  while (mqttConnect() == false) continue;
  Serial.println();

  
  
}
