/**********************************************************************************************************************
 * \file Cpu0_Main.c
 * \brief Sensor ECU Main
 *********************************************************************************************************************/

#include "Ifx_Types.h"
#include "IfxCpu.h"
#include "IfxScuWdt.h"

#include "MCMCAN.h"
#include "Scheduler.h"
#include "HallSensor.h"
#include "FlashOta.h"
#include "UART_VCOM.h"
#include "IfxAsclin_Asc.h"
#include "SotaUcb.h"
#include <stdio.h>

#define APP_SENSOR_VERSION "1.0.0"

#define SLOW

IFX_ALIGN(4) IfxCpu_syncEvent g_cpuSyncEvent = 0;

/* Watch 확인용 */
volatile uint32_t mainLoopCount = 0U;

boolean g_isGroupBActive = FALSE;

extern IfxAsclin_Asc g_ascPrint;

int _write(int fd, char *buf, int len)
{
    (void)fd;

    if ((buf == NULL_PTR) || (len <= 0))
    {
        return 0;
    }

    for (int i = 0; i < len; i++)
    {
        uint32 timeout = 1000000u;

        while ((IfxAsclin_getTxFifoFillLevel(&MODULE_ASCLIN0) >= 16u) && (timeout-- > 0u))
        {
            __nop();
        }

        if (timeout == 0u)
        {
            break;
        }

        IfxAsclin_writeTxData(&MODULE_ASCLIN0, (uint16)(uint8)buf[i]);
    }

    return len;
}

void core0_main(void)
{
    IfxCpu_enableInterrupts();

    /*
     * Watchdog disable
     */
    IfxScuWdt_disableCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());
    IfxScuWdt_disableSafetyWatchdog(IfxScuWdt_getSafetyWatchdogPassword());

    /*
     * CPU sync
     * CPU1이 준비될 때까지 대기
     */
    IfxCpu_emitEvent(&g_cpuSyncEvent);
    IfxCpu_waitEvent(&g_cpuSyncEvent, 1);

    /*
     * Init
     */
    initMcmcan();
    HallSensor_init();
    initScheduler();

    /* debug용 UART */
    init_UART();

    IfxPort_setPinModeOutput(&MODULE_P00, 5,
                             IfxPort_OutputMode_pushPull,
                             IfxPort_OutputIdx_general);

    g_isGroupBActive = Sota_IsGroupBActive();
    g_isGroupBActive ? printf("Bank B!\r\n") : printf("Bank A!\r\n");

    printf("Sensor ECU v%s\r\n", APP_SENSOR_VERSION);

#ifdef SLOW
    printf("Sensor ECU Main - SLOW\r\n");
#else
    printf("Sensor ECU Main - FAST\r\n");
#endif

    static volatile uint32 ledCounter = 0U;

    while (1)
    {
        mainLoopCount++;

        /*
         * 1ms / 10ms / 100ms 주기 태스크
         *  - 1ms  : HallSensor update, OTA Request 처리, CAN TX service
         *  - 10ms : TofDistanceData 송신 (FEATURE_TOF_SENSOR == 1U)
         *  - 100ms: SpeedData 송신
         */
        Scheduler_run();

        /*
         * RX ISR에서 복사해둔 0x600 UDS OTA Request를
         * main context에서 처리한다.
         * (Flash erase/write는 CPU1 IPC로 분리됨)
         */
        CanIf_ProcessPendingOtaRequest();

        /*
         * OTA pending flag 저장 및 system reset 처리
         */
        FlashOta_Service();

        /*
         * LED toggle - 동작 확인용
         */
#ifdef SLOW
        if (++ledCounter >= 1000000U)
#else
        if (++ledCounter >= 100000U)
#endif
        {
            ledCounter = 0U;
            IfxPort_togglePin(&MODULE_P00, 5);
        }
    }
}
