#ifndef CUSBMANAGER_H
#define CUSBMANAGER_H

#include <stdint.h>
#include <pthread.h>
#include "CBaseClass.h"
#include "NodeState.h"
#include <QString>


class CUsbNode;


#define	MAX_NUM_USB_CHILD	(8)
#define	MAX_USB_DEVICES		(32)
#define MAX_BUS_LOCATION    (16)
#define USB_DEVICE_PATH     "/sys/bus/usb/devices/"
#define	USB_TOP_DEVICE		"/sys/bus/usb/devices/2-1"
#define MAX_NUM_USB_HUB		(8)

#define	MAX_SUPPORTED_CLIENTS	(2)

#define	MAX_DEV_PATH		(18)

typedef struct tUSBDeviceInfo	USB_DEVICE_INFO;
struct tUSBDeviceInfo {
	uint32_t			busNumber;
	uint32_t			idVendor;
	uint32_t			idProduct;
	uint32_t			deviceClass;
	uint32_t			deviceNumber;
	uint32_t			maxChild;
	int32_t				portNumber;
	int32_t				logicalPort;
	uint32_t			numberChild;
	char				devicePath[MAX_DEV_PATH];
	USB_DEVICE_INFO		*parent;
	USB_DEVICE_INFO		*lists[MAX_NUM_USB_CHILD];
};

//
//	Description :
//
//
typedef struct tNxUSBInfoList	NX_USB_INFO;		//	for Communication to UI
struct tNxUSBInfoList {
	uint32_t			numPorts;
	uint32_t			numConnectedPorts;
	USB_DEVICE_INFO		*deviceInfo[MAX_SUPPORTED_PORTS];
	CUsbNode			*node[MAX_SUPPORTED_PORTS];
};

typedef struct tClientInfo	NX_CLIENT_INFO;
struct tClientInfo{
	int32_t				valid;
	uint32_t			addr;
};

class CUsbManager
{
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//								Methods                                     //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
public:
	CUsbManager();
	virtual ~CUsbManager();

	void Connect( void *buf, int32_t &size, uint32_t addr );
	void Disconnect( void *buf, int32_t &size, uint32_t addr );
	void Alive();
	void Scan( void *buf, int32_t &size );
	void Start( void *buf, int32_t &size );
	void Reset( void *buf, int32_t &size );

private:
	void UpdateUsbInfo();
	void ScanDevice( const char *path, USB_DEVICE_INFO *info );
	uint32_t GetUint32Value( const char *path, bool inputHex );
	uint32_t GetStringValue( const char *path, char *buffer, int32_t buffSize );
	bool GetDeviceInfo( const char *path, uint32_t &busNumber, uint32_t &idVendor, uint32_t &idProduct, uint32_t &deviceClass, uint32_t &deviceNumber, uint32_t &maxChild, char *devPath );
	void PrintUSBDeviceInfo( USB_DEVICE_INFO *info, bool allFlag );
	USB_DEVICE_INFO *FindNextHub( USB_DEVICE_INFO *pInfo, int32_t index );
	void RemovesbDevice( USB_DEVICE_INFO *devInfo );

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//								Variables									//
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
private:
	USB_DEVICE_INFO		m_USBDeviceInfo;
	NX_USB_INFO			m_USBInfo;
	char				m_TmpPath[2048];
	int32_t				m_PrintDepth;
	int32_t				m_NumDevices;
	USB_DEVICE_INFO		*m_pDeviceLists[MAX_USB_DEVICES];

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//						Notify Sender Thread								//
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
private:
	pthread_t			m_hNotifyThread;
	CNotifyCommandQueue	m_NotifyCmdQueue;
	CSemaphore			m_hSemNotify;

	NX_CLIENT_INFO		m_Clients[MAX_SUPPORTED_CLIENTS];
	void				StartNotifyThread();
	void				NotifyThreadProc();
	static void*		NotifyThreadStub( void *obj )
	{
		((CUsbManager*)obj)->NotifyThreadProc();
		return (void*)0xDeadDead;
	}
	void				SendProgressCommand(QString step, int32_t modules, QString data[]);
	void				SendErrorMessage(QString port, QString msg);
	void				SendMessage(QString level, QString msg);

//////////////////////////////////////////////////////////////////////////////
//																			//
//					Donwloader Callback & Related Functions					//
//																			//
//////////////////////////////////////////////////////////////////////////////
private:
	static void			DownloadCallbackStub( void *pObj, uint32_t type, uint32_t port, void *pData )
	{
		((CUsbManager*)pObj)->DownloadCallback( type, port, pData );
	}
	void DownloadCallback( uint32_t type, uint32_t port, void *pData );

};

#endif // CUSBMANAGER_H
