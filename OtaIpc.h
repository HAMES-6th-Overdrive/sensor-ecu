/* OtaIpc.h */
#ifndef OTA_IPC_H
#define OTA_IPC_H

#include "Ifx_Types.h"   /* uint32, uint8 등 TASKING 타입 */
#include "IfxFlash.h"

#define OTA_IPC_CMD_NONE    0U
#define OTA_IPC_CMD_ERASE   1U
#define OTA_IPC_CMD_WRITE   2U

#define OTA_IPC_STATUS_IDLE  0U
#define OTA_IPC_STATUS_BUSY  1U
#define OTA_IPC_STATUS_OK    2U
#define OTA_IPC_STATUS_FAIL  3U

typedef struct
{
    volatile uint32 cmd;
    volatile uint32 addrNc;
    volatile uint32 size;
    volatile uint32 flashType;
    volatile uint8  data[32];
} OtaIpcReq_t;

typedef struct
{
    volatile uint32 status;
    volatile uint32 dmuErr;
} OtaIpcRes_t;

extern OtaIpcReq_t g_otaIpcReq;
extern OtaIpcRes_t g_otaIpcRes;

#endif
