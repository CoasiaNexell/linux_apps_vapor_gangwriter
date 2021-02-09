#ifndef CNETWORKMANAGER_H
#define CNETWORKMANAGER_H

#include <stdint.h>
#include <pthread.h>

class CUsbNode;
class CUsbManager;

class CNetworkManager
{
public:
	CNetworkManager();

	bool Start( int32_t port );
	void Stop();

	//  Private Functions
private:
	//  Network Thread
	static void *ThreadProcStub(void *pObj)
	{
		((CNetworkManager*)pObj)->ThreadProc();
		return (void*)0xDeadDead;
	}
	void ThreadProc();

	//  Private Data
private:
	int         m_Sock;
	int32_t     m_Port;
	pthread_t   m_hThread;
	bool        m_bExitLoop;
	uint8_t     m_TxBuf[4096];
	uint8_t     m_RxBuf[4096];

	CUsbManager *m_pUsbMgr;

};

#endif // CNETWORKMANAGER_H
