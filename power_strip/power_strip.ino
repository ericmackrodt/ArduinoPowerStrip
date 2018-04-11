#include <SPI.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <SerialMessenger.h>
SoftwareSerial ESPSerial(2, 3); // RX, TX
SerialMessenger messenger(&ESPSerial);

bool relays[8] = { false, false, false, false, false, false, false, false }; 

int CLOCK = 13;
int DATA = 11;
int LATCH = 4;
int BUTTONS = 7;

byte input, output, check=1;

void onStatus(String message) {
  messenger.send("status", getStatus());
}

void onTurnOn(String message) {
  enable_relay(message);
  messenger.send("on", getOn(message));
 
}

void onTurnOff(String message) {
  disable_relay(message);
  messenger.send("off", getOff(message));
}

void setup() {
  pinMode(CLOCK, OUTPUT);//clock
  pinMode(DATA, OUTPUT);//data
  pinMode(LATCH, OUTPUT);//latch
  pinMode(BUTTONS, INPUT);//Input from buttons
  
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.begin();
  SPI.transfer(255);
  SPI.transfer(0);
  digitalWrite(LATCH, HIGH);
  digitalWrite(LATCH, LOW);
  Serial.begin(115200);
  ESPSerial.begin(115200);

  messenger.subscribe("status", &onStatus);
  messenger.subscribe("on", &onTurnOn);
  messenger.subscribe("off", &onTurnOff);
  
  Serial.println("Started!");
}//setup

int count = 0;

String getStatus() {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  for (int i = 0; i < 8; i++) {
    root["relay_" + String(i)] = relays[i];
  }
  String result;
  root.printTo(result);
  return result;
}

String getOn(String relay) {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["status"] = relay;
  String result;
  root.printTo(result);
  return result;
}

String getOff(String relay) {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["status"] = relay;
  String result;
  root.printTo(result);
  return result;
}

void loop(){
  messenger.handle();
  
  if (digitalRead(BUTTONS)==HIGH) {
    delay(50);
  
      check=1;
      for (int j=0; j<8; j++) {
        SPI.transfer(check);
        SPI.transfer(output);
        digitalWrite(LATCH, HIGH);
        digitalWrite(LATCH, LOW);
        delayMicroseconds(500);
        if (digitalRead(BUTTONS) == HIGH) {
          if (relays[j] == true) 
            relays[j] = false;
          else
            relays[j] = true;
        }//dig check
        updateRelays(j);
        check = check<<1;
      }//j
        
      SPI.transfer(255);
      SPI.transfer(output);
      digitalWrite(LATCH, HIGH);
      digitalWrite(LATCH, LOW);
      
      while(digitalRead(BUTTONS)==HIGH){}
      count = 0;
  }

  updateRelays(count);
  
  count++;
  if (count > 7) { 
    count = 0;
    SPI.transfer(255);
    SPI.transfer(output);
    digitalWrite(LATCH, HIGH);
    digitalWrite(LATCH, LOW);
  }
}//loop

void updateRelays(int count) {
  SPI.transfer(0);
  SPI.transfer(output);

  if(relays[count] != bitRead(output, count)) {
    bitWrite(output, count, relays[count]);
  }
}

int enable_relay(String relay) {
  int count = relay.toInt();
  relays[count] = true;
  return 1;
}

int disable_relay(String relay) {
  int count = relay.toInt();
  relays[count] = false;
  return 1;
}


