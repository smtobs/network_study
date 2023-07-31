#ifndef IE_H
#define IE_H

#define IE_SSID                0
#define IE_SUPPORTED_RATES     1
#define IE_DTIM                5
#define IE_COUNTRY             7
#define IE_RSN                 48

#define IE_TYPE                0
#define IE_LEN                 1
#define IE_VALUE               2

#include <stdio.h>
#include <stdint.h>
#include <string.h>

void ie_handler(char *ie, int ie_len);

#endif
