//Program to turn on / off the repeater at specific hours, verify voltage, temperature control and identify the station with Repeater Callsign
//Audio Out - pin 11
// SD card attached to SPI bus as follows:
 //MOSI - pin 51 or 11
 //MISO - pi 50 or 12
 //CLK - pin 52 or 13
 //CS - pin 53 

//display libery
#include <OLED_I2C.h>
#include <MD_DS1307.h>
#include <Wire.h>
//Repeater ID
#include "SD.h"
#include "TMRpcm.h"
#include "SPI.h"
//clock disply variable definition
extern uint8_t Sinclair_M[];
OLED myOLED (SDA, SCL); //pin in mega 2560 (20,21)
#define VT100_MODE  0
#define RELAY1 7  //Relay Shield, relay 1, Fan control
#define RELAY3 5  //Relay Shield, relay 3, Repeater power / and Remote control for the Radio 
#define RELAY2 6  //Relay Shield, relay 2, PTT 
#define Vol_samples 10 //amount of samples for better voltage reading
//Chip select is pin number 4 jumper to 53, SD Card shield use pin 4
#define SD_ChipSelectPin 53 
TMRpcm audio;
int sensorPin = 1;
int voldivPin = 3;
int t; // Time varialble
//clock variable definition
int clockCenterX=31;
int clockCenterY=31;
int oldsec=0;
int sum = 0;
unsigned char sample_count = 0;
float voltage = 0.0;

void setup()
{
myOLED.begin();
//Set the format font
myOLED.setFont(Sinclair_M);
 Serial.begin(9600);
if (!RTC.isRunning())
    RTC.control(DS1307_CLOCK_HALT, DS1307_OFF);
    
//set the hours, uncomment for setup the time
//RTC.h = 1; 
//RTC.m = 42;
//RTC.yyyy = 2018;
//RTC.dow = 7;
//RTC.mm = 01;
//RTC.dd = 27;
//RTC.writeTime();
pinMode(RELAY1,OUTPUT);
pinMode(RELAY3,OUTPUT);
pinMode(RELAY2,OUTPUT);
if(!SD.begin(SD_ChipSelectPin)){
  Serial.print("SD fail");
  return;
//Serial2.begin(9600); //Open the Serial communication with HC12 Rx(19) / Tx(18)
}
audio.speakerPin = 11; // Audio out on pin 11
audio.setVolume(3);  //0 to 7. set volume level
audio.quality(1);  //set 1 for 2x oversampling set 0 for normal

myOLED.clrScr();  
myOLED.print("WELCOME", CENTER, 0);
myOLED.print("Repeater Callsign", CENTER, 16);//Write repeater callsign
myOLED.print("REPEATER", CENTER, 32);
myOLED.print("GMRS", CENTER, 47);
myOLED.update();
delay(5000);
myOLED.clrScr();  
myOLED.print("Repeater Callsign", CENTER, 0);
myOLED.print("T462.575", LEFT, 32);//Repeater frequency Tx - edit the frequency
myOLED.print("R467.575", LEFT, 47);//Repeater frequency Rx - edit the frequency
myOLED.update();//Push the text to the display
delay(5000);
//This section if you want to add more infomation
myOLED.clrScr();  
myOLED.print("PARTNER", CENTER, 0);
myOLED.print("any name or Callsign", CENTER, 16);
myOLED.print("any name or Callsign", LEFT, 32);
myOLED.print("any name or Callsign", LEFT, 47);
myOLED.update();
delay(5000);
}

//building the clock 
const char *dow2String(uint8_t code)
{
  static const char *str[] = {"--- ", "Sun ", "Mon ", "Tue ", "Wed ", "Thu ", "Fri ", "Sat "};
  return(str[code]);
}

void drawDisplay()
{
  // Clear screen
  myOLED.clrScr();
 // Draw Clockface
  for (int i=0; i<2; i++)
  {
    myOLED.drawCircle(clockCenterX, clockCenterY, 31-i);
  }
  for (int i=0; i<3; i++)
  {
    myOLED.drawCircle(clockCenterX, clockCenterY, i);
  }

  // Draw a small mark for every hour
  for (int i=0; i<12; i++)
  {
  drawMark(i);
  }  

  RTC.readTime();
}

void drawMark(int h)
{
  float x1, y1, x2, y2;
  h=h*30;
  h=h+270;

  x1=29*cos(h*0.0175);
  y1=29*sin(h*0.0175);
  x2=26*cos(h*0.0175);
  y2=26*sin(h*0.0175);

  myOLED.drawLine(x1+clockCenterX, y1+clockCenterY, x2+clockCenterX, y2+clockCenterY);
}

void drawSec(int s)
{
  float x1, y1, x2, y2;
  s=s*6;
  s=s+270;

  x1=29*cos(s*0.0175);
  y1=29*sin(s*0.0175);
  x2=26*cos(s*0.0175);
  y2=26*sin(s*0.0175);

  if ((s % 5) == 0)
  myOLED.clrLine(x1+clockCenterX, y1+clockCenterY, x2+clockCenterX, y2+clockCenterY);

  else
  myOLED.drawLine(x1+clockCenterX, y1+clockCenterY, x2+clockCenterX, y2+clockCenterY);
}

