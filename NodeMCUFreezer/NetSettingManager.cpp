#include "NetSettingManager.h"
#include <Hash.h>

NetSettingManager::NetSettingManager(const char* ssid, const unsigned int port) {
	mSSID = ssid;
	mServer = new WiFiServer(port);
	memset(mPassword, 0x00, 8);
}

void NetSettingManager::begin() {
	if (mSSID == NULL || mPassword == NULL) {
		return;
	}

	String mac = WiFi.softAPmacAddress();
	makePassword("abc", mPassword);
	WiFi.softAP(mSSID, mPassword);
	WiFi.softAPConfig(sIP, sGATEWAY, sSUBNET);
	
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
	}

	if (Serial) {
		Serial.print("[beginBroadcast] SSID = ");
		Serial.print(mSSID);
		Serial.print(", PWD = ");
		Serial.print(mPassword);
		Serial.print(", MAC = ");
		Serial.println(mac);
		Serial.print("[beginBroadcast] LocalIP = ");
		Serial.println(WiFi.softAPIP());
	}
	
	if (mServer != NULL) {
		mServer->begin();
	}
}

void NetSettingManager::stop() {
	memset(mPassword, 0x00, 8);
	WiFi.softAPdisconnect(true);
}

void NetSettingManager::handleNetwork() {
	if (mServer == NULL)
		return;
	
	// https://www.arduino.cc/en/Reference/Stream
	WiFiClient client = mServer->available();
	if (client) {
		client.flush();
		
		if (client.available() > 0) {
			if (Serial) {
				Serial.println(client.readStringUntil('\r'));
			}
		}
	}
}

int NetSettingManager::makePassword(String key, char* password) {
	byte hash[8];
	sha1(key, &hash[0]);
	sprintf(password, "%02X%02X%02X%02X", hash[0], hash[1], hash[2], hash[3]);
	return 8;
}
