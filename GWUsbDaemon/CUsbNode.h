#ifndef _CUSBNODE_H_
#define _CUSBNODE_H_

#include <pthread.h>
#include <vector>
#include "cusbmanager.h"
#include "NodeState.h"

using namespace std;


#define DOWNLOAD_CB_MSG		0
#define DOWNLOAD_CB_NOTIFY	1


class CUsbNode{
public:
	CUsbNode( uint32_t port, USB_DEVICE_INFO *pInfo = nullptr );

	MODULE_STATE GetNodeState();
	void SetNodeState( MODULE_STATE state );
	uint32_t UpdateDeviceNumber( const char *usbDevPath );
	void SetUsbDeviceInfo( USB_DEVICE_INFO *pInfo );
	void SetFileName( const char *filename );
	void RegisterCallback( void (*callback)(void*, uint32_t, uint32_t, void *) , void* pPrivate );

	bool StartDownload()
	{
		if( m_pUsbDevice == nullptr )
			return false;

		//	careate auto detech thread
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		if( 0 != pthread_create( &m_hThread, &attr, ThreadProcSub, this ) )
		{
			return false;
		}
		pthread_attr_destroy(&attr);
		return true;
	}


private:
	void SendStatusChangeNotify( MODULE_STATE newState );
	void SendMessage(QString msg);

private:
	pthread_t			m_hThread;
	static void *ThreadProcSub( void *pObj )
	{
		((CUsbNode *)pObj)->ThreadProc_NXP4330();
		return (void*)0xDeadDead;
	}
	void ThreadProc_NXP4330();

private:
	uint32_t			m_Port;			//	Logical Port
	MODULE_STATE		m_State;
	USB_DEVICE_INFO		*m_pUsbDevice;
	char				m_FileName[1024];
	static bool			m_bInitUsb;
	uint8_t				m_TxBuf[4*1024];

	void				(*m_Callback)(void *, uint32_t, uint32_t, void*);
	void				*m_pPrivate;

	CMutex				m_StateMutex;
};

#endif // _CUSBNODE_H_
