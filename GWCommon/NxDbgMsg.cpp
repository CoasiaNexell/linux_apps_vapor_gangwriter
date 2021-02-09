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
//  Name        : NxDbgMsg.cpp
//  Description : CoAsia Nexell Debug Message
//  Author      : SeongO-Park ( ray@coasia.com )
//
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdarg.h>
#include "NxDbgMsg.h"

static int gst_LogLvel = NXDBG_DEBUG;

#ifdef USE_SYSLOG
#include <syslog.h>
static int gst_IsInitSyslog = 0;
void NxDbgMsg( int level, const char *tag, const char *fmt, ... )
{

	(void)level;
	va_list start;
	char linebuf[1024];

#ifdef EN_SYSLOG_LEVEL_FILTER
	if( level > gst_LogLvel )
		return;
#endif


	if( !gst_IsInitSyslog )
	{
		openlog(nullptr, NXLOG_OPTION, NXLOG_FACILITY);
		gst_IsInitSyslog = 1;
	}
	va_start(start, fmt);
	vsprintf( linebuf, fmt, start );
	syslog(LOG_DEBUG, "%s %s", tag, linebuf);
	va_end(start);
}
#endif	//	USE_SYSLOG



#ifdef USE_PRINTF
void NxDbgMsg( int level, const char *tag, const char *fmt, ... )
{
	char linebuf[1024];
	if( level <= gst_LogLvel )
	{
		va_list start;
		va_start(start, fmt);
		vsprintf(linebuf, fmt, start);
		printf("%s %s", tag, linebuf);
		va_end(start);
	}
}

#endif	//	USE_PRINTF

void ChangDbgLevel( int level )
{
	gst_LogLvel = level;
}
