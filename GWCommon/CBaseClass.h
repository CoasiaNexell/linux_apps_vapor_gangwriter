//------------------------------------------------------------------------------
//
//	Copyright (C) 2020 Nexell Co. All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//  AND	WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//  FOR A PARTICULAR PURPOSE.
//
//	Module		: BaseClasses
//	File		: CBaseClass.h
//	Description	: 
//	Author		: ray@coasia.com
//	Export		:
//	History		:
//
//------------------------------------------------------------------------------

#ifndef __CBASECLASS_H__
#define __CBASECLASS_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include "utils.h"

////////////////////////////////////////////////////////////////////////////////
//
//	NX_CMutex
//
class CMutex{
public:
	CMutex()
	{
		pthread_mutex_init( &m_hMutex, NULL );
	}
	~CMutex()
	{
		pthread_mutex_destroy( &m_hMutex );
	}

public:
	void Lock()
	{
		pthread_mutex_lock( &m_hMutex );
	}
	void Unlock()
	{
		pthread_mutex_unlock( &m_hMutex );
	}

private:
	pthread_mutex_t		m_hMutex;

private:
	CMutex (const CMutex &Ref);
	CMutex &operator=(const CMutex &Ref);
};


////////////////////////////////////////////////////////////////////////////////
//
//	NX_CAutoLock
//
class CAutoLock{
public:
	CAutoLock( CMutex &Lock )
	: m_pLock(Lock)
	{
		m_pLock.Lock();
	}
	~CAutoLock()
	{
		m_pLock.Unlock();
	}

protected:
	CMutex& m_pLock;

private:
	CAutoLock (const CAutoLock &Ref);
	CAutoLock &operator=(CAutoLock &Ref);
};


////////////////////////////////////////////////////////////////////////////////
//
//	NX_CSample
//
class CNotifyCommand
{
public:
	CNotifyCommand()
	{
	}

public:
	bool SetData( void *data, int32_t size )
	{
		if( MAX_DATA_SIZE < size )
			return false;
		memcpy( m_Data, data, size );
		m_AcutualDataSize = size;
		return true;
	}
	void GetData( void **data, int32_t &size )
	{
		*data = m_Data;
		size = m_AcutualDataSize;
	}

private:
	enum {
		MAX_DATA_SIZE = 4096,
	};

	uint8_t			m_Data[MAX_DATA_SIZE];
	int32_t			m_AcutualDataSize;

private:
	CNotifyCommand (const CNotifyCommand &Ref);
	CNotifyCommand &operator=(const CNotifyCommand &Ref);
};


////////////////////////////////////////////////////////////////////////////////
//
//	NX_CSampleQueue
//
class CNotifyCommandQueue
{
public:
	CNotifyCommandQueue( int32_t iMaxQueueCnt = 128 )
	:	m_iHeadIndex	( 0 )
	,	m_iTailIndex	( 0 )
	,	m_iSampleCount	( 0 )
	,	m_iMaxQueueCnt	( iMaxQueueCnt )
	{
		m_iMaxQueueCnt = iMaxQueueCnt;
		m_ppNotifyCommandPool = (CNotifyCommand **)malloc( sizeof(CNotifyCommand*) * m_iMaxQueueCnt );
	}

	virtual ~CNotifyCommandQueue()
	{
		if( m_ppNotifyCommandPool )
			free( m_ppNotifyCommandPool );
	}

public:
	int32_t PushSample( CNotifyCommand *pSample )
	{
		CAutoLock lock( m_hMutex );
		if( m_iMaxQueueCnt <= m_iSampleCount )
			return -1;

		m_ppNotifyCommandPool[m_iTailIndex] = pSample;
		m_iTailIndex = (m_iTailIndex+1) % m_iMaxQueueCnt;
		m_iSampleCount++;

		return 0;
	}

	int32_t PopSample( CNotifyCommand **ppSample )
	{
		CAutoLock lock( m_hMutex );
		*ppSample = NULL;

		if( 0 >= m_iSampleCount )
			return -1;

		*ppSample = m_ppNotifyCommandPool[m_iHeadIndex];
		m_iHeadIndex = (m_iHeadIndex+1) % m_iMaxQueueCnt;
		m_iSampleCount--;

		return 0;
	}

