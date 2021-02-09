#include <QCoreApplication>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include "jcommand.h"
#include "utils.h"

#include <unistd.h>
#include "CBaseClass.h"


class USBClient {
public:
	USBClient();

	bool Connect();
	bool Disconnect();
	bool Alive();
	bool Scan();
	bool Start();
	bool Reset();

private:
	bool                Init();
	uint8_t             m_RxBuf[4096];
	uint8_t             m_TxBuf[4096];

	int                 m_Sock;
	struct sockaddr_in  m_SvrAddr;
	socklen_t           m_AddrLen;

};

USBClient::USBClient()
{
	if( m_Sock > 0 )
		close(m_Sock);
}

bool USBClient::Init()
{
	if((m_Sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		return false;
	}

	memset(&m_SvrAddr, 0x00, sizeof(m_SvrAddr));
	m_SvrAddr.sin_family = AF_INET;
	m_SvrAddr.sin_addr.s_addr = inet_addr( "127.0.0.1" );
	m_SvrAddr.sin_port = htons( 5050 );
	m_AddrLen = sizeof(m_SvrAddr);
	return true;
}

bool USBClient::Connect()
{
	int ready;
	int32_t recvLen;
	struct pollfd fds[2];
	int32_t sendSize;
	CJCommand cmd;
	QJsonDocument doc = cmd.Connect();

	int32_t len = doc.toJson().size();
	memcpy(m_TxBuf, &len, sizeof(len));
	memcpy(m_TxBuf+4, doc.toJson().data(), len );

	if( !Init() )
		return false;

	// Send Command
	sendSize = sendto(m_Sock, m_TxBuf, len+4, 0, (struct sockaddr*)&m_SvrAddr, m_AddrLen);

	if( sendSize != (len+4) )
	{
		//	Send fail
		return false;
	}

	//  poll
	memset( fds, 0, sizeof(fds) );
	fds[0].fd = m_Sock;
	fds[0].events |= POLLIN;
	ready = poll( fds, 1, 500 );   // 500 msec

	if( ready == 0 )
	{
		close( m_Sock );
		m_Sock = -1;
		return false;
	}

	if( fds[0].revents & POLLIN )
	{
		 // Receive Response
		memset( m_RxBuf, 0, sizeof(m_RxBuf) );
		if((recvLen = recvfrom(m_Sock, m_RxBuf, 1024, 0, (struct sockaddr *)&m_SvrAddr, &m_AddrLen)) < 0)
		{
			return false;
		}

		//  Check Connect Command
		int32_t len;
		memcpy( &len, m_RxBuf, 4 );
		if( len + 4 != recvLen )
		{
			printf("Protocol Error !!!\n");
			return false;
		}
		QJsonDocument resDoc = QJsonDocument::fromJson((char *)(m_RxBuf+4));
		QJsonObject resObj = resDoc.object();
		QString cmd = resObj["command"].toString();
		if( cmd != "connect" )
		{
			printf("Error!!!(%s)\n", cmd.toStdString().data());
			return false;
		}
		QString status = resObj["status"].toString();
		if( status != "ok" )
		{
			printf("Connect return fail(%s)\n", status.toStdString().data());
			return false;
		}
	}
	else
	{
		printf("SendCmd : recvfrom timeout!!!\n");
		return false;
	}

	// Close Socket
	close(m_Sock);
	m_Sock = -1;
	return true;
}


bool USBClient::Disconnect()
{
	int ready;
	int32_t recvLen;
	struct pollfd fds[2];
	int32_t sendSize;
	CJCommand cmd;
	QJsonDocument doc = cmd.Disconnect();

	int32_t len = doc.toJson().size();
	memcpy(m_TxBuf, &len, sizeof(len));
	memcpy(m_TxBuf+4, doc.toJson().data(), len );

	if( !Init() )
		return false;

	// Send Command
	sendSize = sendto(m_Sock, m_TxBuf, len+4, 0, (struct sockaddr*)&m_SvrAddr, m_AddrLen);
	if( sendSize != (len+4) )
	{
		//	send fail
		return false;
	}

	//  poll
	memset( fds, 0, sizeof(fds) );
	fds[0].fd = m_Sock;
	fds[0].events |= POLLIN;
	ready = poll( fds, 1, 500 );   // 500 msec

	if( ready == 0 )	//	wait timeout
	{
		close(m_Sock);
		m_Sock = -1;
		return false;
	}

	if( fds[0].revents & POLLIN )
	{
		 // Receive Response
		memset( m_RxBuf, 0, sizeof(m_RxBuf) );
		if((recvLen = recvfrom(m_Sock, m_RxBuf, 1024, 0, (struct sockaddr *)&m_SvrAddr, &m_AddrLen)) < 0)
		{
			return false;
		}

		//  Check Disconnect Command
		int32_t len;
		memcpy( &len, m_RxBuf, 4 );
		if( len + 4 != recvLen )
		{
			printf("Protocol Error !!!\n");
			return false;
		}
		QJsonDocument resDoc = QJsonDocument::fromJson((char *)(m_RxBuf+4));
		QJsonObject resObj = resDoc.object();
		QString cmd = resObj["command"].toString();
		if( cmd != "disconnect" )
		{
			printf("Error!!!(%s)\n", cmd.toStdString().data());
			return false;
		}
		QString status = resObj["status"].toString();
		if( status != "ok" )
		{
			printf("Disconnect return fail(%s)\n", status.toStdString().data());
			return false;
		}
	}
	else
	{
		printf("SendCmd : recvfrom timeout!!!\n");
		return false;
	}

	// Close Socket
	close(m_Sock);
	m_Sock = -1;
	return true;
}


bool USBClient::Alive()
{
	int32_t sendSize;
	CJCommand cmd;
	QJsonDocument doc = cmd.Alive();

	int32_t len = doc.toJson().size();
	memcpy(m_TxBuf, &len, sizeof(len));
	memcpy(m_TxBuf+4, doc.toJson().data(), len );

	if( !Init() )
		return false;

	// Send Command
	sendSize = sendto(m_Sock, m_TxBuf, len+4, 0, (struct sockaddr*)&m_SvrAddr, m_AddrLen);
	if(sendSize <= 0 )
		return false;

	// Close Socket
	close(m_Sock);
	m_Sock = -1;
	return true;
}

bool USBClient::Scan()
{
	int ready;
	int32_t recvLen;
	struct pollfd fds[2];
	int32_t sendSize;
	CJCommand cmd;
	QJsonDocument doc = cmd.Scan();

	int32_t len = doc.toJson().size();
	memcpy(m_TxBuf, &len, sizeof(len));
	memcpy(m_TxBuf+4, doc.toJson().data(), len );

	if( !Init() )
		return false;

	// Send Command
	sendSize = sendto(m_Sock, m_TxBuf, len+4, 0, (struct sockaddr*)&m_SvrAddr, m_AddrLen);
	if( sendSize != len+4 )
	{
		// Sendfail
		return false;
	}

	//  poll
	memset( fds, 0, sizeof(fds) );
	fds[0].fd = m_Sock;
	fds[0].events |= POLLIN;
	ready = poll( fds, 1, 500 );   // 500 msec

	if( ready == 0 )
	{
		// Timeout
		close( m_Sock );
		m_Sock = -1;
	}

	if( fds[0].revents & POLLIN )
	{
		 // Receive Response
		memset( m_RxBuf, 0, sizeof(m_RxBuf) );
		if((recvLen = recvfrom(m_Sock, m_RxBuf, 1024, 0, (struct sockaddr *)&m_SvrAddr, &m_AddrLen)) < 0)
		{
			return false;
		}

		//  Check Connect Command
		int32_t len;
		memcpy( &len, m_RxBuf, 4 );
		if( len + 4 != recvLen )
		{
			printf("Protocol Error !!!\n");
			return false;
		}
		QJsonDocument resDoc = QJsonDocument::fromJson((char *)(m_RxBuf+4));
		QJsonObject resObj = resDoc.object();
		QString cmd = resObj["command"].toString();
		if( cmd != "scan" )
		{
			printf("Error!!!(%s)\n", cmd.toStdString().data());
			return false;
		}
	}
	else
	{
		printf("SendCmd : recvfrom timeout!!!\n");
		return false;
	}

	// Close Socket
	close(m_Sock);
	m_Sock = -1;
	return true;
}

bool USBClient::Start()
{
	int ready;
	int32_t recvLen;
	struct pollfd fds[2];
	int32_t sendSize;
	CJCommand cmd;
	QString port = "all";
	QJsonDocument doc = cmd.Start( port );

	int32_t len = doc.toJson().size();
	memcpy(m_TxBuf, &len, sizeof(len));
	memcpy(m_TxBuf+4, doc.toJson().data(), len );

	if( !Init() )
		return false;

	// Send Command
	sendSize = sendto(m_Sock, m_TxBuf, len+4, 0, (struct sockaddr*)&m_SvrAddr, m_AddrLen);
	if( sendSize != (len+4) )
	{
		close(m_Sock);
		return false;
	}

	//  poll
	memset( fds, 0, sizeof(fds) );
	fds[0].fd = m_Sock;
	fds[0].events |= POLLIN;
	ready = poll( fds, 1, 500 );   // 500 msec

	if( ready == 0 )
	{
		close( m_Sock );
		m_Sock = -1;
		return false;
	}

	if( fds[0].revents & POLLIN )
	{
		 // Receive Response
		memset( m_RxBuf, 0, sizeof(m_RxBuf) );
		if((recvLen = recvfrom(m_Sock, m_RxBuf, 1024, 0, (struct sockaddr *)&m_SvrAddr, &m_AddrLen)) < 0)
		{
			return false;
		}

		//  Check Connect Command
		int32_t len;
		memcpy( &len, m_RxBuf, 4 );
		if( len + 4 != recvLen )
		{
			printf("Protocol Error !!!\n");
			return false;
		}
		QJsonDocument resDoc = QJsonDocument::fromJson((char *)(m_RxBuf+4));
		QJsonObject resObj = resDoc.object();
		QString cmd = resObj["command"].toString();
		if( cmd != "start" )
		{
			printf("Error!!!(%s)\n", cmd.toStdString().data());
			return false;
		}
	}
	else
	{
		printf("SendCmd : recvfrom timeout!!!\n");
		return false;
	}

	// Close Socket
	close(m_Sock);
	m_Sock = -1;
	return true;
}

bool USBClient::Reset()
{
	int ready;
	int32_t recvLen;
	struct pollfd fds[2];
	int32_t sendSize;
	CJCommand cmd;
	QString port = "all";
	QJsonDocument doc = cmd.Reset( port );

	int32_t len = doc.toJson().size();
	memcpy(m_TxBuf, &len, sizeof(len));
	memcpy(m_TxBuf+4, doc.toJson().data(), len );

	if( !Init() )
		return false;

	// Send Command
	sendSize = sendto(m_Sock, m_TxBuf, len+4, 0, (struct sockaddr*)&m_SvrAddr, m_AddrLen);
	if(sendSize != (len+4))
	{
		close(m_Sock);
		m_Sock = -1;
		return false;
	}

	//  poll
	memset( fds, 0, sizeof(fds) );
	fds[0].fd = m_Sock;
	fds[0].events |= POLLIN;
	ready = poll( fds, 1, 500 );   // 500 msec

	if( ready == 0 )
	{
		close( m_Sock );
		m_Sock = -1;
		return false;
	}

	if( fds[0].revents & POLLIN )
	{
		 // Receive Response
		memset( m_RxBuf, 0, sizeof(m_RxBuf) );
		if((recvLen = recvfrom(m_Sock, m_RxBuf, 1024, 0, (struct sockaddr *)&m_SvrAddr, &m_AddrLen)) < 0)
		{
			return false;
		}

		//  Check Connect Command
		int32_t len;
		memcpy( &len, m_RxBuf, 4 );
		if( len + 4 != recvLen )
		{
			printf("Protocol Error !!!\n");
			return false;
		}
		QJsonDocument resDoc = QJsonDocument::fromJson((char *)(m_RxBuf+4));
		QJsonObject resObj = resDoc.object();
		QString cmd = resObj["command"].toString();
		if( cmd != "reset" )
		{
			printf("Error!!!(%s)\n", cmd.toStdString().data());
			return false;
		}
	}
	else
	{
		printf("SendCmd : recvfrom timeout!!!\n");
		return false;
	}

	// Close Socket
	close(m_Sock);
	m_Sock = -1;
	return true;
}


void NotifyCallback( void *, void *buf, int32_t size )
{
	QJsonDocument doc = QJsonDocument::fromJson((char*)buf);
	QJsonObject obj = doc.object();
	QString msg = obj["message"].toString();
	//printf("[Cleint] Message : %s\n", msg.toStdString().data());
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	CNotifyReceiver notifyReceiver;
	notifyReceiver.StartNotifyReceiver( 5060, &NotifyCallback, nullptr );
	USBClient *pClient = new USBClient();

//	for( int i= 0 ; i < 10000 ; i++ )
	{
		pClient->Connect();
		pClient->Alive();
		pClient->Scan();
		pClient->Start();
		pClient->Disconnect();
		pClient->Reset();
	}

	usleep(1000000);

	exit(0);
	return a.exec();
}
