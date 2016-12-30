/*
	Multiple exploit fixes
	Main.cpp
	by Dutchmeat
*/



#include "version.h"
#include <q_shared.h>
#include <g_local.h>
#include <qmmapi.h>
#include <string.h>
#include "main.h"
#include "util.h"
#include <time.h>
#include <string>
using std::string;

pluginres_t* g_result = NULL;
plugininfo_t g_plugininfo = {
	DUTCHFIX_QMM_PNAME,				//name of plugin
	DUTCHFIX_QMM_VERSION,			//version of plugin
	DUTCHFIX_QMM_PDESC,		//description of plugin
	DUTCHFIX_QMM_BUILDER,			//author of plugin
	DUTCHFIX_QMM_WEBSITE,			//website of plugin
	0,					//can this plugin be paused?
	0,					//can this plugin be loaded via cmd
	1,					//can this plugin be unloaded via cmd
	QMM_PIFV_MAJOR,				//plugin interface version major
	QMM_PIFV_MINOR				//plugin interface version minor
};
eng_syscall_t g_syscall = NULL;
mod_vmMain_t g_vmMain = NULL;
pluginfuncs_t* g_pluginfuncs = NULL;

playerinfo_t g_playerinfo[MAX_CLIENTS];	//store qadmin-specific user info

int g_vmbase = 0;

gentity_t* g_gents = NULL;
int g_gentsize = sizeof(gentity_t);
gclient_t* g_clients = NULL;
int g_clientsize = sizeof(gclient_t);

int ab_enabled = 0;

int levelTime;
int startTime;

C_DLLEXPORT void QMM_Query(plugininfo_t** pinfo) {
	QMM_GIVE_PINFO();
}

C_DLLEXPORT int QMM_Attach(eng_syscall_t engfunc, mod_vmMain_t modfunc, pluginres_t* presult, pluginfuncs_t* pluginfuncs, int vmbase, int iscmd) {
	QMM_SAVE_VARS();

	memset(g_playerinfo, 0, sizeof(g_playerinfo));

	iscmd = 0;

	return 1;
}

C_DLLEXPORT void QMM_Detach(int iscmd) {
	iscmd = 0;
}

