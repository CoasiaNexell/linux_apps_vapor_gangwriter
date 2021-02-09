#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cusbmanager.h"
#include "jcommand.h"
#include "CUsbNode.h"

#define LOG_TAG "[USB MANAGER]"
#include "NxDbgMsg.h"

CUsbManager::CUsbManager()
{
	memset( &m_USBDeviceInfo, 0, sizeof(m_USBDeviceInfo) );
	memset( &m_USBInfo, 0, sizeof(m_USBInfo) );
	m_PrintDepth = 0;
	m_USBInfo.numPorts = 8;
	m_USBInfo.numConnectedPorts = 0;

	for( int32_t i=0 ; i<MAX_SUPPORTED_CLIENTS ; i++ )
		m_Clients[i].valid = 0;


	for( int32_t i=0 ; i<m_USBInfo.numPorts ; i++ )
	{
		m_USBInfo.node[i] = new CUsbNode( i+1, nullptr );
		m_USBInfo.node[i]->RegisterCallback( DownloadCallbackStub, this );
	}

	StartNotifyThread();
}

CUsbManager::~CUsbManager()
{
}


void CUsbManager::RemovesbDevice( USB_DEVICE_INFO *devInfo )
{
	for( uint32_t i=0 ; i<devInfo->numberChild ; i++ )
	{
		RemovesbDevice( devInfo->lists[i] );
	}
	devInfo->numberChild = 0;
	if( devInfo->parent )
		free( devInfo );
}



uint32_t CUsbManager::GetUint32Value( const char *path, bool inputHex )
{
	char line[1024];
	ssize_t readSize;
	int fd = open( path, O_RDONLY );
	uint32_t retValue = 0xffffffff;
	char *end;
	if( -1 != fd )		//	exist
	{
		memset( line, 0, sizeof(line) );
		readSize = read( fd, line, sizeof(line) );
		if( readSize > 0 )
		{
			if( inputHex )
			{
				retValue = strtoul( (const char*)line, &end, 16 );
			}
			else
			{
				retValue = strtoul( (const char*)line, &end, 10 );
			}
		}
		close( fd );
	}
	return retValue;
}

void RemoveLineFeed(char *buffer, int32_t bufferSize)
{
	for( int32_t i=0 ; i< bufferSize ; i++ )
	{
		if( buffer[i] == 0x0a )
			buffer[i] = 0;
	}
}

uint32_t CUsbManager::GetStringValue( const char *path, char *buffer, int32_t buffSize )
{
	ssize_t readSize;
	int fd = open( path, O_RDONLY );
	uint32_t retValue = 0xffffffff;
	if( -1 != fd )		//	exist
	{
		memset( buffer, 0, buffSize );
		readSize = read( fd, buffer, buffSize );
		// NXLOGD("%s() readSize = %d, %s\n", __FUNCTION__, readSize, buffer);
		// HexDump(buffer, readSize);
		close( fd );
	}
	return retValue;
}

bool CUsbManager::GetDeviceInfo( const char *path, uint32_t &busNumber, uint32_t &idVendor, uint32_t &idProduct, uint32_t &deviceClass, uint32_t &deviceNumber, uint32_t &maxChild, char *devPath )
{
	//
	//	Get bus Nmber
	//
	sprintf( m_TmpPath, "%s/busnum", path );
	busNumber = GetUint32Value( m_TmpPath, true );


	//
	//	Get Vendor ID
	//
	sprintf( m_TmpPath, "%s/idVendor", path );
	idVendor = GetUint32Value( m_TmpPath, true );

	//
	//	Get Product ID
	//
	sprintf( m_TmpPath, "%s/idProduct", path );
	idProduct = GetUint32Value( m_TmpPath, true );

	//
	//	Get Device Class
	//
	sprintf( m_TmpPath, "%s/bDeviceClass", path );
	deviceClass = GetUint32Value( m_TmpPath, true );

	//
	//	Get Device Number
	//
	sprintf( m_TmpPath, "%s/devnum", path );
	deviceNumber = GetUint32Value( m_TmpPath, false );

	//
	//	Get max child
	//
	sprintf( m_TmpPath, "%s/maxchild", path );
	maxChild = GetUint32Value( m_TmpPath, false );

	//
	//	Get device Path
	//
	sprintf( m_TmpPath, "%s/devpath", path );
	GetStringValue( m_TmpPath, devPath, MAX_DEV_PATH-1 );
	RemoveLineFeed(devPath, MAX_DEV_PATH-1);

	if( (busNumber == 0xffffffff) || (idVendor == 0xffffffff) || (idProduct == 0xffffffff) ||
		(deviceClass == 0xffffffff) || (deviceNumber == 0xffffffff) || (maxChild == 0xffffffff) )
	{
		return false;
	}


	//printf("BUS %d [%04x:%04x] : class (%d), number(%d)\n", busNumber, idVendor, idProduct, deviceClass, deviceNumber);

	return true;
}

