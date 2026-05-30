/* OtaIpc.c */
#include "OtaIpc.h"

/* cpu0_dlmu에 배치 - SRI 버스로 CPU0/CPU1 양쪽에서 접근 가능 */
__attribute__((section(".data.lmudata")))
OtaIpcReq_t g_otaIpcReq = {0};

__attribute__((section(".data.lmudata")))
OtaIpcRes_t g_otaIpcRes = {0};