void drawMin(int m)
{
  float x1, y1, x2, y2, x3, y3, x4, y4;
  m=m*6;
  m=m+270;

  x1=25*cos(m*0.0175);
  y1=25*sin(m*0.0175);
  x2=3*cos(m*0.0175);
  y2=3*sin(m*0.0175);
  x3=10*cos((m+8)*0.0175);
  y3=10*sin((m+8)*0.0175);
  x4=10*cos((m-8)*0.0175);
  y4=10*sin((m-8)*0.0175);

  myOLED.drawLine(x1+clockCenterX, y1+clockCenterY, x3+clockCenterX, y3+clockCenterY);
  myOLED.drawLine(x3+clockCenterX, y3+clockCenterY, x2+clockCenterX, y2+clockCenterY);
  myOLED.drawLine(x2+clockCenterX, y2+clockCenterY, x4+clockCenterX, y4+clockCenterY);
  myOLED.drawLine(x4+clockCenterX, y4+clockCenterY, x1+clockCenterX, y1+clockCenterY);
}

void drawHour(int h, int m)
{
  float x1, y1, x2, y2, x3, y3, x4, y4;
  h=(h*30)+(m/2);
  h=h+270;  

  x1=20*cos(h*0.0175);
  y1=20*sin(h*0.0175);
  x2=3*cos(h*0.0175);
  y2=3*sin(h*0.0175);
  x3=8*cos((h+12)*0.0175);
  y3=8*sin((h+12)*0.0175);
  x4=8*cos((h-12)*0.0175);
  y4=8*sin((h-12)*0.0175);

  myOLED.drawLine(x1+clockCenterX, y1+clockCenterY, x3+clockCenterX, y3+clockCenterY);
  myOLED.drawLine(x3+clockCenterX, y3+clockCenterY, x2+clockCenterX, y2+clockCenterY);
  myOLED.drawLine(x2+clockCenterX, y2+clockCenterY, x4+clockCenterX, y4+clockCenterY);
  myOLED.drawLine(x4+clockCenterX, y4+clockCenterY, x1+clockCenterX, y1+clockCenterY);
}

void printDate()
{
  myOLED.print(dow2String(RTC.dow), RIGHT, 0);
  myOLED.printNumI((RTC.mm), RIGHT, 16);
  myOLED.printNumI((RTC.dd), RIGHT, 32);
  myOLED.printNumI(RTC.yyyy, RIGHT, 47);
}

//voltage divider to verify voltage drop
void VolDiv()
{
while (sample_count < Vol_samples)//take 10 voltage reading samples to better value
{
sum += analogRead(voldivPin);
sample_count++;
delay(10);
}// Maybe the value 5.015 need to change to adjust the correct voltage
float voltage = ((float)sum / (float)Vol_samples * 5.015) / 1023.0; 
float voltageRadio = voltage * 11.15;//The 11.15 value is to adjust to the real voltage 
sample_count = 0;
sum = 0;
if(voltageRadio > 11.0) //Value to make decision, if the voltage is above of 10DC
{
digitalWrite(RELAY3,1);
myOLED.clrScr();  
myOLED.print("VOLTAGE", CENTER, 0);
myOLED.print("CONTROL:", CENTER, 16);
myOLED.print(">11.0VDC", LEFT, 32);
myOLED.printNumI((voltageRadio), CENTER, 47);
myOLED.update();
delay(2000);
}
 else if(voltageRadio < 10.9)//Turn off the repeater radio if the voltage is below the 10DC
{
delay(5000);
if(voltageRadio < 10.9)
{
digitalWrite(RELAY3,0);
myOLED.clrScr();  
myOLED.print("VOLTAGE", CENTER, 0);
myOLED.print("CONTROL:", CENTER, 16);
myOLED.print("<10.9VDC", LEFT, 32);
myOLED.printNumI((voltageRadio), CENTER, 47);
myOLED.update();
delay(2000);
}
}
}

//For safety monitoring the temperature of the controller box
void TempFan()
{
int reading = analogRead(sensorPin);
float voltageTemp = reading * 0.48828125;
float temperatureF = (voltageTemp * 9.0 / 5.0) + 32.0;
if(temperatureF < 100)
{
digitalWrite(RELAY1,0);
myOLED.clrScr();  
myOLED.print("TEMP.", CENTER, 0);
myOLED.print("CONTROL:", CENTER, 16);
myOLED.print("< 100 C", CENTER, 32);
myOLED.printNumI((temperatureF), CENTER, 47);
myOLED.update();
delay(2000);
}
else if(temperatureF > 100)
{
digitalWrite(RELAY1,1);
myOLED.clrScr();  
myOLED.print("TEMP.", CENTER, 0);
myOLED.print("CONTROL:", CENTER, 16);
myOLED.print("> 100 C", CENTER, 32);
myOLED.printNumI((temperatureF), CENTER, 47);
myOLED.update();
delay(2000);
}
}