void CUsbManager::PrintUSBDeviceInfo( USB_DEVICE_INFO *info, bool allFlag )
{
	for( uint32_t i=0 ; i<info->numberChild ; i++ )
	{
		USB_DEVICE_INFO *subDevice = info->lists[i];
		for( int32_t i=0 ; i<m_PrintDepth ; i++ )
			printf("  ");
		printf("BUS %d [%04x:%04x] : port(%d) class (%d), number(%d), child(%d/%d), devPath(%s)\n",
				subDevice->busNumber, subDevice->idVendor, subDevice->idProduct, subDevice->portNumber, subDevice->deviceClass, subDevice->deviceNumber,
				subDevice->numberChild, subDevice->maxChild, subDevice->devicePath);
		if( allFlag )
		{
			m_PrintDepth ++;
			PrintUSBDeviceInfo( subDevice, allFlag );
			m_PrintDepth --;
		}
	}
}

//	Genesys 4Port USB Hub Info
void CUsbManager::ScanDevice( const char *path, USB_DEVICE_INFO *info )
{
	struct stat st;
	char devPath[18];
	for( int32_t i=1 ; i<=MAX_NUM_USB_HUB ; i++ )
	{
		sprintf(m_TmpPath, "%s.%d", path, i );

		if( 0 == stat(m_TmpPath, &st) )
		{
			uint32_t busNumber, idVendor, idProduct, deviceClass, deviceNumber, maxChild;
			char *subPath = strdup(m_TmpPath);

			//	deviceClass == 9 (Hub) : Search Sub Devices
			memset( devPath, 0, sizeof(devPath) );
			if( GetDeviceInfo( subPath, busNumber, idVendor, idProduct, deviceClass, deviceNumber, maxChild, devPath ) )
			{
				if( info->numberChild < MAX_NUM_USB_CHILD )
				{
					USB_DEVICE_INFO *device = (USB_DEVICE_INFO *)malloc( sizeof(USB_DEVICE_INFO) );
					memset( device, 0, sizeof(USB_DEVICE_INFO) );
					device->busNumber    = busNumber;
					device->idVendor     = idVendor;
					device->idProduct    = idProduct;
					device->deviceClass  = deviceClass;
					device->deviceNumber = deviceNumber;
					device->portNumber   = i;
					device->logicalPort  = 0;
					device->numberChild  = 0;
					device->maxChild     = maxChild;
					strncpy( device->devicePath, devPath, strlen(devPath) );
					device->devicePath[strlen(devPath)] = 0;

					info->lists[info->numberChild] = device;
					info->numberChild ++;

					m_pDeviceLists[m_NumDevices] = device;
					m_NumDevices ++;

					//	If current device is hub class, scan sub devices.
					if( deviceClass == 9 )
					{
						device->parent = info;
						ScanDevice( subPath, device );
					}
				}
				else
				{
					printf("~~~~~~~~~~~~~~ Warning ~~~~~~~~~~~~~\n");
				}
			}
			free( subPath );
		}
	}
}

void CUsbManager::UpdateUsbInfo()
{
	int32_t offset = 0;
	int32_t logicalPortNumber;

	m_USBInfo.numPorts = 0;
	for( int32_t i=0 ; i<m_NumDevices ; i++ )
	{
		m_USBInfo.deviceInfo[i] = 0;
	}

	//	Find Hub & His Child
	for( int i=0 ; i<m_NumDevices ; i++ )
	{
		if( (m_pDeviceLists[i]->idProduct == PRODUCT_GENESYS) &&
			(m_pDeviceLists[i]->idVendor  == VENDOR_GENESYS) )
		{
			logicalPortNumber = offset;
			for( uint32_t j=0 ; j<m_pDeviceLists[i]->numberChild ; j++ )
			{
				logicalPortNumber += m_pDeviceLists[i]->lists[j]->portNumber - 1;
				m_pDeviceLists[i]->lists[j]->logicalPort = logicalPortNumber;

				if( m_pDeviceLists[i]->lists[j]->idVendor == VENDOR_NEXELL &&
					m_pDeviceLists[i]->lists[j]->idProduct == PRODUCT_4330 )
				m_USBInfo.deviceInfo[logicalPortNumber] = m_pDeviceLists[i]->lists[j];
				m_USBInfo.numConnectedPorts ++;
			}
			m_USBInfo.numPorts += m_pDeviceLists[i]->maxChild;
			offset += m_pDeviceLists[i]->maxChild;
		}
	}
	//printf("m_USBInfo.numPorts = %d, m_USBInfo.numConnectedPorts = %d\n", m_USBInfo.numPorts, m_USBInfo.numConnectedPorts);
}



