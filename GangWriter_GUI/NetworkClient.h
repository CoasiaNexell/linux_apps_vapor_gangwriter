#ifndef _NETWORKCLIENT_H_
#define _NETWORKCLIENT_H_

#include <stdint.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <QJsonDocument>
#include "CBaseClass.h"

class CNetworkClient {
public:
	CNetworkClient();

	bool Connect();
	bool Disconnect();
	bool Alive();
	bool Scan(QJsonDocument &resDoc);
	bool Start(QJsonDocument &resDoc);
	bool Status(QJsonDocument &resDoc);
	bool Reset();

private:
	bool				Init( int &sock, struct sockaddr_in &svrAddr );
	uint8_t				m_RxBuf[4096];
	uint8_t				m_TxBuf[4096];
	socklen_t			m_AddrLen;
	CMutex				m_NetLock;
	CAliveSender		m_AliveSender;

	bool				m_bConnected;

	static void AliveSendStub( void *obj )
	{
		if( obj )
			((CNetworkClient*)obj)->AliveSender();
	}

	void AliveSender();

};

#endif // _NETWORKCLIENT_H_