//Using the clock to control the repeater and ID
void RepeaterControl()
{
  switch (RTC.h)//RTC.h is the hour of the clock to make decision
  {
    case 5 ... 22://Time of Repeater operation
    VolDiv();//First verify the voltage
    TempFan();//Second the Temperature
    RepeateRemoteOff();//Verify for remoter control if need to turn off/on for any reason
      switch (RTC.m)
      {
        case 00://every 00 minute the voice ID will play 
        myOLED.clrScr();  
        myOLED.print("Repeater Callsign", CENTER, 16);
        myOLED.print("AUTOMATIC", LEFT, 32);
        myOLED.print("Voice ID", CENTER, 47);
        myOLED.update();
        digitalWrite(RELAY2,1);//Active the PTT of the radio
        delay(1000);
        audio.play("VoiceID.wav");//Record the voice in .wav format
        delay(15000);//Time to allow to play voice Callsign
        digitalWrite(RELAY2,0);//
        delay(35000);
        break;
        case 30://every 30 minutes the CW ID will play
        myOLED.clrScr();  
        myOLED.print("Repeater Callsign", CENTER, 16);
        myOLED.print("AUTOMATIC", LEFT, 32);
        myOLED.print("CW ID", CENTER, 47);
        myOLED.update();
        digitalWrite(RELAY2,1);//Active the PTT of the radio
        delay(1000);
        audio.play("IDCW.wav");//Record the CW in .wav format
        delay(12000);//Time to allow to play CW Callsign
        digitalWrite(RELAY2,0);
        delay(35000);
        break;
        default:
        break;
        }
    break;
    default://During time 23hour to 4AM repeater is off, if remote control was not activated
    TempFan();//Verify temperature for safety
    RepeateRemoteOn(); //If need to turn on the repeater in the night
    myOLED.clrScr(); 
    myOLED.print("Repeater Callsign", CENTER, 0); 
    myOLED.print("TIMER", CENTER, 16);
    myOLED.print("CONTROL", CENTER, 32);
    myOLED.print("RADIO-OFF", LEFT, 47);
    myOLED.update();
    digitalWrite(RELAY3,0);
    delay(3000);
    break;
   }
}

void RepeateRemoteOn()//This is to turn on the repeater
{
  if (Serial.available()> 0) //Verify for serial communication
  {
  char serialData = Serial.read();
  Serial.println(serialData);
  delay(1000);
  if (serialData == 'A')//If receive "A" the repeater will turned on
   {
    myOLED.clrScr();  
    myOLED.print("Repeater Callsign", CENTER, 0);
    myOLED.print("REMOTE", CENTER, 16);
    myOLED.print("CONTROL", CENTER, 32);
    myOLED.print("ACTIVATED", LEFT, 47);
    myOLED.update();
    digitalWrite(RELAY3, 1);
    delay(2000);
    while (true)  // The repeater will remained on until receive a "B" from serial communication
     {
      char serialData = Serial.read();
       if (serialData == 'B')// If receive "B" will turned off
        {
    Serial.println(serialData);
    myOLED.clrScr();
    myOLED.print("Repeater Callsign", CENTER, 0);
    myOLED.print("REMOTE", CENTER, 16);
    myOLED.print("CONTROL", CENTER, 32);
    myOLED.print("DISARM", CENTER, 47);
    myOLED.update();
    digitalWrite(RELAY3,0);
    delay(2000);
    serialData = 0;
    Serial.flush();
    break;
         }
      }
    }
   }
  }


void RepeateRemoteOff()//This is to turn off to repeater if needed for any issue
{
  if (Serial.available()> 0) //Verify for serial communication
  {
  char serialData = Serial.read();
  Serial.println(serialData);
  delay(1000);
  if (serialData == 'B')//Recive "B" will be turned off
   {
    myOLED.clrScr();  
    myOLED.print("Repeater Callsign", CENTER, 0);
    myOLED.print("REMOTE", CENTER, 16);
    myOLED.print("CONTROL", CENTER, 32);
    myOLED.print("ACTIVATED", LEFT, 47);
    myOLED.update();
    digitalWrite(RELAY3,0);
    delay(2000);
    while (true)  // The repeater will remained on until receive a "A" from serial communication 
     {
      char serialData = Serial.read();
       if (serialData == 'A')//If revieve "A" will turned on
        {
    Serial.println(serialData);
    myOLED.clrScr();
    myOLED.print("Repeater Callsign", CENTER, 0);
    myOLED.print("REMOTE", CENTER, 16);
    myOLED.print("CONTROL", CENTER, 32);
    myOLED.print("DISARM", CENTER, 47);
    myOLED.update();
    digitalWrite(RELAY3,1);
    delay(2000);
    serialData = 0;
    Serial.flush();
    break;
         }
      }
    }
   }
}

void loop()//In this portion is were the program run
{//Build the clock
  int x, y;
  int prevSec;
  drawDisplay();
  drawSec(RTC.s);
  drawMin(RTC.m);
  drawHour(RTC.h, RTC.m);
  printDate();
  myOLED.update();//Display de Clock
  myOLED.clrScr();
  delay(5000);  
  RepeaterControl();//Run the repeater control
  delay(5000);
 }

  

