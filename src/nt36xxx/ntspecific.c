#include <Cross Platform Shim\compat.h>
#include <internal.h>
#include <spb.h>
#include <report.h>
#include <nt36xxx\ntinternal.h>
#include <nt36xxx\ntspecific.h>
#include <ntinternal.tmh>
#include <wdm.h>
#include <gpio.h>
#include <GpioClx.h>



NVT_FWInfo_t fwInfo;
PNV36XXX_MemMap_t memMap = NULL;
UCHAR bTouchIsAwake = FALSE;

static const struct _NV36XXX_MemMap_t NT36526_memory_map = {
    .EVENT_BUF_ADDR = 0x22D00,
    .RAW_PIPE0_ADDR = 0x24000,
    .RAW_PIPE1_ADDR = 0x24000,
    .BASELINE_ADDR = 0x21758,
    .BASELINE_BTN_ADDR = 0,
    .DIFF_PIPE0_ADDR = 0x20AB0,
    .DIFF_PIPE1_ADDR = 0x24AB0,
    .RAW_BTN_PIPE0_ADDR = 0,
    .RAW_BTN_PIPE1_ADDR = 0,
    .DIFF_BTN_PIPE0_ADDR = 0,
    .DIFF_BTN_PIPE1_ADDR = 0,
    .READ_FLASH_CHECKSUM_ADDR = 0x24000,
    .RW_FLASH_DATA_ADDR = 0x24002,
};

static const struct _NV36XXX_MemMap_t NT36675_memory_map = {
    .EVENT_BUF_ADDR = 0x22D00,
    .RAW_PIPE0_ADDR = 0x24000,
    .RAW_PIPE1_ADDR = 0x24000,
    .BASELINE_ADDR = 0x21B90,
    .BASELINE_BTN_ADDR = 0,
    .DIFF_PIPE0_ADDR = 0x20C60,
    .DIFF_PIPE1_ADDR = 0x24C60,
    .RAW_BTN_PIPE0_ADDR = 0,
    .RAW_BTN_PIPE1_ADDR = 0,
    .DIFF_BTN_PIPE0_ADDR = 0,
    .DIFF_BTN_PIPE1_ADDR = 0,
    .READ_FLASH_CHECKSUM_ADDR = 0x24000,
    .RW_FLASH_DATA_ADDR = 0x24002,
};

static const struct _NV36XXX_MemMap_t NT36672A_memory_map = {
    .EVENT_BUF_ADDR = 0x21C00,
    .RAW_PIPE0_ADDR = 0x20000,
    .RAW_PIPE1_ADDR = 0x23000,
    .BASELINE_ADDR = 0x20BFC,
    .BASELINE_BTN_ADDR = 0x23BFC,
    .DIFF_PIPE0_ADDR = 0x206DC,
    .DIFF_PIPE1_ADDR = 0x236DC,
    .RAW_BTN_PIPE0_ADDR = 0x20510,
    .RAW_BTN_PIPE1_ADDR = 0x23510,
    .DIFF_BTN_PIPE0_ADDR = 0x20BF0,
    .DIFF_BTN_PIPE1_ADDR = 0x23BF0,
    .READ_FLASH_CHECKSUM_ADDR = 0x24000,
    .RW_FLASH_DATA_ADDR = 0x24002,
};

static const struct _NV36XXX_MemMap_t NT36772_memory_map = {
    .EVENT_BUF_ADDR = 0x11E00,
    .RAW_PIPE0_ADDR = 0x10000,
    .RAW_PIPE1_ADDR = 0x12000,
    .BASELINE_ADDR = 0x10E70,
    .BASELINE_BTN_ADDR = 0x12E70,
    .DIFF_PIPE0_ADDR = 0x10830,
    .DIFF_PIPE1_ADDR = 0x12830,
    .RAW_BTN_PIPE0_ADDR = 0x10E60,
    .RAW_BTN_PIPE1_ADDR = 0x12E60,
    .DIFF_BTN_PIPE0_ADDR = 0x10E68,
    .DIFF_BTN_PIPE1_ADDR = 0x12E68,
    .READ_FLASH_CHECKSUM_ADDR = 0x14000,
    .RW_FLASH_DATA_ADDR = 0x14002,
};

