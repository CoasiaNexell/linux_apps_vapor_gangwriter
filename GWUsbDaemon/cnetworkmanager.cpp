#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <poll.h>
#include "cnetworkmanager.h"
#include "jcommand.h"
#include "utils.h"
#include "cusbmanager.h"
#include "CUsbNode.h"

#define LOG_TAG "[USB DEAMON]"
#include "NxDbgMsg.h"

CNetworkManager::CNetworkManager()
	: m_Port( -1 )
	, m_bExitLoop( true )
{
	m_pUsbMgr = new CUsbManager();
}


bool CNetworkManager::Start( int32_t port )
{
	m_bExitLoop = false;
	m_Port = port;
	if( 0 != pthread_create(&m_hThread, NULL, this->ThreadProcStub, this ) )
	{
		m_bExitLoop = true;
		return false;
	}
	return true;
}

void CNetworkManager::Stop()
{
	m_bExitLoop = true;
	//  Send Exit
}


void CNetworkManager::ThreadProc()
{
	int opt, ready;
	struct sockaddr_in addr;
	struct sockaddr_in clientAddr;
	int recvLen;
	socklen_t addrLen;
	struct pollfd pfds[2];
	int32_t bufLen;

	if( 0 > (m_Sock = socket(AF_INET, SOCK_DGRAM, 0)) )
	{
		goto ERROR_EXIT;
	}

	memset( &addr, 0, sizeof(addr) );
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(m_Port);

	opt = 1;
	if(0 > setsockopt(m_Sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt) ) )
	{
		goto ERROR_EXIT;
	}

	if(0 > setsockopt(m_Sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt) ) )
	{
		goto ERROR_EXIT;
	}

	if( 0 > bind(m_Sock, (struct sockaddr*)&addr, sizeof(addr)) )
	{
		goto ERROR_EXIT;
	}

	while ( !m_bExitLoop )
	{
		//  poll
		memset( pfds, 0, sizeof(pfds) );
		pfds[0].fd =m_Sock;
		pfds[0].events |= POLLIN;

		ready = poll( pfds, 1, 500 );   // 500 msec

		//  Timeout
		if( ready == 0 ){
			continue;
		}
		else if( 0 > ready)
		{
			// error
			NXLOGE("poll error!!!\n");
			break;
		}

		addrLen = sizeof(clientAddr);
		recvLen = recvfrom( m_Sock, (void*)m_RxBuf, sizeof(m_RxBuf), 0, (struct sockaddr*)&clientAddr, &addrLen );
		//  Don't delete : QJsonDument::fromJson find EOS or null charactor.
		m_RxBuf[recvLen] = 0;

		if( recvLen > 0 )
		{
			//	Parsing Command
			QJsonDocument doc = QJsonDocument::fromJson((char*)(m_RxBuf+4));
			QJsonObject obj = doc.object();
			QString cmd = obj["command"].toString();
			QString ver = obj["version"].toString();

			// NXLOGV("[Server] receive command = %s \n", cmd.toStdString().data());
			bufLen = sizeof(m_TxBuf);
			if( cmd == "connect" )
			{
				m_pUsbMgr->Connect( m_TxBuf, bufLen, clientAddr.sin_addr.s_addr );
				sendto( m_Sock, m_TxBuf, bufLen, 0, (struct sockaddr*)&clientAddr, addrLen );
			}
			else if( cmd == "disconnect" )
			{
				m_pUsbMgr->Disconnect( m_TxBuf, bufLen, clientAddr.sin_addr.s_addr );
				sendto( m_Sock, m_TxBuf, bufLen, 0, (struct sockaddr*)&clientAddr, addrLen );
			}
			else if( cmd == "alive" )
			{
				// no reply
				m_pUsbMgr->Alive();
			}
			else if( cmd == "scan" )
			{
				m_pUsbMgr->Scan( m_TxBuf, bufLen );
				//HexDump(m_TxBuf, bufLen);
				sendto( m_Sock, m_TxBuf, bufLen, 0, (struct sockaddr*)&clientAddr, addrLen );
			}
			else if( cmd == "start" )
			{
				m_pUsbMgr->Start( m_TxBuf, bufLen );
				sendto( m_Sock, m_TxBuf, bufLen, 0, (struct sockaddr*)&clientAddr, addrLen );
			}
			else if( cmd == "reset" )
			{
				m_pUsbMgr->Reset( m_TxBuf, bufLen );
				sendto( m_Sock, m_TxBuf, bufLen, 0, (struct sockaddr*)&clientAddr, addrLen );
			}
			else
			{
				NXLOGW("unknown command : (%s)\n", cmd.toStdString().data());
			}

		}
	}

	ERROR_EXIT:
	if( m_Sock > 0 )
	{
		close( m_Sock );
		m_Sock = -1;
	}
}
