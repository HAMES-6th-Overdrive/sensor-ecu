/* OtaWorker.c */
#include "OtaIpc.h"
#include "OtaWorker.h"
#include "SensorOtaFlash.h"
#include "IfxCpu.h"
#include "IfxStm.h"

volatile uint32 g_otaWorkerAlive     = 0U;
volatile uint32 g_otaWorkerReqCount  = 0U;
volatile uint32 g_otaWorkerOkCount   = 0U;
volatile uint32 g_otaWorkerFailCount = 0U;

volatile uint32 g_otaEraseStartTick  = 0U;
volatile uint32 g_otaEraseEndTick    = 0U;

void OtaWorker_run(void)
{
    uint32 cmd;

    IfxCpu_setCoreMode(&MODULE_CPU2, IfxCpu_CoreMode_halt);

    g_otaWorkerAlive = 1U;

    for (;;)
    {
        g_otaWorkerAlive++;

        if (g_otaIpcReq.cmd == OTA_IPC_CMD_NONE)
        {
            __nop();
            continue;
        }

        g_otaWorkerReqCount++;
        g_otaIpcRes.status = OTA_IPC_STATUS_BUSY;

        cmd = g_otaIpcReq.cmd;
        __dsync();

        switch (cmd)
        {
            case OTA_IPC_CMD_ERASE:
            {
                boolean ok;

                g_otaEraseStartTick = IfxStm_getLower(&MODULE_STM0);

                ok = SensorOtaFlash_Erase(
                    g_otaIpcReq.addrNc,
                    g_otaIpcReq.size,
                    (IfxFlash_FlashType)g_otaIpcReq.flashType);

                g_otaEraseEndTick = IfxStm_getLower(&MODULE_STM0);

                g_otaIpcRes.dmuErr = MODULE_DMU.HF_ERRSR.U;
                g_otaIpcRes.status = ok ? OTA_IPC_STATUS_OK
                                        : OTA_IPC_STATUS_FAIL;

                if (ok) { g_otaWorkerOkCount++; }
                else    { g_otaWorkerFailCount++; }

                break;
            }
            case OTA_IPC_CMD_WRITE:
            {
                boolean ok;

                ok = SensorOtaFlash_Write(
                    g_otaIpcReq.addrNc,
                    (const uint8 *)g_otaIpcReq.data,
                    32U,
                    (IfxFlash_FlashType)g_otaIpcReq.flashType);

                g_otaIpcRes.dmuErr = MODULE_DMU.HF_ERRSR.U;
                g_otaIpcRes.status = ok ? OTA_IPC_STATUS_OK
                                        : OTA_IPC_STATUS_FAIL;

                if (ok) { g_otaWorkerOkCount++; }
                else    { g_otaWorkerFailCount++; }

                break;
            }
            default:
            {
                g_otaIpcRes.dmuErr = 0U;
                g_otaIpcRes.status = OTA_IPC_STATUS_FAIL;
                g_otaWorkerFailCount++;
                break;
            }
        }

        __dsync();
        g_otaIpcReq.cmd = OTA_IPC_CMD_NONE;
    }
}