C_DLLEXPORT int QMM_vmMain(int cmd, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11) {
	int timeLimit2;
	time_t rawtime;
	struct tm * timeinfo;
	char buffer [80];

	if (cmd == GAME_INIT) {
		//init msg
		g_syscall(G_PRINT, "[DutchFix 1.6] by Dutchmeat is loaded\n");

		//register cvars
		g_syscall(G_CVAR_REGISTER, NULL, "dutchfix", "1.6", CVAR_ROM | CVAR_SERVERINFO);
		g_syscall(G_CVAR_SET, "dutchfix", "1.6");
		g_syscall(G_CVAR_REGISTER, NULL, "dutchfix_autoban", "1", CVAR_SERVERINFO | CVAR_ARCHIVE);

		ab_enabled = QMM_GETINTCVAR("dutchfix_autoban");
		//let's record the leveltime like a normal mod
		levelTime = arg0;
		startTime = levelTime;

	}

	if (cmd == GAME_RUN_FRAME){
		//let's record the leveltime like a normal mod
		levelTime = arg0;
	}

	if (cmd == GAME_CLIENT_COMMAND) {
		//check what command it is
		char tempbuf[128];
		char tempbuf2[128];
		char tempbuf3[128];
		int client;

		client = arg0;

		g_syscall(G_ARGV, 0, tempbuf, sizeof(tempbuf));

		if (Q_stricmpn(tempbuf, "ws", 2) == 0) {
			g_syscall(G_ARGV, 1, tempbuf, sizeof(tempbuf)); //reused the variable
			
			if (tempbuf == NULL) {
				g_syscall(G_SEND_SERVER_COMMAND, -1, QMM_VARARGS("chat \"^7ignoring tempbuff, no arg'%d'\"",arg1));
				QMM_RET_SUPERCEDE(1);
			}

			int arg1 = getIntFromString(tempbuf);

			//okay a mistake can be made
			if (arg1 == NULL) {
				QMM_RET_SUPERCEDE(1);
			}

			//but this is just wrong
			if (arg1 < 0 || arg1 > 22) {
				g_syscall(G_DROP_CLIENT, client, "^7You have been kicked due using the ^1WS exploit^7");
				g_syscall(G_SEND_SERVER_COMMAND, -1, QMM_VARARGS("chat \"^7%s is kicked due using the ^1WS exploit^7!\"",g_playerinfo[client].name ));
				QMM_RET_SUPERCEDE(1);
			}
	
		}
		//====================================================================
		// TEAM COMMAND CHECK;
		// When argument 2 and 3 are above 63, remove the player and log it.
		//====================================================================
		if (Q_stricmpn(tempbuf, "team", 4) == 0) {
			g_syscall(G_ARGV, 1, tempbuf, sizeof(tempbuf));
			g_syscall(G_ARGV, 2, tempbuf2, sizeof(tempbuf2));
			g_syscall(G_ARGV, 3, tempbuf3, sizeof(tempbuf3));
			
			//nade bug
			if (atoi(tempbuf3) == 9){
				g_syscall(G_DROP_CLIENT, client, "^7You have been kicked due using the ^1Nade exploit^7");
				g_syscall(G_SEND_SERVER_COMMAND, -1, QMM_VARARGS("chat \"^7%s is kicked due using the ^1Nade exploit^7!\"",g_playerinfo[client].name ));

				//Block the actual command from reaching the game mod.
				QMM_RET_SUPERCEDE(1);
			}

			//team nuke
			if(atoi(tempbuf2) > 63 || atoi(tempbuf3) > 63){

				if (ab_enabled){
					g_syscall(G_SEND_CONSOLE_COMMAND, EXEC_APPEND, QMM_VARARGS("PB_SV_BanGuid \"%s\" \"%s\" \"%s\" \"caught nuking by DutchFix\"\n", g_playerinfo[client].guid, g_playerinfo[client].name, g_playerinfo[client].ip));
					g_syscall(G_DROP_CLIENT, client, "^7You have been banned due ^1NUKING^7");
					g_syscall(G_SEND_SERVER_COMMAND, -1, QMM_VARARGS("chat \"^7%s is banned due ^1NUKING^7!\"",g_playerinfo[client].name ));
				}else{
					g_syscall(G_SEND_SERVER_COMMAND, -1, QMM_VARARGS("chat \"^7%s is kicked due ^1NUKING^7!\"",g_playerinfo[client].name ));
					g_syscall(G_DROP_CLIENT, client, "^7You have been kicked due ^1NUKING^7");
				}

				time ( &rawtime );
				timeinfo = localtime ( &rawtime );

				strftime (buffer,80,"%c",timeinfo);

				LogPrint(QMM_VARARGS("[DutchFix] %s '%s' nuke detected cnum(%d) ip(%s) guid(%s) name(%s)\n",buffer,"teamcommand", client, g_playerinfo[client].ip, g_playerinfo[client].guid, g_playerinfo[client].name));

				//Block the actual command from reaching the game mod.
				QMM_RET_SUPERCEDE(1);
			}
		}

		//===================================================
		// CALLVOTE CHECK; 
		// You can't call a vote when there's only 
		// 20 seconds before the game goes into intermission.
		// We don't remove the player, but we do log it, 
		// to see if a certain player repeats this behaviour
		// TCE only
		//===================================================
		if (Q_stricmpn(tempbuf, "callvote", 8) == 0) {
			time ( &rawtime );
			timeinfo = localtime ( &rawtime );

			strftime (buffer,80,"%c",timeinfo);

			//you can't call a vote if 20 seconds before intermission
			timeLimit2 = QMM_GETINTCVAR("timelimit") * 60000;

			if (!timeLimit2)
				QMM_RET_IGNORED(1);

			if ((levelTime - startTime) >= timeLimit2 - 20000){
				LogPrint(QMM_VARARGS("[DutchFix] %s '%s' exploit detected cnum(%d) ip(%s) guid(%s) name(%s)\n", buffer, "Callvote",client, g_playerinfo[client].ip, g_playerinfo[client].guid, g_playerinfo[client].stripname));
				
				//Block the actual command from reaching the game mod.
				QMM_RET_SUPERCEDE(1);
			}
		}
	}

	//handle the game initialization (dependent on mod being loaded)	
	if (cmd == GAME_CLIENT_CONNECT || cmd == GAME_CLIENT_USERINFO_CHANGED) {

		char userinfo[MAX_INFO_STRING];
		g_syscall(G_GET_USERINFO, arg0, userinfo, sizeof(userinfo));
		
		if (!Info_Validate(userinfo)){
			QMM_RET_IGNORED(1);
		}

		strncpy(g_playerinfo[arg0].name, Info_ValueForKey(userinfo, "name"), sizeof(g_playerinfo[arg0].name));
   		strncpy(g_playerinfo[arg0].stripname, StripCodes(g_playerinfo[arg0].name), sizeof(g_playerinfo[arg0].stripname));

		//You must be thinking, why a second check? 
		//Well we want this only to happen on connect,
		//but things like getting the name will be retrieved on connect and changed.
		if (cmd == GAME_CLIENT_CONNECT) {
			g_playerinfo[arg0].connected = 1;

			strncpy(g_playerinfo[arg0].ip, Info_ValueForKey(userinfo, "ip"), sizeof(g_playerinfo[arg0].ip));
			//if a situation arises where the ip is exactly 15 bytes long, the 16th byte
			//in the buffer will be ':', so this will terminate the string anyway
			char* temp = strstr(g_playerinfo[arg0].ip, ":");
			if (temp) *temp = '\0';

			strncpy(g_playerinfo[arg0].guid, Info_ValueForKey(userinfo, "cl_guid"), sizeof(g_playerinfo[arg0].guid));
		}
	}

	QMM_RET_IGNORED(1);
}

