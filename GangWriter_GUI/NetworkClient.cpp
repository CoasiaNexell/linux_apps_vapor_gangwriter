#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>
#include "NetworkClient.h"
#include "jcommand.h"

#define LOG_TAG "[NetworkClient]"
#include "NxDbgMsg.h"

CNetworkClient::CNetworkClient()
{
	m_AliveSender.StartAlvieSender( AliveSendStub, this, 1000 );
	m_bConnected = false;
}

bool CNetworkClient::Init( int &sock, struct sockaddr_in &svrAddr )
{
	if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		return false;
	}
	memset(&svrAddr, 0x00, sizeof(svrAddr));
	svrAddr.sin_family = AF_INET;
	svrAddr.sin_addr.s_addr = inet_addr( "127.0.0.1" );
	svrAddr.sin_port = htons( 5050 );
	m_AddrLen = sizeof(svrAddr);
	return true;
}

bool CNetworkClient::Connect()
{
	struct pollfd fds[2];
	int32_t sendSize, recvLen;
	int sock, ready;
	struct sockaddr_in svrAddr;

	CAutoLock lock(m_NetLock);
	CJCommand cmd;
	QJsonDocument doc = cmd.Connect();

	int32_t len = doc.toJson().size();
	memcpy(m_TxBuf, &len, sizeof(len));
	memcpy(m_TxBuf+4, doc.toJson().data(), len );

	if( !Init(sock, svrAddr) )
		return false;

	// Send Command
	sendSize = sendto(sock, m_TxBuf, len+4, 0, (struct sockaddr*)&svrAddr, m_AddrLen);

	if( sendSize != (len+4) )
	{
		//	Send fail
		return false;
	}

	//  poll
	memset( fds, 0, sizeof(fds) );
	fds[0].fd = sock;
	fds[0].events |= POLLIN;
	ready = poll( fds, 1, 500 );   // 500 msec

	if( ready == 0 )
	{
		close( sock );
		return false;
	}

	if( fds[0].revents & POLLIN )
	{
		 // Receive Response
		memset( m_RxBuf, 0, sizeof(m_RxBuf) );
		if((recvLen = recvfrom(sock, m_RxBuf, 1024, 0, (struct sockaddr *)&svrAddr, &m_AddrLen)) < 0)
		{
			close( sock );
			return false;
		}

		//  Check Connect Command
		int32_t len;
		memcpy( &len, m_RxBuf, 4 );
		if( len + 4 != recvLen )
		{
			NXLOGE("Protocol Error !!!\n");
			close( sock );
			return false;
		}
		QJsonDocument resDoc = QJsonDocument::fromJson((char *)(m_RxBuf+4));
		QJsonObject resObj = resDoc.object();
		QString cmd = resObj["command"].toString();
		QString status = resObj["status"].toString();
		if( cmd != "connect" || status != "ok" )
		{
			NXLOGE("Error!!!(%s)\n", cmd.toStdString().data());
			close( sock );
			return false;
		}
	}
	else
	{
		NXLOGE("SendCmd : recvfrom timeout!!!\n");
		close( sock );
		return false;
	}

	// Close Socket
	close(sock);
	m_bConnected = true;
	return true;
}


bool CNetworkClient::Disconnect()
{
	int32_t recvLen;
	struct pollfd fds[2];
	int32_t sendSize;
	CJCommand cmd;
	QJsonDocument doc = cmd.Disconnect();
	int sock, ready;
	struct sockaddr_in svrAddr;
	CAutoLock lock(m_NetLock);

	int32_t len = doc.toJson().size();
	memcpy(m_TxBuf, &len, sizeof(len));
	memcpy(m_TxBuf+4, doc.toJson().data(), len );

	if( !Init(sock, svrAddr) )
		return false;

	// Send Command
	sendSize = sendto(sock, m_TxBuf, len+4, 0, (struct sockaddr*)&svrAddr, m_AddrLen);
	if( sendSize != (len+4) )
	{
		//	send fail
		close(sock);
		return false;
	}

	//  poll
	memset( fds, 0, sizeof(fds) );
	fds[0].fd = sock;
	fds[0].events |= POLLIN;
	ready = poll( fds, 1, 500 );   // 500 msec

	if( ready == 0 )	//	wait timeout
	{
		close(sock);
		return false;
	}

	if( fds[0].revents & POLLIN )
	{
		 // Receive Response
		memset( m_RxBuf, 0, sizeof(m_RxBuf) );
		if((recvLen = recvfrom(sock, m_RxBuf, 1024, 0, (struct sockaddr *)&svrAddr, &m_AddrLen)) < 0)
		{
			close(sock);
			return false;
		}

		//  Check Disconnect Command
		int32_t len;
		memcpy( &len, m_RxBuf, 4 );
		if( len + 4 != recvLen )
		{
			NXLOGE("Protocol Error !!!\n");
			close(sock);
			return false;
		}
		QJsonDocument resDoc = QJsonDocument::fromJson((char *)(m_RxBuf+4));
		QJsonObject resObj = resDoc.object();
		QString cmd = resObj["command"].toString();
		QString status = resObj["status"].toString();
		if( cmd != "disconnect" || status != "ok" )
		{
			NXLOGE("Error!!!(%s)\n", cmd.toStdString().data());
			close(sock);
			return false;
		}
	}
	else
	{
		NXLOGE("SendCmd : recvfrom timeout!!!\n");
		close( sock );
		return false;
	}

	// Close Socket
	close(sock);
	m_bConnected = false;
	return true;
}


