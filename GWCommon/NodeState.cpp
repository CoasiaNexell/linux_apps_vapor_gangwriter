#include "NodeState.h"

static const char *gst_StrState[] = {
	"STATE_NONE",
	"STATE_READY",
	"STATE_DOWNLOADING",
	"STATE_OK",
	"STATE_ERROR",
	"STATE_UNKNOWN",
};

const char *GetModuleString( MODULE_STATE state )
{
	return gst_StrState[state];
}

