/*
 * Copyright (c) 2015, Majenko Technologies
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * * Neither the name of Majenko Technologies nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
//#include <RBD_SerialManager.h>

const char *ssid = "Luminol";
const char *password = "spritelynevada666";

ESP8266WebServer server ( 80 );
//RBD::SerialManager serial_manager;

void handleStatus() {
//  serial_manager.print("status;"); 
  Serial.println("#status;");
}

void handleOn() {
//  serial_manager.print("status;"); 
  if (server.argName(0) == "relay") {
    Serial.println("#on," + server.arg(0) + ";");
  }
}

void handleOff() {
//  serial_manager.print("status;"); 
  if (server.argName(0) == "relay") {
    Serial.println("#off," + server.arg(0) + ";");
  }
}

void handleNotFound() {
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";

	for ( uint8_t i = 0; i < server.args(); i++ ) {
		message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
	}

	server.send ( 404, "text/plain", message );
}

void setup ( void ) {
	Serial.begin ( 115200 );
//  serial_manager.setFlag(';');

	WiFi.mode ( WIFI_STA );
	WiFi.begin ( ssid, password );
//	Serial.println ( "" );

	// Wait for connection
	while ( WiFi.status() != WL_CONNECTED ) {
		delay ( 500 );
	}

//	Serial.println ( "" );
//	Serial.print ( "Connected to " );
//	Serial.println ( ssid );
//	Serial.print ( "IP address: " );
//	Serial.println ( WiFi.localIP() );

	if ( MDNS.begin ( "esp8266" ) ) {
//		Serial.println ( "MDNS responder started" );
	}

	server.on ( "/status", handleStatus );
  server.on ( "/on", handleOn );
  server.on ( "/off", handleOff );
	server.on ( "/inline", []() {
		server.send ( 200, "text/plain", "this works as well" );
	} );
	server.onNotFound ( handleNotFound );
	server.begin();
	Serial.println ( "HTTP server started" );
}

String getCommand(String data) {
  int start = data.indexOf('#');
  int comma = data.indexOf(';');
  int colon = data.indexOf('$');

    if(start > -1 && comma > -1 && comma > start) {
      return data.substring(start + 1, comma);
    } else if (start > -1 && colon > -1 && colon > start) {
      return data.substring(start + 1, colon);
    } else {
      return data;
    }
}

String getParameter(String data) {
  int start = data.indexOf(';');
  int end = data.indexOf('$');

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

void loop ( void ) {
  server.handleClient();
  if (Serial.available()) {
    // reads input from esp8266 and writes to serial console
    String data = Serial.readString();
    if (isCommand(data, "status") == true) {
      server.send ( 200, "text/json", getParameter(data) );
    } else if (isCommand(data, "on") == true) {
      server.send ( 200, "text/json", getParameter(data) );
    } else if (isCommand(data, "off") == true) {
      server.send ( 200, "text/json", getParameter(data) );
    }
  }
}