	void Reset( void )
	{
		CAutoLock lock( m_hMutex );
		m_iHeadIndex = 0;
		m_iTailIndex = 0;
		m_iSampleCount = 0;
	}

	int32_t GetSampleCount( void )
	{
		CAutoLock lock( m_hMutex );
		return m_iSampleCount;
	}

private:
	CNotifyCommand	**m_ppNotifyCommandPool;
	CMutex			m_hMutex;
	int32_t			m_iHeadIndex, m_iTailIndex, m_iSampleCount;
	int32_t 		m_iMaxQueueCnt;

private:
	CNotifyCommandQueue (const CNotifyCommandQueue &Ref);
	CNotifyCommandQueue &operator=(const CNotifyCommandQueue &Ref);
};



////////////////////////////////////////////////////////////////////////////////
//
//	CSemaphore
//
class CSemaphore{
public:
	CSemaphore()
	: m_iValue	( 1 )
	, m_iMax	( 1 )
	, m_iInit	( 0 )
	, m_bReset	( false )
	{
		pthread_cond_init ( &m_hCond,  NULL );
		pthread_mutex_init( &m_hMutex, NULL );
	}

	CSemaphore( int32_t iMax, int32_t iInit )
	: m_iValue	( iInit )
	, m_iMax	( iMax )
	, m_iInit	( iInit )
	, m_bReset	( false )
	{
		pthread_cond_init ( &m_hCond,  NULL );
		pthread_mutex_init( &m_hMutex, NULL );
	}

	~CSemaphore()
	{
		ResetSignal();
		pthread_cond_destroy( &m_hCond );
		pthread_mutex_destroy( &m_hMutex );
	}

public:
	int32_t Post()
	{
		int32_t iRet = 0;
		pthread_mutex_lock( &m_hMutex );
		if( m_iValue >= m_iMax ) {
			pthread_mutex_unlock( &m_hMutex );
			return -1;
		}

		m_iValue ++;
		pthread_cond_signal ( &m_hCond );
		if( m_bReset || m_iValue<=0 ){
			iRet = -1;
		}
		pthread_mutex_unlock( &m_hMutex );
		return iRet;
	}

	int32_t Pend( int32_t nSec )
	{
		int32_t iRet = 0;
		pthread_mutex_lock( &m_hMutex );
		if( m_iValue == 0 && !m_bReset ){
			timespec time;
			int32_t iErr;
			do{
				clock_gettime(CLOCK_REALTIME, &time);
				if( time.tv_nsec > (1000000 - nSec) ){
					time.tv_sec += 1;
					time.tv_nsec -= (1000000 - nSec);
				}else{
					time.tv_nsec += nSec;
				}
				iErr = pthread_cond_timedwait( &m_hCond, &m_hMutex, &time );

				if( m_bReset == true ){
					iRet = -1;
					break;
				}
			}while( ETIMEDOUT == iErr );
			m_iValue --;
		}else if( m_iValue < 0 || m_bReset ){
			iRet = -1;
		}else{
			m_iValue --;
			iRet = 0;
		}
		pthread_mutex_unlock( &m_hMutex );
		return iRet;
	}

	void ResetSignal()
	{
		pthread_mutex_lock ( &m_hMutex );
		m_bReset = true;
		for (int32_t i = 0; i<m_iMax; i++){
			pthread_cond_broadcast( &m_hCond );
		}
		pthread_mutex_unlock( &m_hMutex );
	}

	void ResetValue()
	{
		pthread_mutex_lock( &m_hMutex );
		m_iValue = m_iInit;
		m_bReset = false;
		pthread_mutex_unlock( &m_hMutex );
	}

	int32_t GetValue()
	{
		pthread_mutex_lock( &m_hMutex );
		int32_t iValue = m_iValue;
		pthread_mutex_unlock( &m_hMutex );
		return iValue;
	}

private:
	enum { MAX_SEM_VALUE = 1024 };

