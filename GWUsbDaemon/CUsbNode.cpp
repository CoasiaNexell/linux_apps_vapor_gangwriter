#include "CUsbNode.h"
#include "NodeState.h"

#include <usb.h>

#define LOG_TAG "[CUsbNode]"
#include "NxDbgMsg.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

bool CUsbNode::m_bInitUsb = false;


CUsbNode::CUsbNode( uint32_t port, USB_DEVICE_INFO *pInfo )
	: m_Port( port )
	, m_pUsbDevice(pInfo)
{
	m_State = MODULE_STATE::STATE_NONE;
}


int32_t GetFileSsize(FILE *fd)
{
	if( fd )
	{
		int32_t size;
		fseek(fd, 0, SEEK_END);
		size = ftell(fd);
		fseek(fd, 0, SEEK_SET);
		return size;
	}
	return 0;
}

MODULE_STATE CUsbNode::GetNodeState()
{
	CAutoLock lock(m_StateMutex);
	return m_State;
}
void CUsbNode::SetNodeState( MODULE_STATE state )
{
	CAutoLock lock(m_StateMutex);

	if( m_State != state )
	{
		m_State = state;
		SendStatusChangeNotify( m_State );
	}
}


uint32_t CUsbNode::UpdateDeviceNumber( const char *usbDevPath )
{
	char path[256], line[256];
	ssize_t readSize;
	int fd;
	uint32_t retValue = 0xffffffff;
	char *end;

	sprintf( path, "/sys/bus/usb/devices/2-%s/devnum", usbDevPath );
	fd = open( path, O_RDONLY );
	NXLOGV("UpdateDeviceNumber : path = %s\n", path);

	if( -1 != fd )		//	exist
	{
		memset( line, 0, sizeof(line) );
		readSize = read( fd, line, sizeof(line) );
		if( readSize > 0 )
		{
			retValue = strtoul( (const char*)line, &end, 10 );
		}
		close( fd );
	}
	return retValue;
}


void CUsbNode::SetUsbDeviceInfo( USB_DEVICE_INFO *pInfo )
{
	CAutoLock lock(m_StateMutex);
	m_pUsbDevice = pInfo;
	MODULE_STATE oldState = m_State;
	if( m_pUsbDevice )
	{
		if( m_pUsbDevice->idVendor == VENDOR_NEXELL && m_pUsbDevice->idProduct == PRODUCT_4330 )
		{
			m_State = MODULE_STATE::STATE_READY;
		}
		else
		{
			m_State = MODULE_STATE::STATE_NONE;
		}
	}
	if( oldState != m_State )
		SendStatusChangeNotify( m_State );
}

void CUsbNode::SetFileName( const char *filename )
{
	memset( m_FileName, 0, sizeof(m_FileName) );
	strncpy( m_FileName, filename, strlen(filename) );
}

void CUsbNode::RegisterCallback( void (*callback)(void*, uint32_t, uint32_t, void *) , void* pPrivate )
{
	m_Callback = callback;
	m_pPrivate = pPrivate;
}


void CUsbNode::SendMessage(QString msg)
{
	if( m_Callback )
	{
		QString msg2;
		msg2.sprintf( "[%04x:%04x-%d]", m_pUsbDevice->idVendor, m_pUsbDevice->idProduct, m_Port );
		msg2 = msg2+msg;
		m_Callback( m_pPrivate, DOWNLOAD_CB_MSG, 0, &msg2 );
	}
}

void CUsbNode::SendStatusChangeNotify( MODULE_STATE newState )
{
	if( m_Callback )
	{
		QString msg = GetModuleString(newState);
		m_Callback( m_pPrivate, DOWNLOAD_CB_NOTIFY, m_Port, &msg );
	}
}

