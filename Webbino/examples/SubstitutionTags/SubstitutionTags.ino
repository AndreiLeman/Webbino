/***************************************************************************
 *   This file is part of Webbino.                                         *
 *                                                                         *
 *   Copyright (C) 2012-2016 by SukkoPera                                  *
 *                                                                         *
 *   Webbino is free software: you can redistribute it and/or modify       *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Webbino is distributed in the hope that it will be useful,            *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with Webbino. If not, see <http://www.gnu.org/licenses/>.       *
 ***************************************************************************/

#include <Webbino.h>
#include <avr/pgmspace.h>

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
 * DEFINITION OF PAGES                                                        *
 ******************************************************************************/

#include "html.h"

static const Page indexPage PROGMEM = {index_html_name, index_html, NULL};

static const Page * const pages[] PROGMEM = {
	&indexPage,
 	NULL
};


/******************************************************************************
 * DEFINITION OF TAGS                                                         *
 ******************************************************************************/

#define REP_BUFFER_LEN 32
static char replaceBuffer[REP_BUFFER_LEN];
PString subBuffer (replaceBuffer, REP_BUFFER_LEN);

static PString& evaluate_ip (void *data __attribute__ ((unused))) {
   subBuffer.print (netint.getIP ());

  return subBuffer;
}

static PString& evaluate_netmask (void *data __attribute__ ((unused))) {
  subBuffer.print (netint.getNetmask ());

  return subBuffer;
}

static PString& evaluate_gw (void *data __attribute__ ((unused))) {
  subBuffer.print (netint.getGateway ());

  return subBuffer;
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
	subBuffer.print (WEBBINO_VERSION);

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

  if (d > 0) {
    subBuffer.print (d);
    subBuffer.print (d == 1 ? F(" day, ") : F(" days, "));
  }

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

static const ReplacementTag subMacAddrVarSub PROGMEM = {subMacAddrStr, evaluate_mac_addr, NULL};
static const ReplacementTag subIPAddressVarSub PROGMEM = {subIPAddressStr, evaluate_ip, NULL};
static const ReplacementTag subNetmaskVarSub PROGMEM = {subNetmaskStr, evaluate_netmask, NULL};
static const ReplacementTag subGatewayVarSub PROGMEM = {subGatewayStr, evaluate_gw, NULL};
static const ReplacementTag subNetConfSrcVarSub PROGMEM = {subNetConfSrcStr, evaluate_ip_src, NULL};
static const ReplacementTag subWebbinoVerVarSub PROGMEM = {subWebbinoVerStr, evaluate_webbino_version, NULL};
static const ReplacementTag subUptimeVarSub PROGMEM = {subUptimeStr, evaluate_uptime, NULL};
static const ReplacementTag subFreeRAMVarSub PROGMEM = {subFreeRAMStr, evaluate_free_ram, NULL};

static const ReplacementTag* const substitutions[] PROGMEM = {
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

void setup () {
	Serial.begin (9600);
	Serial.println (F("Webbino " WEBBINO_VERSION));

	Serial.println (F("Trying to get an IP address through DHCP"));
#if defined (WEBBINO_USE_ENC28J60) || defined (WEBBINO_USE_WIZ5100)
	byte mac[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
	bool ok = netint.begin (mac);
#elif defined (WEBBINO_USE_ESP8266)
	swSerial.begin (9600);
	bool ok = netint.begin (swSerial, WIFI_SSID, WIFI_PASSWORD);
#endif

	if (!ok) {
		Serial.println (F("Failed to get configuration from DHCP"));
		while (42)
			;
	} else {
		Serial.println (F("DHCP configuration done:"));
		Serial.print (F("- IP: "));
		Serial.println (netint.getIP ());
		Serial.print (F("- Netmask: "));
		Serial.println (netint.getNetmask ());
		Serial.print (F("- Default Gateway: "));
		Serial.println (netint.getGateway ());

		webserver.begin (netint, pages, substitutions);
	}
}

void loop () {
	webserver.loop ();
}
