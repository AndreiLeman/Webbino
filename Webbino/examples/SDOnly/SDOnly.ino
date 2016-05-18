/***************************************************************************
 *   This file is part of SmartStrip.                                      *
 *                                                                         *
 *   Copyright (C) 2012-2016 by SukkoPera                                  *
 *                                                                         *
 *   SmartStrip is free software: you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   SmartStrip is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with SmartStrip.  If not, see <http://www.gnu.org/licenses/>.   *
 ***************************************************************************/

#include <SD.h>
#include <Webbino.h>
#include <avr/pgmspace.h>

/* SS pin for the SD card reader. Pin 4 is used for the reader included
 * on most WIZ5100-based Ethernet shields
 */
#define SD_SS 4

// Instantiate the WebServer
WebServer webserver;

// Instantiate the network interface defined in the Webbino headers
#if defined (WEBBINO_USE_ENC28J60)
	#include <WebServer_ENC28J60.h>
	NetworkInterfaceENC28J60 netint;
#elif defined (WEBBINO_USE_WIZ5100)
	#include <WebServer_WIZ5100.h>
	NetworkInterfaceWIZ5100 netint;
#elif defined (WEBBINO_USE_ESP8266)
	#include <WebServer_ESP8266.h>

	#include <SoftwareSerial.h>
	SoftwareSerial swSerial (7, 8);

	// Wi-Fi parameters
	#define WIFI_SSID        "ssid"
	#define WIFI_PASSWORD    "password"

	NetworkInterfaceESP8266 netint;
#endif


/******************************************************************************
 * DEFINITION OF TAGS                                                         *
 ******************************************************************************/

#define REP_BUFFER_LEN 32
static char replaceBuffer[REP_BUFFER_LEN];
PString subBuffer (replaceBuffer, REP_BUFFER_LEN);

static PString& ip2str (const byte *buf) {
	for (byte i = 0; i < 4; i++) {
		subBuffer.print (buf[i]);

		if (i < 3)
			subBuffer.print ('.');
	}

	return subBuffer;
}

static PString& evaluate_ip (void *data __attribute__ ((unused))) {
 	return ip2str (netint.getIP ());
}

static PString& evaluate_netmask (void *data __attribute__ ((unused))) {
	return ip2str (netint.getNetmask ());
}

static PString& evaluate_gw (void *data __attribute__ ((unused))) {
	return ip2str (netint.getGateway ());
}

static PString& evaluate_mac_addr (void *data __attribute__ ((unused))) {
	const byte *buf = netint.getMAC ();

	for (byte i = 0; i < 6; i++) {
		if (buf[i] < 16)
			subBuffer.print ('0');
		subBuffer.print (buf[i], HEX);

		if (i < 5)
			subBuffer.print (':');
	}

	return subBuffer;
}

static PString& evaluate_ip_src (void *data __attribute__ ((unused))) {
	if (netint.usingDHCP ())
		subBuffer.print (F("DHCP"));
	else
		subBuffer.print (F("MANUAL"));

	return subBuffer;
}

static PString& evaluate_webbino_version (void *data __attribute__ ((unused))) {
	subBuffer.print (F("WEBBINO_VERSION"));

	return subBuffer;
}

static PString& evaluate_uptime (void *data __attribute__ ((unused))) {
	unsigned long uptime = millis () / 1000;
	byte d, h, m, s;

	d = uptime / 86400;
	uptime %= 86400;
	h = uptime / 3600;
	uptime %= 3600;
	m = uptime / 60;
	uptime %= 60;
	s = uptime;

	replaceBuffer[0] = '\0';

	if (d > 0) {
		itoa (d, replaceBuffer, DEC);
		strcat_P (replaceBuffer, d == 1 ? PSTR (" day, ") : PSTR (" days, "));
	}

	// Shorter format: "2 days, 4:12:22", saves 70 bytes and doesn't overflow :D
	if (h < 10)
		subBuffer.print ('0');
	subBuffer.print (h);
	subBuffer.print (':');
	if (m < 10)
		subBuffer.print ('0');
	subBuffer.print (m);
	subBuffer.print (':');
	if (s < 10)
		subBuffer.print ('0');
	subBuffer.print (s);

	return subBuffer;
}

