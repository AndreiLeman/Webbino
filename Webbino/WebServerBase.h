/***************************************************************************
 *   This file is part of SmartStrip.                                      *
 *                                                                         *
 *   Copyright (C) 2012-2015 by SukkoPera                                  *
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

#ifndef _WEBSERVER_H_
#define _WEBSERVER_H_

#include "webbino_common.h"
#include "NetworkInterface.h"
#include "HTTPRequestParser.h"

class WebClientBase;


typedef void (*PageFunction) (HTTPRequestParser& request);

struct Page {
	PGM_P name;
	PGM_P content;
	PageFunction function;

	// Methods that (try to) hide the complexity of accessing PROGMEM data
	PGM_P getName () {
		return reinterpret_cast<PGM_P> (pgm_read_word (&(this -> name)));
	}

	PGM_P getContent () {
		return reinterpret_cast<PGM_P> (pgm_read_word (&(this -> content)));
	}

	PageFunction getFunction () {
		return reinterpret_cast<PageFunction> (pgm_read_word (&(this -> function)));
	}
};


#ifdef ENABLE_TAGS

// Maximum length of a substitution tag
#define MAX_TAG_LEN 24

// Maximum length of the string generated by a substitution
#define MAX_SUBSTITUTION_LEN 32

typedef char *(*var_evaluate_func) (void *data);

struct var_substitution {
	PGM_P name;				// Max length: MAX_TAG_LEN
	var_evaluate_func function;
	void *data;

		// Methods that (try to) hide the complexity of accessing PROGMEM data
	PGM_P getName () {
		return reinterpret_cast<PGM_P> (pgm_read_word (&(this -> name)));
	}

	var_evaluate_func getFunction () {
		return reinterpret_cast<var_evaluate_func> (pgm_read_word (&(this -> function)));
	}

	void *getData () {
		return reinterpret_cast<void *> (pgm_read_word (&(this -> data)));
	}
};

#endif


class WebServer {
private:
	NetworkInterface* netint;

	const Page * const *pages;

#ifdef ENABLE_TAGS
	const var_substitution * const *substitutions;
#endif

	void sendPage (WebClientBase *client);

	Page *get_page (const char *name);

#ifdef ENABLE_TAGS
	char *findSubstitutionTag (char *tag);

	char *findSubstitutionTagGetParameter (HTTPRequestParser& request, const char *tag);
#endif

public:
	void setPages (const Page * const _pages[]);

#ifdef ENABLE_TAGS
	void setSubstitutions (const var_substitution * const _substitutions[]);
#endif

	bool begin (NetworkInterface& netint);

	bool loop ();
};

#endif