//////////////////////////////////////////////////////////////////////////////////
//																				//
//							Public Methods API									//
//																				//
//////////////////////////////////////////////////////////////////////////////////

void CUsbManager::Connect( void *buf, int32_t &size, uint32_t addr )
{
	int32_t resLen;
	QString status = "ok";
	CJCmdResponse res;
	QJsonDocument resDoc = res.Connect( status );

	//  Respons Data
	resLen = resDoc.toJson().size();
	memcpy( buf, &resLen, sizeof(resLen) );
	memcpy( ((uint8_t*)buf)+4, resDoc.toJson().data(), resLen );
	size = resLen +4;

	bool needReg = true;
	for( int32_t i=0 ; i<MAX_SUPPORTED_CLIENTS; i++ )
	{
		if( m_Clients[i].valid && addr == m_Clients[i].addr )
		{
			needReg = false;
			break;
		}
	}
	if( needReg )
	{
		for( int32_t i=0 ; i<MAX_SUPPORTED_CLIENTS; i++ )
		{
			if( !m_Clients[i].valid )
			{
				m_Clients[i].addr = addr;
				m_Clients[i].valid = 1;
				NXLOGI("Register address(%08x)\n", addr);
				break;
			}
		}
	}
	SendMessage("debug", "connect");
}

void CUsbManager::Disconnect( void *buf, int32_t &size, uint32_t addr )
{
	int32_t resLen;
	QString status = "ok";
	CJCmdResponse res;
	QJsonDocument resDoc = res.Disconnect( status );

	//  Respons Data

	resLen = resDoc.toJson().size();
	memcpy( buf, &resLen, sizeof(resLen) );
	memcpy( ((uint8_t*)buf)+4, resDoc.toJson().data(), resLen );
	size = resLen +4;
	SendMessage("debug", "disconnect");

	for( int32_t i=0 ; i<MAX_SUPPORTED_CLIENTS; i++ )
	{
		if( m_Clients[i].valid && m_Clients[i].addr == addr )
		{
			m_Clients[i].valid = 0;
			NXLOGI("[Server] Unregister address(%08x)\n", addr);
		}
	}

}

void CUsbManager::Alive()
{
	SendMessage("debug", "alive");
}


void CUsbManager::Scan( void *buf, int32_t &size )
{
	int32_t resLen;
	CJCmdResponse res;
	QString strResult[16];
	QJsonDocument resDoc;

	m_USBDeviceInfo.parent = 0;
	m_NumDevices = 0;
	//	Clear Device Information
	RemovesbDevice(&m_USBDeviceInfo);
	//	Scan & Update Device Information
	ScanDevice(USB_TOP_DEVICE , &m_USBDeviceInfo);
	//	Update Usb Information
	UpdateUsbInfo();
	//	PrintUSBDeviceInfo(&m_USBDeviceInfo, true);

	//	make scan response JSON command
	for( uint32_t i=0; i<m_USBInfo.numPorts; i++ )
	{
		strResult[i] = QString::asprintf("%s", m_USBInfo.deviceInfo[i] ? "ok" : "none");
	}
	resDoc = res.Scan(m_USBInfo.numPorts, strResult);

	//  Respons Data
	resLen = resDoc.toJson().size();
	memcpy( buf, &resLen, sizeof(resLen) );
	memcpy( ((uint8_t*)buf)+4, resDoc.toJson().data(), resLen );
	size = resLen +4;
	SendMessage("debug", "scan");
}


void CUsbManager::Start( void *buf, int32_t &size )
{
	QString strResult[16];

	//	Make Download Thread & Resutl Data
	for( uint32_t i=0 ; i<m_USBInfo.numPorts ; i++ )
	{
		CUsbNode *node = m_USBInfo.node[i];
		if( m_USBInfo.deviceInfo[i] )
		{
			node->SetUsbDeviceInfo( m_USBInfo.deviceInfo[i] );
			node->SetFileName("/home/root/test.bin");
			node->SetNodeState(STATE_DOWNLOADING);
			if( node->StartDownload() )
			{
				strResult[i] = "ok";
			}
			else
			{
				strResult[i] = "fail";
				m_USBInfo.node[i]->SetNodeState(STATE_ERROR);
			}
		}
		else
		{
			strResult[i] = "none";
			node->SetNodeState(STATE_NONE);
		}
	}

	//	Make Response Data
	{
		int32_t resLen;
		CJCmdResponse res;
		QString port = "all";
		QJsonDocument resDoc = res.Start(port, 8, strResult);
		//  Respons Data
		resLen = resDoc.toJson().size();
		memcpy( buf, &resLen, sizeof(resLen) );
		memcpy( ((uint8_t*)buf)+4, resDoc.toJson().data(), resLen );
		size = resLen + 4;
		SendMessage("debug", "start");
	}
}

