#include <Arduino.h>
#ifdef ESP32
  #include <WiFi.h>
  #include "SPIFFS.h"
#else
  #include <ESP8266WiFi.h>
#endif
#include "AudioFileSourceSPIFFS.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SNoDAC.h"

enum Pins{
  Led1 = 0,
  Dht = 1,
  Led2 = 2,
  Buzzer = 3,
  Hall = 12,
  Energia = 13,
  Motor = 14,
  BtWifi = 15,
  Wake = 16,
  BatSense = A0
};




// To run, set your ESP8266 build to 160MHz, and include a SPIFFS of 512KB or greater.
// Use the "Tools->ESP8266/ESP32 Sketch Data Upload" menu to write the MP3 to SPIFFS
// Then upload the sketch normally.

// pno_cs from https://ccrma.stanford.edu/~jos/pasp/Sound_Examples.html

AudioGeneratorMP3 *mp3;
AudioFileSourceSPIFFS *file;
AudioOutputI2SNoDAC *out;
AudioFileSourceID3 *id3;


// Called when a metadata event occurs (i.e. an ID3 tag, an ICY block, etc.
void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string)
{
  (void)cbData;
  Serial.printf("ID3 callback for: %s = '", type);

  if (isUnicode) {
    string += 2;
  }

  while (*string) {
    char a = *(string++);
    if (isUnicode) {
      string++;
    }
    Serial.printf("%c", a);
  }
  Serial.printf("'\n");
  Serial.flush();
}

int pinsTest[] = {5,4,0,2,15,13,12,14};
//1,16

void blink()
{
  unsigned long int atual=millis();
  unsigned long static next=1000;
  const static int interval=1000;
  static bool value = false;
  if(atual>next)
  {
    value = !value;
    for(int pin:pinsTest)
      digitalWrite(pin,value);
    next=atual+interval;
  }
}


void ocila()
{
  unsigned long int atual=millis();
  unsigned long static next=0;
  const static int interval=10;
  static int value = 0;
  if(atual>next)
  {
    value = (value>1013)?0:(value+10);
    for(int pin:pinsTest)
      analogWrite(pin,value);
    next=atual+interval;
  }

}

void play()
{
    file = new AudioFileSourceSPIFFS("/pno-cs.mp3");
    id3 = new AudioFileSourceID3(file);
    id3->RegisterMetadataCB(MDCallback, (void*)"ID3TAG");
    mp3->begin(id3, out);
}

void soundLoop()
{
  if (mp3->isRunning()) {
    if (!mp3->loop()){
      mp3->stop();
      delete mp3;
      delete file;
      ESP.deepSleep(15e6);}

  } else {
    Serial.printf("MP3 done\n");
    play();
  }
}

void setupPins()
{
  for(int pin:pinsTest)
    pinMode(pin,OUTPUT);
}

void longPress(){
  pinMode(Pins::Wake,INPUT_PULLUP);
  unsigned const long int LONGPRESS = 10000;
  unsigned long int start = millis();
  unsigned long int atual;
  unsigned long int end = start + LONGPRESS;
  int status;
  do{
    status = digitalRead(Pins::Wake);
    atual = millis();
    yield();
  }while(status==0 && atual < end);


  Serial.begin(115200);
  delay(1000);

  if(atual >= end){
    Serial.println("OCORREU LONG PRESS");
  }

  Serial.print("Time: ");
  Serial.println(atual-start);
}


void setup()
{
  //setupPins();
  //blink();
  //delay(1e3);
  //ESP.deepSleep(10e6);
  longPress();
  WiFi.mode(WIFI_OFF);

  SPIFFS.begin();
  Serial.printf("Sample MP3 playback begins...\n");
  // file = new AudioFileSourceSPIFFS("/pno-cs.mp3");
  // id3 = new AudioFileSourceID3(file);
  // id3->RegisterMetadataCB(MDCallback, (void*)"ID3TAG");
  out = new AudioOutputI2SNoDAC();
  mp3 = new AudioGeneratorMP3();
  // mp3->begin(id3, out);
  setupPins();
}

void loop()
{
  soundLoop();
  //blink();
  ocila();
}
