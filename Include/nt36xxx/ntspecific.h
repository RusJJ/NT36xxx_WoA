#pragma once

#include "ntinternal.h"

#define TIMEMS_BOOTLOADER_RESET_DELAY	35
#define TIMEMS_SUSPEND_DELAY			50
#define TIMEMS_WARM_RESTART_DELAY		16
#define TIMEMS_UNLOCK_CMD_DELAY			10

typedef struct _NVT_FWInfo_t
{
	unsigned char firmwareVer;
	unsigned char x_num;
	unsigned char y_num;
	unsigned char maxButton;
	unsigned short abs_x_max;
	unsigned short abs_y_max;
} NVT_FWInfo_t, * PNVT_FWInfo_t;

typedef struct _NV36XXX_MemMap_t
{
	unsigned int EVENT_BUF_ADDR;
	unsigned int RAW_PIPE0_ADDR;
	unsigned int RAW_PIPE1_ADDR;
	unsigned int BASELINE_ADDR;
	unsigned int BASELINE_BTN_ADDR;
	unsigned int DIFF_PIPE0_ADDR;
	unsigned int DIFF_PIPE1_ADDR;
	unsigned int RAW_BTN_PIPE0_ADDR;
	unsigned int RAW_BTN_PIPE1_ADDR;
	unsigned int DIFF_BTN_PIPE0_ADDR;
	unsigned int DIFF_BTN_PIPE1_ADDR;
	unsigned int READ_FLASH_CHECKSUM_ADDR;
	unsigned int RW_FLASH_DATA_ADDR;
} NV36XXX_MemMap_t, * PNV36XXX_MemMap_t;

NTSTATUS NVT_SetGPIOReset(FT5X_CONTROLLER_CONTEXT* controller, UCHAR off);
NTSTATUS NVT_InitChip(SPB_CONTEXT* context);
NTSTATUS NVT_Resume(SPB_CONTEXT* context);
NTSTATUS NVT_Suspend(SPB_CONTEXT* context);
NTSTATUS NVT_DeepSleep(SPB_CONTEXT* context);
NTSTATUS NVT_SetPage(SPB_CONTEXT* context, unsigned int pageaddr);
NTSTATUS NVT_BootloaderReset(SPB_CONTEXT* context);
NTSTATUS NVT_SwResetIdle(SPB_CONTEXT* context);
NTSTATUS NVT_CheckFWResetState(SPB_CONTEXT* context, UCHAR state);

extern NVT_FWInfo_t fwInfo;
extern PNV36XXX_MemMap_t memMap;
extern UCHAR bTouchIsAwake;