static PString& evaluate_free_ram (void *data __attribute__ ((unused))) {
	extern int __heap_start, *__brkval;
	int v;

	subBuffer.print ((int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval));

	return subBuffer;
}



// Max length of these is MAX_TAG_LEN (24)
static const char subMacAddrStr[] PROGMEM = "NET_MAC";
static const char subIPAddressStr[] PROGMEM = "NET_IP";
static const char subNetmaskStr[] PROGMEM = "NET_MASK";
static const char subGatewayStr[] PROGMEM = "NET_GW";
static const char subNetConfSrcStr[] PROGMEM = "NET_CONF_SRC";
static const char subWebbinoVerStr[] PROGMEM = "WEBBINO_VER";
static const char subUptimeStr[] PROGMEM = "UPTIME";
static const char subFreeRAMStr[] PROGMEM = "FREERAM";

static const var_substitution subMacAddrVarSub PROGMEM = {subMacAddrStr, evaluate_mac_addr, NULL};
static const var_substitution subIPAddressVarSub PROGMEM = {subIPAddressStr, evaluate_ip, NULL};
static const var_substitution subNetmaskVarSub PROGMEM = {subNetmaskStr, evaluate_netmask, NULL};
static const var_substitution subGatewayVarSub PROGMEM = {subGatewayStr, evaluate_gw, NULL};
static const var_substitution subNetConfSrcVarSub PROGMEM = {subNetConfSrcStr, evaluate_ip_src, NULL};
static const var_substitution subWebbinoVerVarSub PROGMEM = {subWebbinoVerStr, evaluate_webbino_version, NULL};
static const var_substitution subUptimeVarSub PROGMEM = {subUptimeStr, evaluate_uptime, NULL};
static const var_substitution subFreeRAMVarSub PROGMEM = {subFreeRAMStr, evaluate_free_ram, NULL};

static const var_substitution * const substitutions[] PROGMEM = {
	&subMacAddrVarSub,
	&subIPAddressVarSub,
	&subNetmaskVarSub,
	&subGatewayVarSub,
	&subNetConfSrcVarSub,
	&subWebbinoVerVarSub,
	&subUptimeVarSub,
	&subFreeRAMVarSub,
	NULL
};


/******************************************************************************
 * MAIN STUFF                                                                 *
 ******************************************************************************/

// Avoid some bug reports :)
#ifndef WEBBINO_ENABLE_SD
#error Please enable WEBBINO_ENABLE_SD in webbino_common.h
#endif

void setup () {
	Serial.begin (9600);
	// Serial.println (F("Webbino " PROGRAM_VERSION));

	Serial.print (F("Initializing SD card..."));
	if (!SD.begin (SD_SS)) {
		Serial.println (F(" failed"));
		while (42)
			;
	}
	Serial.println (F(" done"));

#if defined (WEBBINO_USE_ENC28J60) || defined (WEBBINO_USE_WIZ5100)
	byte mac[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
	netint.begin (mac);
#elif defined (WEBBINO_USE_ESP8266)
	swSerial.begin (9600);
	netint.begin (swSerial, WIFI_SSID, WIFI_PASSWORD);
#endif

	Serial.println (F("Trying to get an IP address through DHCP"));
	if (!webserver.begin (netint, NULL, substitutions)) {
		Serial.println (F("Failed to get configuration from DHCP"));
		while (42)
			;
	} else {
		Serial.println (F("DHCP configuration done"));
#if 0
		ether.printIp ("IP:\t", webserver.getIP ());
		ether.printIp ("Mask:\t", webserver.getNetmask ());
		ether.printIp ("GW:\t", webserver.getGateway ());
		Serial.println ();
#endif
	}
}

void loop () {
	webserver.loop ();
}
