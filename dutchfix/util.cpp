/*

QADMIN_QMM - QMM Administration Plugin
Copyright QMM Team 2004
http://www.quake3mm.net/

Licensing:
    QMM is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    QMM is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with QMM; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Created By:
    Kevin Masterson a.k.a. CyberMind <cybermind@users.sourceforge.net>


  Dutchmeat: For now we only need a few (string)functions, gonna straight that out later.
*/

/* $Id: util.cpp,v 1.9 2006/03/08 08:25:01 cybermind Exp $ */

#include "version.h"
#include <q_shared.h>
#include <g_local.h>
#include <qmmapi.h>
#include <stdarg.h>
#include <stdio.h>
#include "main.h"
#include "util.h"

#ifdef WIN32
 #ifndef vsnprintf
  #define vsnprintf _vsnprintf
 #endif
#endif

#define Q_IsColorString(p)	( p && *(p) == Q_COLOR_ESCAPE && *((p)+1) && *((p)+1) != Q_COLOR_ESCAPE )

char *Q_CleanStr( char *string ) {
	char*	d;
	char*	s;
	int		c;

	s = string;
	d = string;
	while ((c = *s) != 0 ) {
		if ( Q_IsColorString( s ) ) {
			s++;
		}		
		else if ( c >= 0x20 && c <= 0x7E ) {
			*d++ = c;
		}
		s++;
	}
	*d = '\0';

	return string;
}

//returns the first slot id with given ip starting after the slot id specified in 'start_after'
int user_with_ip(const char* ip, int start_after) {
	for (int i = start_after + 1; i < MAX_CLIENTS; ++i) {
		if (!strcmp(ip, g_playerinfo[i].ip))
			return i;
	}

	return -1;
}

int Q_stricmpn (const char *s1, const char *s2, int n) {
	int		c1, c2;
	
	do {
		c1 = *s1++;
		c2 = *s2++;

		if (!n--) {
			return 0;		// strings are equal until end point
		}
		
		if (c1 != c2) {
			if (c1 >= 'a' && c1 <= 'z') {
				c1 -= ('a' - 'A');
			}
			if (c2 >= 'a' && c2 <= 'z') {
				c2 -= ('a' - 'A');
			}
			if (c1 != c2) {
				return c1 < c2 ? -1 : 1;
			}
		}
	} while (c1);
	
	return 0;		// strings are equal
}

int Q_stricmp (const char *s1, const char *s2) {
	return (s1 && s2) ? Q_stricmpn (s1, s2, 99999) : -1;
}


void ClientPrint(int clientnum, const char* msg, bool chat) {
	//remove any QMM_VARARGS() conflicts by making a temp string
	static char temp[MAX_STRING_LENGTH];
	strncpy(temp, msg, MAX_STRING_LENGTH);
	msg = (const char*)temp;

	if (clientnum == -2) //SERVER_CONSOLE
		g_syscall(G_PRINT, msg);
	else {
		if (chat)
			g_syscall(G_SEND_SERVER_COMMAND, clientnum, QMM_VARARGS("chat \"%s\"", msg));
		else
			g_syscall(G_SEND_SERVER_COMMAND, clientnum, QMM_VARARGS("print \"%s\"", msg));
	}
}

char* concatargs(int min) {
	static char text[MAX_DATA_LENGTH];
	char arg[MAX_DATA_LENGTH];
	int max = g_syscall(G_ARGC);
	int x = 1;
	text[0] = '\0';
	for (int i = min; i < max; ++i) {
		g_syscall(G_ARGV, i, arg, sizeof(arg));
		strncat(text, arg, sizeof(text) - x);
		x += strlen(arg);
		strncat(text, " ", sizeof(text) - x);
		++x;
	}
	
	text[sizeof(text)-1] = '\0';

	return text;
}

bool is_valid_map(const char* map) {
	fileHandle_t fmap;
	int mapsize = g_syscall(G_FS_FOPEN_FILE, QMM_VARARGS("maps\\%s", map), &fmap, FS_READ);
	g_syscall(G_FS_FCLOSE_FILE, fmap);
	return mapsize ? 1 : 0;
}

char** tok_parse(const char* str, char split) {
	if (!str || !*str)
		return NULL;

	int i, index;
	//toks is 1 to allocate the NULL terminating pointer
	int toks = 1, slen = strlen(str);
	char* copy = (char*)malloc(slen + 1);
	char* tokstart = copy;
	memcpy(copy, str, slen + 1);

	for (i = 0; i <= slen; ++i) {
		if (copy[i] == split || !copy[i])
			++toks;
	}

	char** arr = (char**)malloc(sizeof(char*) * toks);
	for (i = 0, index = 0; i <= slen; ++i) {
		if (copy[i] == split || !copy[i]) {
			arr[index++] = tokstart;
			tokstart = &copy[i+1];
			copy[i] = '\0';
		}
	}
	arr[index] = NULL;

	return arr;	
}

