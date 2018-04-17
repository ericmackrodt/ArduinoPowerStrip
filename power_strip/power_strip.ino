#include <SPI.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <SerialMessenger.h>

SoftwareSerial ESPSerial(2, 3); // RX, TX
SerialMessenger messenger(&ESPSerial);

#define DATA 11
#define LATCH 4
#define CLOCK 13
#define BUTTONS 8
#define MAIN_BUTTON 7

// BUTTON HELD
int buttonState = 0;     // current state of the button
int lastButtonState = 0; // previous state of the button
int startPressed = 0;    // the time button was pressed
int endPressed = 0;      // the time button was released
int timeHold = 0;        // the time button is hold
int timeReleased = 0;    // the time button is released

bool sockets[8] = { false, false, false, false, false, false, false, false };
byte output = 0;

void latch() {
  SPI.transfer(output); // Relays
  SPI.transfer(255); // Buttons
  SPI.transfer(output); // Leds
  digitalWrite(LATCH, HIGH);
  digitalWrite(LATCH, LOW);
}

void updateSockets() {
  for (int i = 0; i < 8; i++) {
    if (bitRead(output, i) != sockets[i]) {
      bitWrite(output, i, sockets[i]);
    }
  }

  latch();  
}

bool allSocketsSame() {
  bool previous = sockets[0];
  for (int i = 0; i < 8; i++) {
    if (sockets[i] != previous) {
      return false;
    }
    previous = sockets[i];
  }

  return true;
}

void readButtons() {
  if (digitalRead(BUTTONS)==HIGH) {
    delay(50);
    byte check = 1;
    for (int i = 0; i < 8; i++) {
      
      SPI.transfer(check);
      SPI.transfer(output);
      
      digitalWrite(LATCH, HIGH);
      digitalWrite(LATCH, LOW);
      
      delay(5);
    
      if (digitalRead(BUTTONS) == HIGH) {
        sockets[i] = !sockets[i];
      }//dig check
  
      check = check<<1;
    }//j
    
    latch();
 
    while(digitalRead(BUTTONS)==HIGH){}
  }
}

void readMainButton() {
  // read the pushbutton input pin:
  buttonState = digitalRead(MAIN_BUTTON);

  // button state changed
  if (buttonState != lastButtonState) {

    // the button was just pressed
    if (buttonState == HIGH) {
        startPressed = millis();
        timeReleased = startPressed - endPressed;
    // the button was just released
    } else {
        endPressed = millis();
        timeHold = endPressed - startPressed;

        if (timeHold < 5000) {
          if (allSocketsSame()) {
            for (int i = 0; i < 8; i++) {
              sockets[i] = !sockets[i];
            }
          } else {
            for (int i = 0; i < 8; i++) {
              sockets[i] = true;
            }
          }
        }

        if (timeHold >= 5000) {
            Serial.println("Button hold for five seconds or more"); 
        }

        Serial.println(timeHold);
    }
  }
  // save the current state as the last state, 
  //for next time through the loop
  lastButtonState = buttonState;
}

String getSocketResult(String relay, bool status) {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["socket_" + relay] = status;
  String result;
  root.printTo(result);
  return result;
}

String getStatus() {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  for (int i = 0; i < 8; i++) {
    root["socket_" + String(i)] = sockets[i];
  }
  String result;
  root.printTo(result);
  return result;
}

void onStatus(String message) {
  messenger.send("status", getStatus());
}

void switchSocket(String message, bool status) {
   if (message == "all") {
    for (int i = 0; i < 8; i++) {
      sockets[i] = status;
    }
  } else {
    int count = message.toInt();
    sockets[count] = status;
  }
}

void onTurnOn(String message) {
  switchSocket(message, true);
  messenger.send("on", getSocketResult(message, true));
 
}

void onTurnOff(String message) {
  switchSocket(message, false);
  messenger.send("off", getSocketResult(message, false));
}

void setup(){
  pinMode(CLOCK, OUTPUT);//clock
  pinMode(DATA, OUTPUT);//data
  pinMode(LATCH, OUTPUT);//latch
  
  pinMode(BUTTONS, INPUT);//Input from buttons
  pinMode(MAIN_BUTTON, INPUT);  
  
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.begin();
  
  latch();

  Serial.begin(115200);

  messenger.subscribe("status", &onStatus);
  messenger.subscribe("on", &onTurnOn);
  messenger.subscribe("off", &onTurnOff);
}//setup

void loop(){
  messenger.handle();
  updateSockets();
  readMainButton();  
  readButtons();
}//loop


