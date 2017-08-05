#ifndef NET_SETTING_MANAGER_H
#define NET_SETTING_MANAGER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "WiFiClient.h"
#include "WiFiServer.h"

class NetSettingManager {
	public:
		NetSettingManager(const char* ssid, const unsigned int port);
		void begin();
		void stop();
		void handleNetwork();
	
	protected:
	    int makePassword(String key, char* password);
	
	private:
		static const unsigned int sIP      = 0x0101A8C0; // 192.168.1.1
		static const unsigned int sGATEWAY = 0x0101A8C0; // 192.168.1.1
		static const unsigned int sSUBNET  = 0x00FFFFFF; // 255.255.255.0
	
	private:
		const char* mSSID;
		char mPassword[9];
		WiFiServer* mServer;
};

#endif