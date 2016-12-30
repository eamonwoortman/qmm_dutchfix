#define MAX_STRING_LENGTH 1024
#define MAX_DATA_LENGTH 200
#define MAX_COMMAND_LENGTH 64
#define MAX_NUMBER_LENGTH 11

#define MAX_USER_ENTRIES 64

//MAX_NETNAME = (35 + 1)
#define MAX_USER_LENGTH MAX_NETNAME
#define MAX_GUID_LENGTH 32 + 1
#define MAX_PASS_LENGTH 32 + 1
#define MAX_IP_LENGTH 15 + 1
#define MAX_CLIENTS 64

#ifdef WIN32
 #define strcasecmp stricmp
#endif


typedef enum addusertype_e {
	au_ip = 1,
	au_name = 2,
	au_id = 3
} addusertype_t;

typedef struct playerinfo_s {
	char guid[MAX_GUID_LENGTH];
	char ip[MAX_IP_LENGTH];
	char name[MAX_NETNAME];
	//std::string stripname;
	char stripname[MAX_NETNAME];
	int access;
	bool authed;
	bool gagged;
	bool connected;
} playerinfo_t;
extern playerinfo_t g_playerinfo[];

typedef struct userinfo_s {
	char user[MAX_USER_LENGTH];
	char pass[MAX_PASS_LENGTH];
	int access;
	addusertype_t type;
} userinfo_t;
#define usertype(x) (x==au_ip?"IP":(x==au_name?"name":"ID"))

extern userinfo_t g_userinfo[];
extern int g_maxuserinfo;