void CUsbNode::ThreadProc_NXP4330()
{
	NXLOGI("Start Donwloader Thread.\n");
	NXLOGI("PortNumber : %d, DeviceNumber : %d\n", m_pUsbDevice->portNumber, m_pUsbDevice->deviceNumber );
	NXLOGI("File Name  : %s\n", m_FileName );

	QString dbgMsg;
	dbgMsg.sprintf("Start Donwloader Thread.(port=%d)", m_Port);
	SendMessage(dbgMsg);

	struct usb_bus *bus;
	struct usb_device *dev;
	usb_dev_handle *hUsb = nullptr;
	FILE *fd;
	size_t readSize;
	int written;
	uint8_t *pFileBuffer;
	int32_t fileSize;
	uint32_t* header;
	uint32_t deviceNumber = 0xFFFFFFFF;

	if (!m_bInitUsb) {
		usb_init();
		m_bInitUsb = true;
	}

	usb_find_busses();
	usb_find_devices();

	deviceNumber = UpdateDeviceNumber( m_pUsbDevice->devicePath );
	NXLOGD("1. deviceNumber = %d\n", deviceNumber);

	for (bus = usb_get_busses(); bus ; bus = bus->next)
	{
		for (dev = bus->devices; dev ; dev = dev->next)
		{
			if ( (dev->descriptor.idVendor == m_pUsbDevice->idVendor) &&
				 (dev->descriptor.idProduct == m_pUsbDevice->idProduct) &&
				 (dev->devnum == deviceNumber) )
			{
				hUsb = usb_open(dev);
			}
		}
	}

	if( hUsb == nullptr )
	{
		NXLOGE("Cannot found usb device([%d::%d] devnum = %d\n", m_pUsbDevice->idVendor, m_pUsbDevice->idProduct, m_pUsbDevice->deviceNumber);
		goto ERROR_EXIT;
	}

	if (usb_claim_interface(hUsb, 0) < 0)
	{
		NXLOGE("usb_claim_interface() fail!!!\n");
		goto ERROR_EXIT;
	}

	//	Open & Read File
	NXLOGI("=> Open image file\n");
	fd = fopen( "/home/root/bl1-daudio.bin", "rb" );
	if( NULL == fd )
	{
		NXLOGE("Cannot open file (%s)\n", m_FileName);
		goto ERROR_EXIT;
	}

	fileSize = GetFileSsize(fd);
	pFileBuffer = (uint8_t *)malloc(fileSize);

	fread(pFileBuffer, 1, fileSize, fd);

	header =(uint32_t *)pFileBuffer;
	header[17] = fileSize - 512;
	header[18] = 0xFFFF0000;
	header[19] = 0xFFFF0000;

	//	Send 16KB for NXP4330
	written = usb_bulk_write( hUsb, 2, (const char *)pFileBuffer, (16*1024), 1000 );
	NXLOGD( "1. written = %d\n", written );

	if( hUsb )
	{
		usb_release_interface(hUsb, 0);
	}

	if( hUsb )
	{
		usb_close(hUsb);
	}

	hUsb = nullptr;


	usleep(2*1000000);	//	Wait 2 Seconds

	usb_init();
	usb_find_busses();
	usb_find_devices();
	deviceNumber = UpdateDeviceNumber( m_pUsbDevice->devicePath );

	NXLOGD("2. deviceNumber = %d\n", deviceNumber);

	for (bus = usb_get_busses(); bus ; bus = bus->next)
	{
		for (dev = bus->devices; dev ; dev = dev->next)
		{
			if ( (dev->descriptor.idVendor == m_pUsbDevice->idVendor) &&
				 (dev->descriptor.idProduct == m_pUsbDevice->idProduct) &&
				 (dev->devnum == deviceNumber) )
			{
				hUsb = usb_open(dev);
			}
		}
	}

	if( hUsb )
	{
		if (usb_claim_interface(hUsb, 0) < 0)
		{
			NXLOGE("usb_claim_interface() fail!!!\n");
			goto ERROR_EXIT;
		}

		written = usb_bulk_write( hUsb, 2, (const char *)(pFileBuffer + 16*1024), fileSize - (16*1024), 1000 );
		NXLOGD( "2. written = %d\n", written );
	}

	free( pFileBuffer );

	fclose(fd);

	SendMessage("Download Done !!");

	if( hUsb )
	{
		usb_release_interface(hUsb, 0);
	}

	if( hUsb )
	{
		usb_close(hUsb);
	}
	SetNodeState(MODULE_STATE::STATE_OK);
	return;

ERROR_EXIT:
	if( hUsb )
	{
		usb_release_interface(hUsb, 0);
	}

	if( hUsb )
	{
		usb_close(hUsb);
	}

	SetNodeState(MODULE_STATE::STATE_ERROR);
	NXLOGE("Download Error Exit!!!\n");
	SendMessage("Download Error Exit !!!");
}







#if 0
//
//	Receive data using usb_bulk protocol
//
int receive_data(int vid, int pid, unsigned char *data, int size)
{
	int ret = 0;
	usb_dev_handle *dev_handle;

	dev_handle = get_usb_dev_handle(vid, pid);

	if (NULL == dev_handle) {
		printf("Cannot found matching USB device.(vid=0x%04x, pid=0x%04x)\n", vid, pid);
		return -1;
	}

	if (usb_claim_interface(dev_handle, 0) < 0) {
		printf("usb_claim_interface() fail!!!\n");
		usb_close(dev_handle);
		return -1;
	}
	printf("=> try get %d bytes\n", size);

	ret = usb_bulk_read(dev_handle, 1, (void *)data, size, 5 * 1000 * 1000);

	if (ret == size) {
		printf("=> data received. ret:%d, size:%d\n", ret, size);
	} else {
		printf("=> cannot get data!!(ret=%d)\n", ret);
		usb_close(dev_handle);
		return ret;
	}

	usb_release_interface(dev_handle, 0);
	usb_close(dev_handle);

	return 0;
}
#endif