	pthread_cond_t  m_hCond;
	pthread_mutex_t m_hMutex;
	int32_t			m_iValue;
	int32_t			m_iMax;
	int32_t			m_iInit;
	int32_t			m_bReset;

private:
	CSemaphore (const CSemaphore &Ref);
	CSemaphore &operator=(const CSemaphore &Ref);
};

////////////////////////////////////////////////////////////////////////////////
//
//	CNotifyReceiver
//
class CNotifyReceiver {
public:
	CNotifyReceiver(){};

	bool StartNotifyReceiver( int32_t port, void (*callback)(void *, void*, int32_t), void *pPrivate )
	{
		m_Port = port;
		m_CallbackFunc = callback;
		m_Private = pPrivate;
		if( 0 != pthread_create( &m_hThread, nullptr, ThreadStub, this ) )
			return false;
		return true;
	}

private:
	static void *ThreadStub( void *pObj )
	{
		((CNotifyReceiver *)(pObj))->ThreadProc();
		return (void*)0xDeadDead;
	}

	void ThreadProc()
	{
		int sock;
		struct sockaddr_in addr;
		struct sockaddr_in clientAddr;
		int recvLen;
		socklen_t addrLen;
		m_bExitLoop = false;
		socklen_t opt;

		if( 0 > (sock = socket(AF_INET, SOCK_DGRAM, 0)) )
		{
			goto ERROR_EXIT;
		}

		memset( &addr, 0, sizeof(addr) );
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(m_Port);

		opt = 1;
		if(0 > setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt) ) )
		{
			goto ERROR_EXIT;
		}

		if(0 > setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt) ) )
		{
			goto ERROR_EXIT;
		}

		if( 0 > bind(sock, (struct sockaddr*)&addr, sizeof(addr)) )
		{
			goto ERROR_EXIT;
		}

		while ( !m_bExitLoop )
		{
			//  poll
			addrLen = sizeof(clientAddr);
			recvLen = recvfrom( sock, (void*)m_RxBuf, sizeof(m_RxBuf), 0, (struct sockaddr*)&clientAddr, &addrLen );
			//  Don't delete : QJsonDument::fromJson find EOS or null charactor.
			m_RxBuf[recvLen] = 0;

			if( m_CallbackFunc )
			{
				m_CallbackFunc(m_Private, m_RxBuf, recvLen);
			}
		}
	ERROR_EXIT:
		if( sock > 0 )
			close(sock);

	}

	pthread_t		m_hThread;
	int32_t			m_Port;
	bool			m_bExitLoop;
	uint8_t			m_RxBuf[4096];
	void			*m_Private;
	void (*m_CallbackFunc)(void*, void*, int32_t);
};

////////////////////////////////////////////////////////////////////////////////
//
//	CNotifyReceiver
//
class CAliveSender{
public:
	CAliveSender()
		: m_CallbackFunc( nullptr )
		, m_Private( nullptr )
		, m_Interval( 500 )
	{

	};

	bool StartAlvieSender( void (*callback)(void *), void *pPrivate, uint32_t interval )
	{
		m_CallbackFunc = callback;
		m_Private = pPrivate;
		m_Interval =interval;
		if( 0 != pthread_create( &m_hThread, nullptr, ThreadStub, this ) )
		{
			return false;
		}
		return true;
	}

private:
	static void *ThreadStub( void *pObj )
	{
		((CAliveSender *)(pObj))->ThreadProc();
		return (void*)0xDeadDead;
	}

	void ThreadProc()
	{
		m_bExitLoop = false;
		while ( !m_bExitLoop )
		{
			usleep( m_Interval * 1000 );
			if( m_CallbackFunc )
			{
				m_CallbackFunc(m_Private);
			}
		}
	}

	pthread_t		m_hThread;
	bool			m_bExitLoop;
	//	callback function : status gat callback function
	void (*m_CallbackFunc)(void*);
	void			*m_Private;
	uint32_t		m_Interval;
};

#endif	// __CBASECLASS_H__
