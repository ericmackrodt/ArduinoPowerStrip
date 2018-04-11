#include <ESP8266WiFi.h>
//#include <WiFiClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>
#include <SerialMessenger.h>
#include <WiFiManager.h>
SerialMessenger messenger;

std::unique_ptr<ESP8266WebServer> server;

void handleNotFound() {
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server->uri();
	message += "\nMethod: ";
	message += ( server->method() == HTTP_GET ) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server->args();
	message += "\n";

	for ( uint8_t i = 0; i < server->args(); i++ ) {
		message += " " + server->argName ( i ) + ": " + server->arg ( i ) + "\n";
	}

	server->send ( 404, "text/plain", message );
}

void onStatus(String message) {
  server->send ( 200, "text/json", message ); 
}

void onTurnOnResponse(String message) {
  server->send ( 200, "text/json", message ); 
}

void onTurnOffResponse(String message) {
  server->send ( 200, "text/json", message ); 
}

void resetWifi(String message) {
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  ESP.restart();
}

void setup ( void ) {
	Serial.begin ( 115200 );
  messenger.subscribe("status", &onStatus);
  messenger.subscribe("on", &onTurnOnResponse);
  messenger.subscribe("off", &onTurnOffResponse);

  messenger.send("connecting");
  WiFiManager wifiManager;
  wifiManager.autoConnect("EricPowerStrip");
  

//	WiFi.mode ( WIFI_STA );
//	WiFi.begin ( ssid, password );
//	Serial.println ( "" );

	// Wait for connection
//	while ( WiFi.status() != WL_CONNECTED ) {
//		delay ( 500 );
//	}

//	Serial.println ( "" );
//	Serial.print ( "Connected to " );
//	Serial.println ( ssid );
//	Serial.print ( "IP address: " );
//	Serial.println ( WiFi.localIP() );

//	if ( MDNS.begin ( "esp8266" ) ) {
//		Serial.println ( "MDNS responder started" );
//	}

  server.reset(new ESP8266WebServer(WiFi.localIP(), 80));

	server->on ( "/status", []() { messenger.send("status"); } );
  server->on ( "/on", []() {
    if (server->argName(0) == "relay") {
      messenger.send("on", server->arg(0));
    }
  } );
  server->on ( "/off", []() {
    if (server->argName(0) == "relay") {
      messenger.send("off", server->arg(0));
    }
  } );
	server->onNotFound ( handleNotFound );
	server->begin();
	messenger.send("connected", String(WiFi.localIP()));
}

void loop ( void ) {
  server->handleClient();
  messenger.handle();
}
