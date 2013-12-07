/*

* author		Wang Yang
* E-mail		wysaid@gmail.com OR admin@wysaid.org
* blog			http://blog.wysaid.org
* date			2012-7-28
*/

#ifndef _ONLINE_MODE_H_
#define _ONLINE_MODE_H_

#include "wysaid.h"

#pragma comment(lib, "wsock32.lib")

#define DF_SERVERPORT	8900
#define DF_SERVERADDR	MAKEIPADDRESS(127,0,0,1)

#define BACKLOG			10
#define BUFFERSIZE		1024
#define SIZE_DIRECTIVE	((int)sizeof(Directive))

#define RET_COMP_SE(val)	do { if((val) == SOCKET_ERROR) return false; } while(0)
#define BZERO(addr, size)		memset((addr), 0, (size));

enum Directive { OL_START, OL_MESSAGE, OL_WORD, OL_WIN, OL_LOSE };

using std::string;

/*
  A class for both server and client that is used to contact other players.
  It seems a little bad, I think. 
  I will do it better when I have more experience of network coding in the future.
					By wysaid.
*/

class NetControl
{
	friend class Word;
public:
	NetControl();

	bool initNetwork(HWND, HWND);

	void initPool();  //Used when you're a server
	void shutdownNetwork();

	bool createHost(unsigned short); //Used when you're a server
	bool connectHost(DWORD, WORD); //Used when you're a client

	int waitForClient(clock_t); //Used when you're a server
	int waitForServer(LPSTR); // Used when you're a client

//	void startServer();			// Used before 'waitForMsgs' for server.
//	int waitForMsgs(clock_t);	// Used when you're a server
	void statusSync();			// Used when you're a server

	void serverSync(); // Used when you're a server
	bool clientSync(); // Used when you're a client

	void dealMsgs();  //Used when you're a server
	void sendMsg(LPCSTR, int);
	bool isServer() { return m_isServer; }
	bool isServerReady() { return m_isServer ? m_pool.readSet.fd_count > 1 : false; }
	void setServer(bool bSet) { m_isServer = bSet; }
	void addServerSyncWord(Word* word) { if(word != NULL) m_wordToClient.push_back(word); }

	void showStaus();
	Word* getServerSyncWord(); //It'll return the first word of the 'm_wordToClient' and delete it

	LPSTR getMyName() { return m_myName; }
	void setStartTitle();

	void lock(DWORD);
	void unLock();

	~NetControl();
private:
	string m_status;
	char m_myName[128];
	DWORD m_myAddress;
	SOCKET m_listenfd, m_serverfd;

	struct User
	{
		char name[128];
		SOCKET sockfd;
		IN_ADDR addr;
		unsigned short port;
	} m_users[BACKLOG];

	User* findUserBySocket(SOCKET);

	struct PoolFds
	{
		SOCKET maxfd;
		FD_SET readSet;
		FD_SET readySet;
		int nReady;
	} m_pool;

	std::vector<Word*> m_wordToClient;
	CRITICAL_SECTION m_cs;
	bool m_isServer, m_bReady;
};

INT_PTR CALLBACK Connect(HWND, UINT, WPARAM, LPARAM);

void clientFrameSync(void*);
void serverFrameSync(void*);

void wait4Client(void*);
void wait4Server(void*);

void addText(LPCSTR);

bool onlineClientFrameFunc();
bool onlineServerFrameFunc();
bool onlineRenderFunc();

void onlineWin();
void onlineLose();

#endif