void CUsbManager::Reset( void *buf, int32_t &size )
{
	int32_t resLen;
	CJCmdResponse res;
	QString port = "all";
	QJsonDocument resDoc = res.Reset(port, "ok");
	//  Respons Data

	resLen = resDoc.toJson().size();
	memcpy( buf, &resLen, sizeof(resLen) );
	memcpy( ((uint8_t*)buf)+4, resDoc.toJson().data(), resLen );
	size = resLen + 4;
	SendMessage("debug", "reset");
}


//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//						Notify Sender Thread								//
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
void CUsbManager::StartNotifyThread()
{
	if( 0 != pthread_create(&m_hNotifyThread, 0, NotifyThreadStub, this ) )
	{
		NXLOGE("pthread_create failed!!!\n");
	}
}

void CUsbManager::NotifyThreadProc()
{
	NXLOGV("[CUsbManager] Start Notify Thread Procedure!!!\n");
	CNotifyCommand *pCmd;
	while(1)
	{
		if( 0 > m_hSemNotify.Pend(1) )
		{
			break;
		}
		pCmd = nullptr;
		m_NotifyCmdQueue.PopSample(&pCmd);
		if( pCmd )
		{
			// Process Command
			void *pData;
			int32_t size;
			pCmd->GetData( &pData, size );
			for( int32_t i=0 ; i<MAX_SUPPORTED_CLIENTS; i++ )
			{
				if( m_Clients[i].valid )
				{
					int sock = socket(AF_INET, SOCK_DGRAM, 0);
					struct sockaddr_in svrAddr;
					memset(&svrAddr, 0x00, sizeof(svrAddr));
					svrAddr.sin_family = AF_INET;
					svrAddr.sin_addr.s_addr = m_Clients[i].addr;
					svrAddr.sin_port = htons( 5060 );
					socklen_t addrLen = sizeof(svrAddr);
					sendto(sock, pData, size, 0, (struct sockaddr*)&svrAddr, addrLen);
					close(sock);
				}
			}
			delete pCmd;
		}
	}
	NXLOGV("[CUsbManager] End Notify Thread Procedure!!!\n");
}

void CUsbManager::SendProgressCommand(QString step, int32_t modules, QString data[])
{
	CJNotify notify;
	QJsonDocument doc;
	doc = notify.Pogress( step, modules, data );
	CNotifyCommand *cmd = new CNotifyCommand();
	cmd->SetData(doc.toJson().data(), doc.toJson().size());
	m_NotifyCmdQueue.PushSample(cmd);
	m_hSemNotify.Post();
}

void CUsbManager::SendErrorMessage(QString port, QString msg)
{
	CJNotify notify;
	QJsonDocument doc;
	doc = notify.Error( port, msg );
	CNotifyCommand *cmd = new CNotifyCommand();
	cmd->SetData(doc.toJson().data(), doc.toJson().size());
	m_NotifyCmdQueue.PushSample(cmd);
	m_hSemNotify.Post();
}

void CUsbManager::SendMessage(QString level, QString msg)
{
	CJNotify notify;
	QJsonDocument doc;
	doc = notify.Message( level, msg );
	CNotifyCommand *cmd = new CNotifyCommand();
	cmd->SetData(doc.toJson().data(), doc.toJson().size());
	m_NotifyCmdQueue.PushSample(cmd);
	m_hSemNotify.Post();
}


void CUsbManager::DownloadCallback( uint32_t type, uint32_t port, void *pData )
{
	if( type == DOWNLOAD_CB_MSG )
	{
		QString *msg =(QString *)pData;
		CJNotify notify;
		QJsonDocument doc;
		QString level = "information";
		doc = notify.Message( level, *msg );
		CNotifyCommand *cmd = new CNotifyCommand();
		cmd->SetData(doc.toJson().data(), doc.toJson().size());
		m_NotifyCmdQueue.PushSample(cmd);
		m_hSemNotify.Post();
	}
	else if( type == DOWNLOAD_CB_NOTIFY )
	{
		CJNotify notify;
		QString *msg =(QString *)pData;
		QJsonDocument doc;
		QString strPort;
		strPort.sprintf("%d", port);
		doc = notify.StatusChange( strPort, *msg );
		CNotifyCommand *cmd = new CNotifyCommand();
		cmd->SetData(doc.toJson().data(), doc.toJson().size());
		m_NotifyCmdQueue.PushSample(cmd);
		m_hSemNotify.Post();
	}
}