static const struct _NV36XXX_MemMap_t NT36525_memory_map = {
    .EVENT_BUF_ADDR = 0x11A00,
    .RAW_PIPE0_ADDR = 0x10000,
    .RAW_PIPE1_ADDR = 0x12000,
    .BASELINE_ADDR = 0x10B08,
    .BASELINE_BTN_ADDR = 0x12B08,
    .DIFF_PIPE0_ADDR = 0x1064C,
    .DIFF_PIPE1_ADDR = 0x1264C,
    .RAW_BTN_PIPE0_ADDR = 0x10634,
    .RAW_BTN_PIPE1_ADDR = 0x12634,
    .DIFF_BTN_PIPE0_ADDR = 0x10AFC,
    .DIFF_BTN_PIPE1_ADDR = 0x12AFC,
    .READ_FLASH_CHECKSUM_ADDR = 0x14000,
    .RW_FLASH_DATA_ADDR = 0x14002,
};

static const struct _NV36XXX_MemMap_t NT36676F_memory_map = {
    .EVENT_BUF_ADDR = 0x11A00,
    .RAW_PIPE0_ADDR = 0x10000,
    .RAW_PIPE1_ADDR = 0x12000,
    .BASELINE_ADDR = 0x10B08,
    .BASELINE_BTN_ADDR = 0x12B08,
    .DIFF_PIPE0_ADDR = 0x1064C,
    .DIFF_PIPE1_ADDR = 0x1264C,
    .RAW_BTN_PIPE0_ADDR = 0x10634,
    .RAW_BTN_PIPE1_ADDR = 0x12634,
    .DIFF_BTN_PIPE0_ADDR = 0x10AFC,
    .DIFF_BTN_PIPE1_ADDR = 0x12AFC,
    .READ_FLASH_CHECKSUM_ADDR = 0x14000,
    .RW_FLASH_DATA_ADDR = 0x14002,
};

#define msleep(__ms) KeDelayExecutionThread(KernelMode, FALSE, &((LARGE_INTEGER) { __ms * 1000 * 10 }));

NTSTATUS OpenIOTarget(PDEVICE_EXTENSION ctx, LARGE_INTEGER res, ACCESS_MASK use, WDFIOTARGET* target);
NTSTATUS SetGPIO(WDFIOTARGET gpio, unsigned char* value);
NTSTATUS SpbWrite1DataSynchronously(IN SPB_CONTEXT* SpbContext, IN UCHAR Address, IN UCHAR Data)
{
    UCHAR buf[1] = { Data };
    return SpbWriteDataSynchronously(SpbContext, Address, buf, 1);
}


/* NVT_ start! */
NTSTATUS NVT_SetGPIOReset(FT5X_CONTROLLER_CONTEXT* controller, UCHAR off)
{
    unsigned char value;
    LARGE_INTEGER delay;
    PDEVICE_EXTENSION devContext = GetDeviceContext(controller->FxDevice);

    NTSTATUS status = OpenIOTarget(devContext, devContext->ResetGpioId, GENERIC_READ | GENERIC_WRITE, &devContext->ResetGpio);
    if (!NT_SUCCESS(status)) {
        //Trace(TRACE_LEVEL_ERROR, TRACE_DRIVER, "OpenIOTarget failed for Reset GPIO 0x%x", status);
        return status;
    }

    //Trace(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "Starting bring up sequence for the controller");

    //Trace(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "Setting reset gpio pin to low");

    if (!off)
    {
        value = 0;
        status = SetGPIO(devContext->ResetGpio, &value);
        if (!NT_SUCCESS(status)) {
            //Trace(TRACE_LEVEL_ERROR, TRACE_DRIVER, "SetGPIO failed for Reset GPIO 0x%x", status);
            return status;
        }

        //Trace(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "Waiting...");

        delay.QuadPart = -10 * TOUCH_POWER_RAIL_STABLE_TIME;
        KeDelayExecutionThread(KernelMode, TRUE, &delay);

        return STATUS_SUCCESS;
    }

    //Trace(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "Setting reset gpio pin to high");

    value = 1;
    status = SetGPIO(devContext->ResetGpio, &value);
    if (!NT_SUCCESS(status)) {
        //Trace(TRACE_LEVEL_ERROR, TRACE_DRIVER, "SetGPIO failed for Reset GPIO 0x%x", status);
        return status;
    }

    //Trace(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "Waiting...");

    delay.QuadPart = -10 * TOUCH_DELAY_TO_COMMUNICATE;
    KeDelayExecutionThread(KernelMode, TRUE, &delay);

    //Trace(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "Done");

    return STATUS_SUCCESS;
}
NTSTATUS NVT_InitChip(SPB_CONTEXT* context)
{
    UNREFERENCED_PARAMETER(context);
    NTSTATUS ret;

    ret = NVT_BootloaderReset(context);
    if (ret) return ret;

    ret = NVT_CheckFWResetState(context, NT36XXX_STATE_INIT);
    if (ret) return ret;

    bTouchIsAwake = TRUE;

    return STATUS_SUCCESS;
}

