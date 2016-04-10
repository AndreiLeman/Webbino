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

#ifndef _INTERFACE_H_INCLUDED
#define _INTERFACE_H_INCLUDED

#include <Arduino.h>
#include "WebClient.h"


class NetworkInterface {
public:
	virtual WebClient* processPacket () = 0;

	virtual bool usingDHCP () = 0;

	virtual byte* getMAC () = 0;

	virtual byte* getIP () = 0;

	virtual byte* getNetmask () = 0;

	virtual byte* getGateway () = 0;
};

#endif
