#include <SPI.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
SoftwareSerial ESPSerial(2, 3); // RX, TX

bool relays[8] = { false, false, false, false, false, false, false, false }; 

int CLOCK = 13;
int DATA = 11;
int LATCH = 4;
int BUTTONS = 7;

byte input, output, check=1;

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
  Serial.println("Started!");
}//setup

int count = 0;

String getCommand(String data) {
  int start = data.indexOf('#');
  int comma = data.indexOf(',');
  int colon = data.indexOf(';');

    if(start > -1 && comma > -1 && comma > start) {
      return data.substring(start + 1, comma);
    } else if (start > -1 && colon > -1 && colon > start) {
      return data.substring(start + 1, colon);
    } else {
      return data;
    }
}

String getParameter(String data) {
  int start = data.indexOf(',');
  int end = data.indexOf(';');

    if(start > -1 && end > start) {
      return data.substring(start + 1, end);
    }
    else {
      return data;
    }
}

bool isCommand(String data, String cmd) {
  return getCommand(data) == cmd;
}

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
  if (ESPSerial.available()) {
    // reads input from esp8266 and writes to serial console
    String data = ESPSerial.readString();
    if (isCommand(data, "status") == true) {
      ESPSerial.println("#status;" + getStatus() + "$"); 
    } else if (isCommand(data, "on") == true) {
      enable_relay(getParameter(data));
      ESPSerial.println("#on;" + getOn(getParameter(data)) + "$");
    } else if (isCommand(data, "off") == true) {
      disable_relay(getParameter(data));
      ESPSerial.println("#off;" + getOff(getParameter(data)) + "$");
    }
  }
  
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