NTSTATUS NVT_Resume(SPB_CONTEXT* context)
{
    UNREFERENCED_PARAMETER(context);

    if (bTouchIsAwake) return STATUS_SUCCESS;

    NTSTATUS ret = NVT_BootloaderReset(context);
    if (ret) return ret;

    ret = NVT_CheckFWResetState(context, NT36XXX_STATE_REK);
    if (ret) return ret;

    bTouchIsAwake = TRUE;

    return STATUS_SUCCESS;
}

NTSTATUS NVT_Suspend(SPB_CONTEXT* context)
{
    UNREFERENCED_PARAMETER(context);

    if(!bTouchIsAwake) return STATUS_SUCCESS;
    bTouchIsAwake = FALSE;

    NTSTATUS ret = NVT_DeepSleep(context);
    if (ret) return ret;

    return STATUS_SUCCESS;
}

NTSTATUS NVT_DeepSleep(SPB_CONTEXT* context)
{
    UNREFERENCED_PARAMETER(context);

    unsigned char buf[2] = { NT36XXX_EVT_HOST_CMD , NT36XXX_CMD_ENTER_SLEEP };
    NTSTATUS ret = WriteI2C_FW(context, buf, sizeof(buf));
    if (ret) return ret;

    return STATUS_SUCCESS;
}

NTSTATUS NVT_SetPage(SPB_CONTEXT* context, unsigned int pageaddr)
{
    UNREFERENCED_PARAMETER(context);
    UNREFERENCED_PARAMETER(pageaddr);

    unsigned int data = (pageaddr) >> 8;
    NTSTATUS ret = WriteI2C_FW(context, &data, sizeof(data));
    if (ret) return ret;

    msleep( 2 );

    return STATUS_SUCCESS;
}

NTSTATUS NVT_BootloaderReset(SPB_CONTEXT* context)
{
    UNREFERENCED_PARAMETER(context);

    unsigned char buf[128] = { NT36XXX_CMD_BOOTLOADER_RESET };
    NTSTATUS ret = WriteI2C_FW(context, buf, sizeof(buf));
    if (ret) return ret;

    msleep( TIMEMS_BOOTLOADER_RESET_DELAY );

    return STATUS_SUCCESS;
}

NTSTATUS NVT_SwResetIdle(SPB_CONTEXT* context)
{
    UNREFERENCED_PARAMETER(context);

    unsigned char buf[2] = { NT36XXX_CMD_SW_RESET };
    NTSTATUS ret = WriteI2C_HW(context, buf, sizeof(buf));
    if (ret) return ret;

    msleep( TIMEMS_WARM_RESTART_DELAY );

    return STATUS_SUCCESS;
}

NTSTATUS NVT_CheckFWResetState(SPB_CONTEXT* context, UCHAR state)
{
    UNREFERENCED_PARAMETER(context);
    UNREFERENCED_PARAMETER(state);

    unsigned char buf[2] = { NT36XXX_EVT_RESET_COMPLETE , 0 };
    NTSTATUS ret;
    int retry;
    for (retry = 50; retry >= 0; --retry)
    {
        ret = WriteI2C_FW(context, buf, 1);
        ret = ReadI2C_FW(context, buf, sizeof(buf));
        if (!ret && buf[0] >= state && buf[0] <= NT36XXX_STATE_MAX) break;

        msleep( 10 );
    }

    return STATUS_SUCCESS;
}


