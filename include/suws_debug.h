#ifndef __SUWS_DEBUG__H_
#define __SUWS_DEBUG__H_

#include <common.h>

//想要打印,只需要将 SWS_DEBUG_NO 改为 SWS_DEBUG
#define SWS_DEBUG_NO

#ifdef SWS_DEBUG
#define SUWS_PRINT(format, ...) 	printf(format, ##__VA_ARGS__)
#else
#define SUWS_PRINT(format, ...)
#endif

#endif
