//------------------------------------------------------------------------------
//
//  Copyright (C) 2020 CoAsia Nexell Co., Ltd. All Rights Reserved
//  CoAsia Nexell Co., Ltd. Proprietary & Confidential
//
//  COASIA NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//  AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//  FOR A PARTICULAR PURPOSE.
//
//  Name        : NxDbgMsg.h
//  Description : CoAsia Nexell Debug Message
//  Author      : SeongO-Park ( ray@coasia.com )
//
//------------------------------------------------------------------------------

#ifndef _NXDBGMSG_H_
#define _NXDBGMSG_H_

#define USE_SYSLOG
//#define	USE_PRINTF


#if (defined(USE_SYSLOG) && defined(USE_PRINTF))
#error Please select one USB_SYSLOG or USE_PRINTF !!
#endif

#ifndef LOG_TAG
#define LOG_TAG	"[NXDBGMSG]"
#endif


#ifdef USE_SYSLOG

#ifndef NXLOG_OPTION
#define NXLOG_OPTION    LOG_CONS | LOG_NDELAY | LOG_PID
#endif
#ifndef NXLOG_FACILITY
#define NXLOG_FACILITY  LOG_USER
#endif

#define	EN_SYSLOG_LEVEL_FILTER

extern void NxDbgMsg( int level, const char *tag, const char *fmt, ... );

#endif	//	USE_SYSLOG

enum {
	NXDBG_ERR = 0,
	NXDBG_WARNING,
	NXDBG_NOTICE,
	NXDBG_INFO,
	NXDBG_DEBUG,
	NXDBG_VERBOSE
};

#ifdef USE_PRINTF
extern void NxDbgMsg( int level, const char *tag, const char *fmt, ... );
extern void ChangDbgLevel( int level );
#endif

enum {
	NXLOG_ERR = 0,
	NXLOG_WARNING,
	NXLOG_NOTICE,
	NXLOG_INFO,
	NXLOG_DEBUG,
	NXLOG_VERBOSE
};

#define NXLOGV(...)	NxDbgMsg(NXDBG_VERBOSE, LOG_TAG "/V", ##__VA_ARGS__)
#define NXLOGD(...)	NxDbgMsg(NXDBG_DEBUG  , LOG_TAG "/D", ##__VA_ARGS__)
#define NXLOGI(...)	NxDbgMsg(NXDBG_INFO   , LOG_TAG "/I", ##__VA_ARGS__)
#define NXLOGN(...)	NxDbgMsg(NXDBG_NOTICE , LOG_TAG "/N", ##__VA_ARGS__)
#define NXLOGW(...)	NxDbgMsg(NXDBG_WARNING, LOG_TAG "/W", ##__VA_ARGS__)
#define NXLOGE(...)	NxDbgMsg(NXDBG_ERR    , LOG_TAG "/E", ##__VA_ARGS__)


#endif	//	_NXDBGMSG_H_
