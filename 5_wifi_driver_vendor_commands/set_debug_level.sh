#!/bin/bash

OUI="0x001A12"
SUBCMD="0x1018"

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 [interface] [debug_level (0x00 to 0x05)]"
    exit 1
fi

INTERFACE=$1
DEBUG_LEVEL=$2

# DEBUG LEVEL 0 -> _DRV_NONE_
# DEBUG LEVEL 1 -> _DRV_ALWAYS_
# DEBUG LEVEL 2 -> _DRV_ERR_
# DEBUG LEVEL 3 -> _DRV_WARNING_
# DEBUG LEVEL 4 -> _DRV_INFO_
# DEBUG LEVEL 5 -> _DRV_DEBUG_
if [[ "$DEBUG_LEVEL" =~ ^0x0[0-5]$ ]]; then
    sudo iw dev $INTERFACE vendor send $OUI $SUBCMD $DEBUG_LEVEL
    echo "Debug level set to $DEBUG_LEVEL for interface $INTERFACE"
else
    echo "Invalid debug level value. Please enter a value between 0x00 to 0x05."
    exit 2
fi

