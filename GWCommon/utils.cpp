#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "NxDbgMsg.h"

void HexDump( const void *data, int32_t size )
{
	int32_t i=0, offset = 0;
	char tmp[32];
	static char lineBuf[1024];
	const uint8_t *_data = (const uint8_t*)data;
	while( offset < size )
	{
		sprintf( lineBuf, "%08lx :  ", (unsigned long)offset );
		for( i=0 ; i<16 ; ++i )
		{
			if( i == 8 ){
				strcat( lineBuf, " " );
			}
			if( offset+i >= size )
			{
				strcat( lineBuf, "   " );
			}
			else{
				sprintf(tmp, "%02x ", _data[offset+i]);
				strcat( lineBuf, tmp );
			}
		}
		strcat( lineBuf, "   " );

		//     Add ACSII A~Z, & Number & String
		for( i=0 ; i<16 ; ++i )
		{
			if( offset+i >= size )
			{
				break;
			}
			else{
				if( isprint(_data[offset+i]) )
				{
					sprintf(tmp, "%c", _data[offset+i]);
					strcat(lineBuf, tmp);
				}
				else
				{
					strcat( lineBuf, "." );
				}
			}
		}

		strcat(lineBuf, "\n");
		printf( "%s", lineBuf );
		offset += 16;
	}
}

#define MAXLINE	(256)
int AppIsRunning(const char *appName)
{
	FILE *fp;
	int ret = 0;
	char cmd[128];
	char buff[MAXLINE];
	sprintf( cmd, "pidof %s", appName );
	fp = popen(cmd, "r");
	if( fgets(buff, MAXLINE, fp) != NULL )
	{
		ret = atoi(buff);
	}
	pclose(fp);
	return ret;
}