bool CNetworkClient::Alive()
{
	int32_t sendSize, len;
	CJCommand cmd;
	int sock;
	struct sockaddr_in svrAddr;
	CAutoLock lock(m_NetLock);

	QJsonDocument doc = cmd.Alive();
	len = doc.toJson().size();
	memcpy(m_TxBuf, &len, sizeof(len));
	memcpy(m_TxBuf+4, doc.toJson().data(), len );

	if( !Init(sock, svrAddr) )
		return false;

	// Send Command
	sendSize = sendto(sock, m_TxBuf, len+4, 0, (struct sockaddr*)&svrAddr, m_AddrLen);
	if(sendSize <= 0 )
	{
		close(sock);
		return false;
	}

	// Close Socket
	close(sock);
	return true;
}

bool CNetworkClient::Scan(QJsonDocument &resDoc)
{
	int ready;
	int32_t recvLen;
	struct pollfd fds[2];
	int32_t sendSize;
	CJCommand cmd;
	QJsonDocument doc = cmd.Scan();
	int sock;
	struct sockaddr_in svrAddr;
	CAutoLock lock(m_NetLock);

	int32_t len = doc.toJson().size();
	memcpy(m_TxBuf, &len, sizeof(len));
	memcpy(m_TxBuf+4, doc.toJson().data(), len );

	if( !Init(sock, svrAddr) )
		return false;

	// Send Command
	sendSize = sendto(sock, m_TxBuf, len+4, 0, (struct sockaddr*)&svrAddr, m_AddrLen);
	if( sendSize != len+4 )
	{
		// Sendfail
		return false;
	}

	//  poll
	memset( fds, 0, sizeof(fds) );
	fds[0].fd = sock;
	fds[0].events |= POLLIN;
	ready = poll( fds, 1, 500 );   // 500 msec

	if( ready == 0 )
	{
		// Timeout
		close( sock );
	}

	if( fds[0].revents & POLLIN )
	{
		 // Receive Response
		memset( m_RxBuf, 0, sizeof(m_RxBuf) );
		if((recvLen = recvfrom(sock, m_RxBuf, 1024, 0, (struct sockaddr *)&svrAddr, &m_AddrLen)) < 0)
		{
			close( sock );
			return false;
		}

		//  Check Connect Command
		int32_t len;
		memcpy( &len, m_RxBuf, 4 );
		if( len + 4 != recvLen )
		{
			NXLOGE("Protocol Error !!!\n");
			close( sock );
			return false;
		}
		resDoc = QJsonDocument::fromJson((char *)(m_RxBuf+4));
		QJsonObject resObj = resDoc.object();
		QString cmd = resObj["command"].toString();
		if( cmd != "scan" )
		{
			NXLOGE("Error!!!(%s)\n", cmd.toStdString().data());
			close( sock );
			return false;
		}
	}
	else
	{
		NXLOGE("SendCmd : recvfrom timeout!!!\n");
		close( sock );
		return false;
	}

	// Close Socket
	close(sock);
	return true;
}

bool CNetworkClient::Start(QJsonDocument &resDoc)
{
	int32_t recvLen, sendSize, len;
	struct pollfd fds[2];
	CJCommand cmd;
	int sock, ready;
	struct sockaddr_in svrAddr;
	CAutoLock lock(m_NetLock);


	QString port = "all";
	QJsonDocument doc = cmd.Start( port );

	len = doc.toJson().size();
	memcpy(m_TxBuf, &len, sizeof(len));
	memcpy(m_TxBuf+4, doc.toJson().data(), len );
	if( !Init(sock, svrAddr) )
		return false;

	// Send Command
	sendSize = sendto(sock, m_TxBuf, len+4, 0, (struct sockaddr*)&svrAddr, m_AddrLen);
	if( sendSize != (len+4) )
	{
		close(sock);
		return false;
	}

	//  poll
	memset( fds, 0, sizeof(fds) );
	fds[0].fd = sock;
	fds[0].events |= POLLIN;
	ready = poll( fds, 1, 500 );   // 500 msec

	if( ready == 0 )
	{
		close( sock );
		return false;
	}

	if( fds[0].revents & POLLIN )
	{
		 // Receive Response
		memset( m_RxBuf, 0, sizeof(m_RxBuf) );
		if((recvLen = recvfrom(sock, m_RxBuf, 1024, 0, (struct sockaddr *)&svrAddr, &m_AddrLen)) < 0)
		{
			close( sock );
			return false;
		}

		//  Check Connect Command
		int32_t rxLen;
		memcpy( &rxLen, m_RxBuf, 4 );
		if( rxLen + 4 != recvLen )
		{
			NXLOGE("Protocol Error !!!\n");
			close(sock);
			return false;
		}
		resDoc = QJsonDocument::fromJson((char *)(m_RxBuf+4));
		QJsonObject resObj = resDoc.object();
		QString cmd = resObj["command"].toString();
		if( cmd != "start" )
		{
			NXLOGE("Error!!!(%s)\n", cmd.toStdString().data());
			close( sock );
			return false;
		}
	}
	else
	{
		NXLOGE("SendCmd : recvfrom timeout!!!\n");
		close( sock );
		return false;
	}

	// Close Socket
	close(sock);
	return true;
}


