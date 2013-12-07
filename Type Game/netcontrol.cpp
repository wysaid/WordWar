/*

* author		Wang Yang
* E-mail		wysaid@gmail.com OR admin@wysaid.org
* blog			http://blog.wysaid.org
* date			2012-7-28
*/

#include "netcontrol.h"

#define _HIDE_WINDOW

NetControl g_netCtrl;
HWND g_hMsg, g_hDlg;
bool g_bRun = false;

INT_PTR CALLBACK Connect(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND s_hServerIP, s_hServerPort, s_hMyIP, s_hMsgBox, s_hMyPort,
		s_hBTNConnect, s_hBTNCreate, s_hBTNMsg;
	static bool s_bReady = false;
	static HANDLE s_thread;
	DWORD ip;
	WORD port;

	switch (message)
	{
	case WM_INITDIALOG:
#ifdef _HIDE_WINDOW
		ShowWindow(g_hwnd, SW_HIDE);
#endif
		RECT rt;
		GetWindowRect(hDlg, &rt);
		rt.right -= rt.left;
		rt.bottom -= rt.top;
		MoveWindow(hDlg, (GetSystemMetrics(SM_CXSCREEN) - rt.right) / 2 ,
			(GetSystemMetrics(SM_CYSCREEN) - rt.bottom) / 2, rt.right, rt.bottom, FALSE);
		g_hDlg = hDlg;
		g_hMsg = GetDlgItem(hDlg, IDC_OLINFO);
		s_hServerIP = GetDlgItem(hDlg, IDC_OLSERVERADDRESS);
		s_hServerPort = GetDlgItem(hDlg, IDC_OLSERVERPORT);
		s_hMyIP = GetDlgItem(hDlg, IDC_OLMYADDRESS);
		s_hMyPort = GetDlgItem(hDlg, IDC_OLMYPORT);
		s_hBTNConnect = GetDlgItem(hDlg, IDC_OLBTNCONNECT);
		s_hBTNCreate = GetDlgItem(hDlg, IDC_OLBTNCREATE);
		s_hMsgBox = GetDlgItem(hDlg, IDC_OLMSG);
		s_hBTNMsg = GetDlgItem(hDlg, IDC_OLBTNMSG);
		SendMessage(GetDlgItem(hDlg, IDC_OLCONNECTSERVER), BM_SETCHECK, 1, 0);
		SendMessage(hDlg, WM_COMMAND, IDC_OLCONNECTSERVER, 0);
		SendMessage(s_hServerIP, IPM_SETADDRESS, 0, DF_SERVERADDR);
		EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
		if(!g_netCtrl.initNetwork(hDlg, s_hMyIP))
		{
			MessageBoxA(hDlg, "请检查是否禁用网络", "打开网络失败", MB_ICONERROR | MB_OK);
			EndDialog(hDlg, 0);
		}
		SetDlgItemInt(hDlg, IDC_OLMYPORT, DF_SERVERPORT, FALSE);
		SetDlgItemInt(hDlg, IDC_OLSERVERPORT, DF_SERVERPORT, FALSE);
		addText("欢迎使用联机模式! 请按界面提示操作!\r\n"
			"联机模式是主机与多个客户端之间的较量,\r\n"
			"当主机游戏失败或者所有客户机全部失败时，决出胜负╰(￣▽￣)╮"
			"\r\n\r\n\t\t祝您游戏愉快 -- By wysaid\r\n");
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_OLCONNECTSERVER:
			EnableWindow(s_hServerIP, TRUE);
			EnableWindow(s_hBTNCreate, FALSE);
			EnableWindow(s_hBTNConnect, TRUE);
			SendMessage(s_hServerPort, EM_SETREADONLY, FALSE, 0);
			SendMessage(s_hMyPort, EM_SETREADONLY, TRUE, 0);
			g_netCtrl.setServer(false);
			break;
		case IDC_OLCREATESERVER:
			EnableWindow(s_hServerIP, FALSE);
			EnableWindow(s_hBTNCreate, TRUE);
			EnableWindow(s_hBTNConnect, FALSE);
			SendMessage(s_hServerPort, EM_SETREADONLY, TRUE, 0);
			SendMessage(s_hMyPort, EM_SETREADONLY, FALSE, 0);
			g_netCtrl.setServer(true);
			break;

		case IDC_OLBTNMSG:
			{
				char szBuffer[160 + BUFFERSIZE], szText[BUFFERSIZE];
				int buflen, textlen;
				*(Directive*)szBuffer = OL_MESSAGE;
				textlen = GetDlgItemTextA(hDlg, IDC_OLMSG, szText, BUFFERSIZE);
				if(textlen == 0)
				{
					MessageBoxA(hDlg, "蛋疼了吗,少年?\r\n莫说空话!", "先去喝口茶?", MB_ICONWARNING | MB_OK);
					break;
				}
				buflen = sprintf(szBuffer + SIZE_DIRECTIVE, "\r\n%s: ", g_netCtrl.getMyName());
				szText[textlen] = 0;
				strcat(szBuffer + SIZE_DIRECTIVE, szText);
				g_netCtrl.sendMsg(szBuffer, buflen + textlen + SIZE_DIRECTIVE);
				if(g_netCtrl.isServer()) addText(szBuffer + SIZE_DIRECTIVE);
				SetDlgItemTextA(hDlg, IDC_OLMSG, "");
				SetFocus(s_hMsgBox);
			}
			break;
		case IDC_OLBTNCONNECT:
			addText("\r\n正在连接到指定主机，请耐心等候...");
			{
				SendMessage(s_hServerIP, IPM_GETADDRESS, NULL, (LPARAM)&ip);
				port = GetDlgItemInt(hDlg, IDC_OLSERVERPORT, NULL, FALSE);
				if(g_netCtrl.connectHost(ip, port))
				{
					s_bReady = true;
					EnableWindow(GetDlgItem(hDlg, IDC_OLCREATESERVER), FALSE);
					EnableWindow(GetDlgItem(hDlg, IDC_OLCONNECTSERVER), FALSE);
					EnableWindow(s_hBTNMsg, TRUE);
					EnableWindow(s_hBTNConnect, FALSE);
					EnableWindow(s_hServerIP, FALSE);
					SendMessage(s_hServerPort, EM_SETREADONLY, TRUE, 0);
					addText("\r\n连接主机成功, 正在等待主机启动游戏! 现在你可以和加入游戏的玩家聊聊天~ ^_^");
					s_thread = (HANDLE)_beginthread(wait4Server, 0, &s_bReady);
				}
				else
				{
					addText("\r\n连接主机失败...");
				}
			}
			break;
		case IDC_OLBTNCREATE:
			addText("\r\n正在创建主机...");
			{
				port = GetDlgItemInt(hDlg, IDC_OLMYPORT, NULL, FALSE);
				if(port < 100)
					MessageBoxA(hDlg, "请输入正确的主机端口号(默认: 8900)", "端口号小于100了", MB_ICONWARNING | MB_OK);
				else if(g_netCtrl.createHost(port))
				{
					s_bReady = true;
					EnableWindow(GetDlgItem(hDlg, IDC_OLCREATESERVER), FALSE);
					EnableWindow(GetDlgItem(hDlg, IDC_OLCONNECTSERVER), FALSE);
					EnableWindow(s_hBTNMsg, TRUE);
					SendMessage(s_hMyPort, EM_SETREADONLY, TRUE, 0);
					EnableWindow(s_hBTNCreate, FALSE);
					addText("\r\n主机创建成功! 正在等待用户的加入!");
					s_thread = (HANDLE)_beginthread(wait4Client, 0, &s_bReady);
				}
				else addText("\r\n网络初始化失败...");
			}
			break;
		case IDOK:
#ifdef _HIDE_WINDOW
			ShowWindow(g_hwnd, SW_SHOW);
#endif
			s_bReady = false;
			g_bRun = true;
			if(g_netCtrl.isServer())
			{
				Directive dir = OL_START;
				g_netCtrl.sendMsg((LPCSTR)&dir, SIZE_DIRECTIVE);
				g_hge->System_SetState(HGE_FRAMEFUNC,onlineServerFrameFunc);
				_beginthread(serverFrameSync, 0, NULL);
			}
			else 
			{
				g_hge->System_SetState(HGE_FRAMEFUNC,onlineClientFrameFunc);
				_beginthread(clientFrameSync, 0, NULL);
			}
			g_hge->System_SetState(HGE_RENDERFUNC,onlineRenderFunc);
			g_hge->System_SetState(HGE_DONTSUSPEND, true);
			
			EndDialog(hDlg, 1);
			return TRUE;
		case IDCANCEL:
#ifdef _HIDE_WINDOW
			ShowWindow(g_hwnd, SW_SHOW);
#endif
			s_bReady = false;
//			TerminateThread(s_thread, 0);
			g_netCtrl.shutdownNetwork();
			g_fnt1->SetScale(1.0f);
			EndDialog(hDlg, 0);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

NetControl::NetControl() : m_isServer(false)//, m_userNo(0)
{
	for(int i = 0; i != BACKLOG; ++i)
	{
		m_users[i].sockfd = SOCKET_ERROR;
		*m_users[i].name = 0;
	}
	InitializeCriticalSection(&m_cs);
}

NetControl::~NetControl()
{
	shutdownNetwork();
	DeleteCriticalSection(&m_cs);
}

bool NetControl::initNetwork(HWND hTitle, HWND hAddr)
{
	WSADATA wsadata;
	HOSTENT *pHost;
	char buffer[BUFFERSIZE];
	if(!WSAStartup(0x0202u, &wsadata) && gethostname(m_myName, 128))
		return false;
	pHost = gethostbyname(m_myName);
	m_myAddress = *(DWORD*)*pHost->h_addr_list;
	strcpy(m_myName, pHost->h_name);
	sprintf(buffer, "我的主机名: %s", m_myName);
	SetWindowTextA(hTitle, buffer);
	SetWindowTextA(hAddr, inet_ntoa(*(IN_ADDR*)*pHost->h_addr_list));
	return true;
}

void NetControl::shutdownNetwork()
{
	WSACleanup();
	for(vector<Word*>::iterator iter = m_wordToClient.begin(); iter != m_wordToClient.end(); ++iter)
		delete *iter;
	m_wordToClient.clear();
	m_status.clear();
	for(int i = 0; i != BACKLOG; ++i)
	{
		*m_users[i].name = 0;
		m_users[i].sockfd = SOCKET_ERROR;
	}
}

bool NetControl::connectHost(DWORD dwAddr, WORD wPort)
{
	SOCKADDR_IN serverAddr;
	char szBuffer[BUFFERSIZE];
	RET_COMP_SE(m_serverfd = socket(AF_INET, SOCK_STREAM, 0));

	BZERO(&serverAddr, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(dwAddr);
	serverAddr.sin_port = htons(wPort);
	RET_COMP_SE(connect(m_serverfd, (SOCKADDR*)&serverAddr, sizeof(SOCKADDR)));

	*(Directive*)szBuffer = OL_MESSAGE;
	strcpy(szBuffer + SIZE_DIRECTIVE, m_myName);
	return send(m_serverfd, szBuffer, strlen(m_myName) + SIZE_DIRECTIVE, 0) != SOCKET_ERROR;
}

bool NetControl::createHost(unsigned short port)
{
	DWORD optval = 1;
	SOCKADDR_IN serverAddr;
	addText("\r\n网络初始化中...");
	RET_COMP_SE(m_listenfd = socket(AF_INET, SOCK_STREAM, 0));
	RET_COMP_SE(setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, (LPCSTR)&optval, sizeof(DWORD)));

	BZERO(&serverAddr, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(port);

	RET_COMP_SE(bind(m_listenfd, (SOCKADDR*)&serverAddr, sizeof(SOCKADDR)));
	RET_COMP_SE(listen(m_listenfd, BACKLOG));

	return true;
}

void NetControl::initPool()
{
	m_pool.maxfd = m_listenfd;
	FD_ZERO(&m_pool.readSet);
	FD_SET(m_listenfd, &m_pool.readSet);
}

int NetControl::waitForClient(clock_t dwDelay)
{
	TIMEVAL tvl = {0, dwDelay};
	m_pool.readySet = m_pool.readSet;
	return m_pool.nReady = select(m_pool.maxfd+1, &m_pool.readySet, NULL, NULL, &tvl);
}

// int NetControl::waitForMsgs(clock_t dwDelay)
// {
// 	TIMEVAL tvl = {0, dwDelay};
// 	return m_pool.nReady = select(m_pool.maxfd+1, &m_pool.readySet, NULL, NULL, &tvl);
// }

void NetControl::statusSync()
{
	SOCKET* psockfd;
	char szBuffer[BUFFERSIZE];
	NetControl::User* puser;
	int buflen, addrLen = sizeof(SOCKADDR);
	SOCKADDR_IN userAddr;

	psockfd = m_pool.readySet.fd_array;
	for(int i = 0; i < m_pool.readySet.fd_count; ++i, ++psockfd)
	{
		if(*psockfd == m_listenfd)
		{
			SOCKET newfd = accept(m_listenfd, (SOCKADDR*)&userAddr, &addrLen);
			if(newfd != SOCKET_ERROR)
			{
				*(Directive*)szBuffer = OL_MESSAGE;
				send(newfd, szBuffer, SIZE_DIRECTIVE + sprintf(szBuffer + SIZE_DIRECTIVE, "\r\n游戏已开始,无法接入..."), 0);
				closesocket(newfd);
			}
		}
		else
		{
			buflen = recv(*psockfd, szBuffer, BUFFERSIZE, 0);
			puser = findUserBySocket(*psockfd);
			*(Directive*)szBuffer = OL_LOSE;
			if(buflen <= 0)
			{
				if(puser != NULL)
				{
					buflen = sprintf(szBuffer + SIZE_DIRECTIVE, "User %s ( Addr: %s,Port: %d ) losed...\n", puser->name, inet_ntoa(puser->addr), puser->port);
					m_status += (szBuffer + SIZE_DIRECTIVE);
					if(m_status.size() > 140)
					{
						string::size_type n = m_status.find('\n');
						if(n == string::npos) m_status.erase(m_status.begin(), m_status.begin() + n);
					}
					*puser->name = 0;
					puser->sockfd = SOCKET_ERROR;
				}
				closesocket(*psockfd);
				FD_CLR(*psockfd, &m_pool.readSet);
				sendMsg(szBuffer, buflen + SIZE_DIRECTIVE);
				if(m_pool.readSet.fd_count <= 1)
				{
					// Server Win.
					onlineWin();
					return ;
				}
			}
		}
// 		else if(buflen != BUFFERSIZE && buflen >= SIZE_DIRECTIVE)
// 		{
// 			szBuffer[buflen] = 0;
// 			switch(*(Directive*)szBuffer)
// 			{
// 			case OL_LOSE:
// 				m_status += szBuffer + SIZE_DIRECTIVE;
// 				break;
// 			case OL_WIN:
// 
// 			default:
// 				;
// 			}
// 		}
	}
}

// void NetControl::startServer()
// {
// 	FD_CLR(m_listenfd, &m_pool.readySet);
// 	m_pool.maxfd = 0;
// 	for(int i = 0; i != m_pool.readySet.fd_count; ++i)
// 	{
// 		if(m_pool.maxfd < m_pool.readySet.fd_array[i])
// 			m_pool.maxfd = m_pool.readySet.fd_array[i];
// 	}
// 	g_bRun = (m_pool.readySet.fd_count != 0);
// }

int NetControl::waitForServer(LPSTR lpMsg)
{
	return recv(m_serverfd, lpMsg, BUFFERSIZE, 0);
}

void NetControl::dealMsgs()
{
	SOCKET* psockfd;
	int buflen, addrLen = sizeof(SOCKADDR);
	char szRecv[BUFFERSIZE], szSend[BUFFERSIZE];
	SOCKADDR_IN userAddr;
	NetControl::User* puser;

	psockfd = m_pool.readySet.fd_array;
	for(int i = 0; i < m_pool.readySet.fd_count; ++i, ++psockfd)
	{
		if(*psockfd == m_listenfd)
		{
			SOCKET newfd = accept(m_listenfd, (SOCKADDR*)&userAddr, &addrLen);
			if(newfd == SOCKET_ERROR)
			{
				addText("\r\n一个新用户加入时发生错误...");
				return;
			}

			addText("\r\n有新用户尝试加入...");
			puser = findUserBySocket(SOCKET_ERROR);
			if(puser == NULL)
			{
				send(newfd, szSend, sprintf(szSend, "\r\n最多允许%d个玩家加入...服务器人满了...", BACKLOG), 0);
				addText(szSend);
				closesocket(newfd);
			}
			else
			{
				*puser->name = 0;
				puser->addr = userAddr.sin_addr;
				puser->port = ntohs(userAddr.sin_port);
				puser->sockfd = newfd;
				send(newfd, szSend, sprintf(szSend, "\r\n主机 %s 欢迎你的加入!", m_myName), 0);
				sprintf(szSend, "新用户IP:%s , Port: %d", inet_ntoa(puser->addr), puser->port);
				addText(szSend);
				FD_SET(newfd, &m_pool.readSet);
				if(newfd > m_pool.maxfd) m_pool.maxfd = newfd;
			}
		}
		else
		{
			buflen = recv(*psockfd, szRecv, BUFFERSIZE, 0);
			puser = findUserBySocket(*psockfd);
			*(Directive*)szSend = OL_MESSAGE;
			if(buflen <= 0)
			{
				if(puser != NULL)
				{
					buflen = sprintf(szSend + SIZE_DIRECTIVE, "\r\n用户 %s ( %s : %d ) 退出...", puser->name, inet_ntoa(puser->addr), puser->port);
					addText(szSend + SIZE_DIRECTIVE);
					*puser->name = 0;
					puser->sockfd = SOCKET_ERROR;
				}
				closesocket(*psockfd);
				FD_CLR(*psockfd, &m_pool.readSet);
				sendMsg(szSend, buflen + SIZE_DIRECTIVE);
			}
			else if(buflen != BUFFERSIZE && *(Directive*)szRecv == OL_MESSAGE)
			{
				szRecv[buflen] = 0;
				if(*puser->name == 0)
				{
					szRecv[127 + SIZE_DIRECTIVE] = 0;
					strcpy(puser->name, szRecv + SIZE_DIRECTIVE);
					buflen = sprintf(szSend + SIZE_DIRECTIVE, "\r\n新用户 %s 加入 ...", puser->name);
					addText(szSend + SIZE_DIRECTIVE);
					sendMsg(szSend, buflen + SIZE_DIRECTIVE);
				}
				else if(buflen > SIZE_DIRECTIVE && *(Directive*)szRecv == OL_MESSAGE)
				{
					addText(szRecv + SIZE_DIRECTIVE);
					sendMsg(szRecv, buflen);
				}
			}
		}
	}
}

NetControl::User* NetControl::findUserBySocket(SOCKET sockfd)
{
	for(int i=0; i != BACKLOG; ++i)
	{
		if(m_users[i].sockfd == sockfd)
		{
			return m_users + i;
		}
	}
	return NULL;
}

void NetControl::sendMsg(LPCSTR lpMsg, int len)
{
	if(m_isServer)
	{
		for(int i = 0; i != BACKLOG; ++i)
		{
			if(m_users[i].sockfd != SOCKET_ERROR)
			{
				send(m_users[i].sockfd, lpMsg, len, 0);
			}
		}
	}
	else
	{
		send(m_serverfd, lpMsg, len, 0);
	}
}

Word* NetControl::getServerSyncWord()
{
	Word* word;
	if(!m_wordToClient.empty())
	{
		word = m_wordToClient[0];
		g_netCtrl.lock(INFINITE);
		m_wordToClient.erase(m_wordToClient.begin());
		g_netCtrl.unLock();
	}
	else word = NULL;
	return word;
}

void NetControl::serverSync()
{
	char szBuffer[BUFFERSIZE];
	int buflen;
	*(Directive*)szBuffer = OL_WORD;
	while(!m_wordToClient.empty())
	{
		buflen = m_wordToClient[0]->makeWordMsg(szBuffer + SIZE_DIRECTIVE, BUFFERSIZE - SIZE_DIRECTIVE);
		g_netCtrl.lock(INFINITE);
		m_wordToClient.erase(m_wordToClient.begin());
		g_netCtrl.unLock();
		if(buflen != 0)
			sendMsg(szBuffer, buflen + SIZE_DIRECTIVE);
	}
}

bool NetControl::clientSync()
{
	char szBuffer[BUFFERSIZE];
	int buflen;

	buflen = recv(m_serverfd, szBuffer, BUFFERSIZE-1, 0);
	if(buflen >= SIZE_DIRECTIVE && buflen != BUFFERSIZE-1)
	{
		szBuffer[buflen] = 0;
		switch(*(Directive*)szBuffer)
		{
		case OL_WORD:
			g_netCtrl.lock(INFINITE);  //Sync between different threads.
			g_game->makeAndAddWord(szBuffer + SIZE_DIRECTIVE, buflen - SIZE_DIRECTIVE);
			g_netCtrl.unLock();
			break;
		case OL_LOSE:
			m_status += szBuffer + SIZE_DIRECTIVE;
			if(m_status.size() > 140)
			{
				string::size_type n = m_status.find('\n');
				if(n == string::npos) m_status.erase(m_status.begin(), m_status.begin() + n);
			}
			break;
		case OL_WIN:
			onlineWin();
			break;
		default:;
		}
	}
	else if(buflen <= 0) return false;
	return true;
}

void NetControl::showStaus()
{
	g_fnt3->printfb(500, 500, 280, 80, HGETEXT_LEFT, m_status.c_str());
}

void NetControl::setStartTitle()
{
	string s;
	if(m_isServer)
		s = "服务器(";
	else s = "客户机(";
	s += m_myName;
	s += "): 联机模式游戏中...";
	g_hge->System_SetState(HGE_TITLE, s.c_str());	
}

void NetControl::lock(DWORD dwTime)
{
	EnterCriticalSection(&m_cs);
}

void NetControl::unLock()
{
	LeaveCriticalSection(&m_cs);
}

void wait4Client(void* arg)
{
	bool* bRun = (bool*)arg;
	if(!*bRun || !g_netCtrl.isServer()) _endthread();
	g_netCtrl.initPool();
	addText("\r\n等待其他玩家加入中... \r\n请将下方本机IP和端口告诉想要加入的用户!");
	
	while(*bRun)
	{
		if(g_netCtrl.waitForClient(25000) == SOCKET_ERROR)
		{
			addText("\r\n主机已断开连接...");
			break;
		}
		g_netCtrl.dealMsgs();
		if(g_netCtrl.isServerReady())
			EnableWindow(GetDlgItem(g_hDlg, IDOK), TRUE);
		else EnableWindow(GetDlgItem(g_hDlg, IDOK), FALSE);
	}
#ifdef _DEBUG
	g_hge->System_SetState(HGE_TITLE, "主机连接线程已退出");
#endif
	_endthread();
}

void wait4Server(void* arg)
{
	bool* bRun = (bool*)arg;
	char szBuffer[BUFFERSIZE];
	int len;
	if(!*bRun || g_netCtrl.isServer()) _endthread();
	addText("\r\n开始聊天吧!");

	while(*bRun)
	{
		len = g_netCtrl.waitForServer(szBuffer);
		szBuffer[len] = 0;
		if(len == SOCKET_ERROR)
		{
			addText("\r\n主机已关闭或其他原因，已和主机断开连接...");
			EnableWindow(GetDlgItem(g_hDlg, IDC_OLCONNECTSERVER), TRUE);
			EnableWindow(GetDlgItem(g_hDlg, IDC_OLCREATESERVER), TRUE);
			SendMessage(g_hDlg, WM_COMMAND, IDC_OLCONNECTSERVER, 0);
			break;
		}
		else if(len == 4 && *(Directive*)szBuffer == OL_START)
		{
			SendMessage(g_hDlg, WM_COMMAND, IDOK, 0);
			break;
		}
		else if(len != BUFFERSIZE && len > SIZE_DIRECTIVE && *(Directive*)szBuffer == OL_MESSAGE)
		{
			addText(szBuffer + SIZE_DIRECTIVE);
		}

	}
#ifdef _DEBUG
	g_hge->System_SetState(HGE_TITLE, "主机连接线程已退出");
#endif
	_endthread();
}

bool onlineClientFrameFunc()
{
	g_dt = g_hge->Timer_GetDelta();
	g_game->onlineClientFrame();
	return false;
}

bool onlineServerFrameFunc()
{
	Word* word;
	g_dt = g_hge->Timer_GetDelta();
	word = g_game->onlineServerFrame();

	g_netCtrl.lock(20);
	g_netCtrl.addServerSyncWord(word);
	g_netCtrl.unLock();
	return false;
}

bool onlineRenderFunc()
{
	g_hge->Gfx_BeginScene();
	if(g_netCtrl.isServer())
		g_game->onlineServerRender();
	else g_game->onlineClientRender();
	g_netCtrl.showStaus();
	g_hge->Gfx_EndScene();

	if(g_game->showSettings())
	{
		g_bRun = false;
		g_game->resetEscape();
		g_hge->System_SetState(HGE_FRAMEFUNC, menuFrameFunc);
		g_hge->System_SetState(HGE_RENDERFUNC, menuRenderFunc);
		g_fnt1->SetScale(1.0f);
		g_gui->Enter();
		g_hge->Channel_StopAll();
		g_hge->Effect_Play(g_effect[HAPPY]);
	}
	if(g_missedWordsNum != 0)
	{
		onlineLose();
	}

	return false;
}

void addText(LPCSTR lpMsg)
{
	int len = GetWindowTextLengthA(g_hMsg);
	SendMessageA(g_hMsg, EM_SETSEL, len, len);
	SendMessageA(g_hMsg, EM_REPLACESEL, TRUE, (LPARAM)lpMsg);
}

/*
	Sync Threads Below.
*/

void clientFrameSync(void* arg)
{
	g_netCtrl.setStartTitle();
	g_bRun = true;
	while(g_bRun)
	{
		if(!g_netCtrl.clientSync())
		{
			MessageBoxA(g_hwnd, "你已掉线，请重新连接~", "你悲剧啦", MB_OK | MB_ICONERROR);
			g_netCtrl.shutdownNetwork();
			g_game->resetEscape();
			g_hge->System_SetState(HGE_FRAMEFUNC, menuFrameFunc);
			g_hge->System_SetState(HGE_RENDERFUNC, menuRenderFunc);
			g_fnt1->SetScale(1.0f);
			g_gui->Enter();
			g_hge->Channel_StopAll();
			g_hge->Effect_Play(g_effect[HAPPY]);
			break;
		}
	}
	g_netCtrl.shutdownNetwork();
	_endthread();
}

void serverFrameSync(void* arg)
{
//	g_netCtrl.startServer();
	g_netCtrl.setStartTitle();
	while(g_bRun)
	{
		int ret;
		g_netCtrl.serverSync();
		ret = g_netCtrl.waitForClient(50000);

		if(ret > 0)
		{
			g_netCtrl.statusSync();
		}
		else if(ret == SOCKET_ERROR)
		{
			MessageBoxA(g_hwnd, "服务器网络出现错误，请重新开始游戏~", "你悲剧啦", MB_OK | MB_ICONERROR);
			g_netCtrl.shutdownNetwork();
			g_game->resetEscape();
			g_hge->System_SetState(HGE_FRAMEFUNC, menuFrameFunc);
			g_hge->System_SetState(HGE_RENDERFUNC, menuRenderFunc);
			g_fnt1->SetScale(1.0f);
			g_gui->Enter();
			g_hge->Channel_StopAll();
			g_hge->Effect_Play(g_effect[HAPPY]);
			break;
		}
	}
	g_netCtrl.shutdownNetwork();
	_endthread();
}

void onlineWin()
{
	HTARGET htar = g_hge->Target_Create(800, 600, false);
	g_hge->Gfx_BeginScene(htar);	// Save the scene;
	g_hge->Gfx_Clear(0);
	g_game->stageRender();
	g_netCtrl.showStaus();
	g_hge->Gfx_EndScene();
	g_bRun = false;
	initBubbles(htar);
	g_hge->System_SetState(HGE_FRAMEFUNC, bubbleFrameFunc);
	g_hge->System_SetState(HGE_RENDERFUNC, bubbleRenderFunc);
}

void onlineLose()
{
	char szBuffer[BUFFERSIZE];
	*(Directive*)szBuffer = OL_WIN;
	g_netCtrl.sendMsg(szBuffer, SIZE_DIRECTIVE);
	g_bRun = false;
	initLoseAction();
	g_hge->System_SetState(HGE_FRAMEFUNC, loseFrameFunc);
	g_hge->System_SetState(HGE_RENDERFUNC, loseRenderFunc);
}