C_DLLEXPORT int QMM_syscall(int cmd, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11, int arg12) {
	if (cmd == G_SEND_SERVER_COMMAND) {
		if (arg1 && strlen((char*)arg1) > 1022) {
				time_t rawtime;

				struct tm * timeinfo;
				char buffer [80];

				if (ab_enabled){
					g_syscall(G_SEND_CONSOLE_COMMAND, EXEC_APPEND, QMM_VARARGS("PB_SV_BanGuid \"%s\" \"%s\" \"%s\" \"caught nuking by DutchFix\"\n", g_playerinfo[arg0].guid, g_playerinfo[arg0].name, g_playerinfo[arg0].ip));
					g_syscall(G_SEND_SERVER_COMMAND, -1, QMM_VARARGS("chat \"^7%s is banned due ^1NUKING^7!\"",g_playerinfo[arg0].name ));
				}else{
					g_syscall(G_SEND_SERVER_COMMAND, -1, QMM_VARARGS("chat \"^7%s is kicked due ^1NUKING^7!\"",g_playerinfo[arg0].name ));
					g_syscall(G_DROP_CLIENT, arg0, "^7You have been kicked due ^1NUKING^7");
				}

				time ( &rawtime );
				timeinfo = localtime ( &rawtime );

				strftime (buffer,80,"%c",timeinfo);

				LogPrint(QMM_VARARGS("[DutchFix] %s '%s' nuke detected cnum(%d) ip(%s) guid(%s) name(%s)\n",buffer,"MsgBoom", arg0, g_playerinfo[arg0].ip, g_playerinfo[arg0].guid, g_playerinfo[arg0].stripname));

				//Block the syscall from reaching the engine.
				QMM_RET_SUPERCEDE(1);
		}

	}
	
	QMM_RET_IGNORED(1);
}

C_DLLEXPORT int QMM_vmMain_Post(int cmd, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11) {
		//handle the game initialization (dependent on mod being loaded)	
	if (cmd == GAME_CLIENT_CONNECT || cmd == GAME_CLIENT_USERINFO_CHANGED) {

		char userinfo[MAX_INFO_STRING];
		g_syscall(G_GET_USERINFO, arg0, userinfo, sizeof(userinfo));

		if (!Info_Validate(userinfo)){
			QMM_RET_IGNORED(1);
		}

		strncpy(g_playerinfo[arg0].name, Info_ValueForKey(userinfo, "name"), sizeof(g_playerinfo[arg0].name));
   		strncpy(g_playerinfo[arg0].stripname, StripCodes(g_playerinfo[arg0].name), sizeof(g_playerinfo[arg0].stripname));

		//You must be thinking, why a second check? 
		//Well we want this only to happen on connect,
		//but things like getting the name will be retrieved on connect and changed.
		if (cmd == GAME_CLIENT_CONNECT) {

			g_playerinfo[arg0].connected = 1;

			strncpy(g_playerinfo[arg0].ip, Info_ValueForKey(userinfo, "ip"), sizeof(g_playerinfo[arg0].ip));
			//if a situation arises where the ip is exactly 15 bytes long, the 16th byte
			//in the buffer will be ':', so this will terminate the string anyway
			char* temp = strstr(g_playerinfo[arg0].ip, ":");
			if (temp) *temp = '\0';

			strncpy(g_playerinfo[arg0].guid, Info_ValueForKey(userinfo, "cl_guid"), sizeof(g_playerinfo[arg0].guid));

		}
	}

	QMM_RET_IGNORED(1);

}

C_DLLEXPORT int QMM_syscall_Post(int cmd, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11, int arg12) {
	QMM_RET_IGNORED(1);
}