/*NTSTATUS NVT_InitChip(SPB_CONTEXT* context)
{
    //unsigned char buf[64] = { 0 };
    NTSTATUS ret;

    //ret = NVT_DoBootloaderReset(context);
    //if (ret) return ret;
    ret = NVT_GetFWInfo(context);
    if (ret) return ret;

    for (unsigned char retries = 0; retries < 5; ++retries)
    {
        ret = NVT_WarmRestart(context);
        if (ret) continue;

        // Init bootloader?
        buf[0] = 0x00;
        buf[1] = 0x00;
        buf[2] = I2C_FW_Address;
        ret = WriteI2C_HW(context, buf, 3);
        if (ret) continue;

        int retry = 0;
        while (1)
        {
            msleep( 1 );
            buf[0] = 0x00;
            buf[1] = 0x00;
            ret = ReadI2C_HW(context, buf, 2);
            if (ret < 0) {
                //NVT_ERR("Check 0xAA (Inittial Flash Block) error!!(%d)\n", ret);
                return ret;
            }
            if (buf[1] == 0xAA) {
                break;
            }
            if (++retry > 20) {
                //NVT_ERR("Check 0xAA (Inittial Flash Block) error!! status=0x%02X\n", buf[1]);
                return STATUS_UNSUCCESSFUL;
            }
        }
        msleep( 20 );

        // resume PD
        buf[0] = 0x00;
        buf[1] = 0xAB;
        ret = WriteI2C_HW(context, buf, 2);
        if (ret) {
            //NVT_ERR("Write Enable error!!(%d)\n", ret);
            return ret;
        }
        retry = 0;
        while (1) {
            msleep(1);
            buf[0] = 0x00;
            buf[1] = 0x00;
            ret = ReadI2C_HW(context, buf, 2);
            if (ret) {
                //NVT_ERR("Check 0xAA (Resume Command) error!!(%d)\n", ret);
                return ret;
            }
            if (buf[1] == 0xAA) {
                break;
            }
            if (++retry > 20) {
                //NVT_ERR("Check 0xAA (Resume Command) error!! status=0x%02X\n", buf[1]);
                return STATUS_UNSUCCESSFUL;
            }
        }
        msleep( 10 );

        NVT_DoBootloaderReset(context); // end it! i hope so

        // other
        ret = NVT_UnlockCmd(context);
        if (ret) continue;

        // set page
        // NT36XXX_EVT_CHIPID part

        if (NVT_IsInCRCReboot(context) != FALSE)
        {
            ret = NVT_StopCRCRebooting(context);
        }
        if (ret == STATUS_SUCCESS) return ret;
    }
    return ret;
}
NTSTATUS NVT_WarmRestart(SPB_CONTEXT* context)
{
    UNREFERENCED_PARAMETER(context);

    unsigned char cmdBuf[2] = { NT36XXX_EVT_I2C_COMMAND , NT36XXX_CMD_SW_RESET };
    NTSTATUS ret = WriteI2C_HW(context, cmdBuf, 2);

    msleep( TIMEMS_WARM_RESTART_DELAY );
    return ret;
}

NTSTATUS NVT_DoBootloaderReset(SPB_CONTEXT* context)
{
	UNREFERENCED_PARAMETER(context);

	unsigned char cmdBuf[2] = { NT36XXX_EVT_I2C_COMMAND , NT36XXX_CMD_BOOTLOADER_RESET };
    NTSTATUS ret = WriteI2C_HW(context, cmdBuf, 2);

	msleep( TIMEMS_BOOTLOADER_RESET_DELAY );
    return ret;
}

NTSTATUS NVT_DoGPIOReset(FT5X_CONTROLLER_CONTEXT* controller, SPB_CONTEXT* context)
{
	UNREFERENCED_PARAMETER(controller);
    UNREFERENCED_PARAMETER(context);

    unsigned char value;
    LARGE_INTEGER delay;
    PDEVICE_EXTENSION devContext = GetDeviceContext(controller->FxDevice);

    if (!devContext->ResetGpio) return STATUS_UNSUCCESSFUL; // Not yet initialized?

    value = 0;
    NTSTATUS ret = SetGPIO(devContext->ResetGpio, &value);
    if (ret) return ret;

    delay.QuadPart = -10 * TOUCH_POWER_RAIL_STABLE_TIME;
    KeDelayExecutionThread(KernelMode, TRUE, &delay);

    value = 1;
    ret = SetGPIO(devContext->ResetGpio, &value);
    if (ret) return ret;

    delay.QuadPart = -10 * TOUCH_DELAY_TO_COMMUNICATE;
    KeDelayExecutionThread(KernelMode, TRUE, &delay);

    return ret;
}

NTSTATUS NVT_Suspend(SPB_CONTEXT* context)
{
	UNREFERENCED_PARAMETER(context);

    //if (bHasPaused) return;
    bHasPaused = TRUE;

	unsigned char cmdBuf[2] = { NT36XXX_EVT_HOST_CMD , NT36XXX_CMD_ENTER_SLEEP };
    NTSTATUS ret = WriteI2C_FW(context, cmdBuf, 2);

	msleep( TIMEMS_SUSPEND_DELAY );
    return ret;
}

NTSTATUS NVT_Resume(FT5X_CONTROLLER_CONTEXT* controller, SPB_CONTEXT* context)
{
    UNREFERENCED_PARAMETER(controller);
	UNREFERENCED_PARAMETER(context);

    //if (!bHasPaused) return;
    bHasPaused = FALSE;

    NTSTATUS ret = NVT_DoGPIOReset(controller, context);
    if (ret) return ret;

    ret = NVT_DoBootloaderReset(context);
    if (ret) return ret;

    ret = NVT_UnlockCmd(context);
    if (ret) return ret;

    if (NVT_IsInCRCReboot(context) != FALSE) ret = NVT_StopCRCRebooting(context);
    return ret;
}

NTSTATUS NVT_UnlockCmd(SPB_CONTEXT* context)
{
    UNREFERENCED_PARAMETER(context);

    unsigned char cmdBuf[2] = { NT36XXX_EVT_I2C_COMMAND , NT36XXX_CMD_UNLOCK };
    NTSTATUS ret = WriteI2C_HW(context, cmdBuf, 2);

    msleep( TIMEMS_UNLOCK_CMD_DELAY );
    return ret;
}

BOOLEAN NVT_IsInCRCReboot(SPB_CONTEXT* context)
{
    UNREFERENCED_PARAMETER(context);

    return TRUE;
}

NTSTATUS NVT_StopCRCRebooting(SPB_CONTEXT* context)
{
    UNREFERENCED_PARAMETER(context);

    unsigned char buf[8] = { 0 };
    NVT_SetPage(context, I2C_BLDR_Address, 0x1F64E);
    buf[0] = 0x4E;
    NTSTATUS ret = WriteI2C_BLDR(context, buf, 4);
    if (ret) return ret;

    if ((buf[1] == 0xFC) ||
        ((buf[1] == 0xFF) && (buf[2] == 0xFF) && (buf[3] == 0xFF)))
    {

        //IC is in CRC fail reboot loop, needs to be stopped!
        for (unsigned char retry = 5; retry > 0; --retry) {

            //---write i2c cmds to reset idle : 1st---
            buf[0] = NT36XXX_EVT_I2C_COMMAND;
            buf[1] = NT36XXX_CMD_SW_RESET;
            WriteI2C_HW(context, buf, 2);

            //---write i2c cmds to reset idle : 2rd---
            buf[0] = NT36XXX_EVT_I2C_COMMAND;
            buf[1] = NT36XXX_CMD_SW_RESET;
            WriteI2C_HW(context, buf, 2);
            KeDelayExecutionThread(KernelMode, FALSE, &((LARGE_INTEGER) { 1 * 1000 * 10 }));

            //---clear CRC_ERR_FLAG---
            NVT_SetPage(context, I2C_BLDR_Address, 0x3F135);

            buf[0] = NT36XXX_EVT_CRC;
            buf[1] = 0xA5;
            WriteI2C_BLDR(context, buf, 2);

            //---check CRC_ERR_FLAG---
            NVT_SetPage(context, I2C_BLDR_Address, 0x3F135);

            buf[0] = NT36XXX_EVT_CRC;
            buf[1] = 0x00;
            ReadI2C_BLDR(context, buf, 2);

            if (buf[1] == 0xA5)
            {
                msleep( 10 );
                return STATUS_SUCCESS;
            }
        }
    }
    return STATUS_UNSUCCESSFUL;
}

NTSTATUS NVT_GetFWInfo(SPB_CONTEXT* context)
{
    UNREFERENCED_PARAMETER(context);
    NTSTATUS ret;

    ret = NVT_SetPage(context, NT36672A_EVENT_BUF_ADDR);

    unsigned char buf[32] = {0};

    NTSTATUS ret = NVT_SetPage(context, I2C_FW_Address, NT36672A_EVENT_BUF_ADDR | NT36XXX_EVT_FWINFO);
    if (ret) return ret;

    ret = ReadI2C_FW(context, buf, 17);
    if (ret || (buf[1] + buf[2]) != 0xFF) // broken firmware ?
    {
        fwInfo.firmwareVer = 0;
        fwInfo.x_num = 18;
        fwInfo.y_num = 32;
        fwInfo.maxButton = TOUCH_MAX_FINGER_NUM;
        fwInfo.abs_x_max = TOUCH_DEVICE_RESOLUTION_X;
        fwInfo.abs_y_max = TOUCH_DEVICE_RESOLUTION_Y;
        return STATUS_SUCCESS; // whatever...
    }
    fwInfo.firmwareVer = buf[1];
    fwInfo.x_num = buf[3];
    fwInfo.y_num = buf[4];
    fwInfo.maxButton = buf[11];
    fwInfo.abs_x_max = (unsigned short)((buf[5] << 8) | buf[6]);
    fwInfo.abs_y_max = (unsigned short)((buf[7] << 8) | buf[8]);

    return ret;
}*/