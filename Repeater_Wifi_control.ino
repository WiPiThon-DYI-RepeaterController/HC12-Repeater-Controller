//Components: ESP-01,relay module, buzzer, 5v power supply

#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//#define RELAY         0
#define LED             13//ESP LED internal light

const char* ssid = "Your WiFi name";
const char* password =  "Your WiFi Password";
const char* mqttServer = "Ip Address where the MQTT is installed Example 10.0.0.10 or 192.168.0.10";
const int mqttPort =1883; //default port for MQTT
const char* mqttUser = "Enter MQTT user name";
const char* mqttPassword = "Enter MQTT password";
WiFiClient esprepeateR; // You can change espClient for any name
PubSubClient client(espReceive);

bool Status = false;
int kUpdFreq = 1;
int kRetries = 10;

unsigned long TTasks;

void setup() {
//Put your setup code here, to run once:
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);//Turn off the light
   
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  }
  
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  while (!client.connected()) {
    
    if(client.connect("ESPreceiver", mqttUser, mqttPassword)) 
{
  
    }else{
      Serial.print("failed state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  
  client.subscribe("ESP/repeater/set");// here is where you later add a wildcard
}
  
void callback(char* topic, byte* payload, unsigned int length)
{
  
  for(int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  if ((char)payload[0] == '1'){ //In Home Assistant when turn "ON", send "1" to turn "ON" the repeater
    //digitalWrite(0, 1);
    digitalWrite(LED, LOW);//Turn "ON" the ESP LED
    Serial.write('A');//Will send the charater "A" via ESP Tx to HC12 device
    client.publish("ESP/repeater","on", true);//Publish "ON" to Home Assistant to display "ON" state
    delay(2000);
    }
    else{//In Home Assistant when turn "OFF", send "0" to turn "Off" the repeater
    Serial.write('B');//Will send the charater "B" via ESP Tx to HC12 device
    //digitalWrite(LED, HIGH);//Turn "OFF" the ESP LED
    client.publish("ESP/repeater","off", true);//Publish "OFF" to Home Assistant to display "OFF" state
      }
  Status = true;
  }

void sendStatus()
{
  if (Status){
    if (digitalRead(LED) == LOW){
    
    }else{
     
    }
    Status = false;
  }
 }
 

void checkConnection(){
  if (WiFi.status() != WL_CONNECTED){
    if (client.connected()){
      //Serial.println("connected in check");
      }
    else{
      reconnect();
      }
  }
    else{
      ESP.restart();
      }
   }
   

void reconnect() {
  // Loop until reconnected
  while (!client.connected()) {
    if (client.connect("ESPreceiver", mqttUser, mqttPassword)){
       client.subscribe("ESP/repeater/set");
    } else {
      Serial.print(client.state());
      delay(5000);
    }
  }
}
//To verify the WIFI connection
void timedTasks() {
  if ((millis() > TTasks + (kUpdFreq * 60000)) || (millis() < TTasks)) {
    TTasks = millis();
    checkConnection();
  }
}

void loop() {
  
  // put your main code here, to run repeatedly:
 if (!client.connected()) {
    reconnect();
  } 
  if(!client.loop())
  client.connect("ESPreceiver", mqttUser, mqttPassword);
  delay(200);
  client.connect("espReceive");
  delay(500);
  timedTasks();
  sendStatus(); 
}