void tok_free(char** arr) {
	if (arr) {
		if (arr[0])
			free((void*)(arr[0]));
		free((void*)arr);
	}
}

void setcvar(char* cvar, int datanum) {
	char value[MAX_DATA_LENGTH];
	g_syscall(G_ARGV, datanum, value, sizeof(value));
	g_syscall(G_CVAR_SET, cvar, value);
}

const char* StripCodes(const char* name) {
	static char temp[MAX_NETNAME];

	int slen = strlen(name);
	if (slen >= MAX_NETNAME)
		slen = MAX_NETNAME - 1;

	for (int i = 0, j = 0; i < slen; ++i) {
		if (name[i] == Q_COLOR_ESCAPE) {
			if (name[i+1] != Q_COLOR_ESCAPE)
				++i;
			continue;
		}
		temp[j++] = name[i];
	}

	return temp;
}

//cycling array of buffers
const char* lcase(const char* string) {
	static char buf[8][1024];
	static int index = 0;
	int i = index;

	int j = 0;
	for (j = 0; string[j]; ++j) {
		buf[i][j] = tolower(string[j]);
	}
	buf[i][j] = '\0';

	index = (index + 1) & 7;
	return buf[i];
}

//returns index of matching name
//-1 when ambiguous or not found
int namematch(const char* string, bool ret_first, int start_after) {
	char lstring[MAX_NAME_LENGTH];
	//copied so the lcase-local static buffer is not overwritten in the big loop below
	strncpy(lstring, lcase(string), sizeof(lstring));

	const char* lfullname = NULL;
	const char* lstripname = NULL;

	int matchid = -1;

	for (int i = start_after + 1; i < MAX_CLIENTS; ++i) {
		if (!g_playerinfo[i].connected)
			continue;

		//on a complete match, return
		if (!strcmp(string, g_playerinfo[i].name))
			return i;

		//try partial matches, if we have 2, cancel
		lfullname = lcase(g_playerinfo[i].name);
		lstripname = lcase(g_playerinfo[i].stripname);

		if (strstr(lfullname, lstring) || strstr(lstripname, lstring)) {
			if (ret_first)
				return i;

			if (matchid == -1)
				matchid = i;
			else
				return -1;
		}
	}
	
	return matchid;
}

//from Q3SDK, thanks id :)
char* Info_ValueForKey(const char* s, const char* key) {
	char pkey[BIG_INFO_KEY];
	static char value[2][BIG_INFO_VALUE];	// use two buffers so compares
						// work without stomping on each other
	static int valueindex = 0;
	char *o;
	
	if (!s || !key)
		return "";

	if (strlen(s) >= BIG_INFO_STRING) {
		g_syscall(G_PRINT, "[QADMIN] ERROR: Info_ValueForKey: oversize infostring\n");
		return "";
	}

	valueindex ^= 1;
	if (*s == '\\')
		s++;
	while (1)
	{
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return "";
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value[valueindex];

		while (*s != '\\' && *s)
		{
			*o++ = *s++;
		}
		*o = 0;

		if (!strcasecmp(key, pkey))
			return value[valueindex];

		if (!*s)
			break;
		s++;
	}

	return "";
}
qboolean Info_Validate(const char* s) {
	return (strchr(s, '\"') || strchr(s, ';')) ? qfalse : qtrue;
}


/*
=============
Q_strncpyz
 
Safe strncpy that ensures a trailing zero
=============
*/
void Q_strncpyz( char *dest, const char *src, int destsize ) {
	if ( !src ) {
		g_syscall(G_PRINT,"[DutchFix] Q_strncpyz: NULL src\n");
		return;
	}
	if ( destsize < 1 ) {
		g_syscall(G_PRINT,"[DutchFix] Q_strncpyz: destsize < 1\n");
		return;
	}

	strncpy( dest, src, destsize-1 );
    dest[destsize-1] = 0;
}


//dutchy
int getIntFromString (const char *string) {
	for (int i = 0; i < strlen(string) - 1; i++) {
		if (!isdigit(string[i]))
		{
			return NULL;
		}
	}
	return atoi(string);
}
void LogPrint(const char *s) {

	fileHandle_t f;

	// open the file
	if (g_syscall(G_FS_FOPEN_FILE, "dutchfix.log", &f, FS_APPEND) < 0){
		g_syscall(G_PRINT,"[DutchFix]Could not open/ or create dutchfix.log, make sure you have writing rights in the homefolder of WOLFET.\n");
		return;
	}

	g_syscall(G_FS_WRITE, s, strlen(s), f );
	g_syscall(G_FS_FCLOSE_FILE, f);
	g_syscall(G_PRINT,QMM_VARARGS("%s\n",s));

	return; //not used yet.
}
