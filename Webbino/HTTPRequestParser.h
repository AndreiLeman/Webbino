/***************************************************************************
 *   This file is part of SmartStrip.                                      *
 *                                                                         *
 *   Copyright (C) 2012 by SukkoPera                                       *
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

#ifndef _HTTPREQUESTPARSER_H_
#define _HTTPREQUESTPARSER_H_

#include <Arduino.h>

// Maximum length of a get parameter value
#define BUF_LEN 32

#define MAX_URL_LEN 128

//#define VERBOSE_REQUEST_PARSER


class HTTPRequestParser {
private:
	char *request;
	char buffer[BUF_LEN];
	
	void extract_url ();
	
public:
	char url[MAX_URL_LEN];
	
	HTTPRequestParser (char *req);
	
	// ~HTTPRequestParser ();
	
	char *get_basename ();

	char *get_get_parameter (const char param[]);

	char *get_get_parameter (const __FlashStringHelper *param);
};	

#endif
