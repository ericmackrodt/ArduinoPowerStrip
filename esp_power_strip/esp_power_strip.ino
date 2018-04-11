#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <SerialMessenger.h>

const char *ssid = "Luminol";
const char *password = "spritelynevada666";

ESP8266WebServer server ( 80 );
SerialMessenger messenger;

void handleStatus() {
  messenger.send("status");
}

void handleOn() {
//  serial_manager.print("status;"); 
  if (server.argName(0) == "relay") {
    messenger.send("on", server.arg(0));
  }
}

void handleOff() {
//  serial_manager.print("status;"); 
  if (server.argName(0) == "relay") {
    messenger.send("off", server.arg(0));
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

void onStatus(String message) {
  server.send ( 200, "text/json", message ); 
}

void onTurnOnResponse(String message) {
  server.send ( 200, "text/json", message ); 
}

void onTurnOffResponse(String message) {
  server.send ( 200, "text/json", message ); 
}

void setup ( void ) {
	Serial.begin ( 115200 );
  messenger.subscribe("status", &onStatus);
  messenger.subscribe("on", &onTurnOnResponse);
  messenger.subscribe("off", &onTurnOffResponse);

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

void loop ( void ) {
  server.handleClient();
  messenger.handle();
}