bool CNetworkClient::Status(QJsonDocument &resDoc)
{
	int32_t recvLen, sendSize, len;
	struct pollfd fds[2];
	CJCommand cmd;
	int sock, ready;
	struct sockaddr_in svrAddr;
	CAutoLock lock(m_NetLock);


	QString port = "all";
	QJsonDocument doc = cmd.Status();

	len = doc.toJson().size();
	memcpy(m_TxBuf, &len, sizeof(len));
	memcpy(m_TxBuf+4, doc.toJson().data(), len );
	if( !Init(sock, svrAddr) )
		return false;

	// Send Command
	sendSize = sendto(sock, m_TxBuf, len+4, 0, (struct sockaddr*)&svrAddr, m_AddrLen);
	if( sendSize != (len+4) )
	{
		close(sock);
		return false;
	}

	//  poll
	memset( fds, 0, sizeof(fds) );
	fds[0].fd = sock;
	fds[0].events |= POLLIN;
	ready = poll( fds, 1, 500 );   // 500 msec

	if( ready == 0 )
	{
		close( sock );
		return false;
	}

	if( fds[0].revents & POLLIN )
	{
		 // Receive Response
		memset( m_RxBuf, 0, sizeof(m_RxBuf) );
		if((recvLen = recvfrom(sock, m_RxBuf, 1024, 0, (struct sockaddr *)&svrAddr, &m_AddrLen)) < 0)
		{
			close( sock );
			return false;
		}

		//  Check Connect Command
		int32_t rxLen;
		memcpy( &rxLen, m_RxBuf, 4 );
		if( rxLen + 4 != recvLen )
		{
			NXLOGE("Protocol Error !!!\n");
			close(sock);
			return false;
		}
		resDoc = QJsonDocument::fromJson((char *)(m_RxBuf+4));
		QJsonObject resObj = resDoc.object();
		QString cmd = resObj["command"].toString();
		if( cmd != "status" )
		{
			NXLOGE("Error!!!(%s)\n", cmd.toStdString().data());
			close( sock );
			return false;
		}
	}
	else
	{
		NXLOGE("SendCmd : recvfrom timeout!!!\n");
		close( sock );
		return false;
	}

	// Close Socket
	close(sock);
	return true;
}

bool CNetworkClient::Reset()
{
	struct pollfd fds[2];
	int32_t sendSize, recvLen, len;
	CJCommand cmd;
	int sock, ready;
	struct sockaddr_in svrAddr;
	CAutoLock lock(m_NetLock);

	QString port = "all";
	QJsonDocument doc = cmd.Reset( port );

	len = doc.toJson().size();
	memcpy(m_TxBuf, &len, sizeof(len));
	memcpy(m_TxBuf+4, doc.toJson().data(), len );

	if( !Init(sock, svrAddr) )
		return false;

	// Send Command
	sendSize = sendto(sock, m_TxBuf, len+4, 0, (struct sockaddr*)&svrAddr, m_AddrLen);
	if(sendSize != (len+4))
	{
		close(sock);
		return false;
	}

	//  poll
	memset( fds, 0, sizeof(fds) );
	fds[0].fd = sock;
	fds[0].events |= POLLIN;
	ready = poll( fds, 1, 500 );   // 500 msec

	if( ready == 0 )
	{
		close( sock );
		return false;
	}

	if( fds[0].revents & POLLIN )
	{
		 // Receive Response
		memset( m_RxBuf, 0, sizeof(m_RxBuf) );
		if((recvLen = recvfrom(sock, m_RxBuf, 1024, 0, (struct sockaddr *)&svrAddr, &m_AddrLen)) < 0)
		{
			close( sock );
			return false;
		}

		//  Check Connect Command
		int32_t len;
		memcpy( &len, m_RxBuf, 4 );
		if( len + 4 != recvLen )
		{
			NXLOGE("Protocol Error !!!\n");
			close( sock );
			return false;
		}
		QJsonDocument resDoc = QJsonDocument::fromJson((char *)(m_RxBuf+4));
		QJsonObject resObj = resDoc.object();
		QString cmd = resObj["command"].toString();
		if( cmd != "reset" )
		{
			NXLOGE("Error!!!(%s)\n", cmd.toStdString().data());
			close( sock );
			return false;
		}
	}
	else
	{
		NXLOGE("SendCmd : recvfrom timeout!!!\n");
		close( sock );
		return false;
	}

	// Close Socket
	close(sock);
	return true;
}


void CNetworkClient::AliveSender()
{
	if( m_bConnected )
	{
		Alive();
	}
}

