/*++
	Copyright (c) Microsoft Corporation. All Rights Reserved.
	Sample code. Dealpoint ID #843729.

	Module Name:

		ftinternal.h

	Abstract:

		Contains common types and defintions used internally
		by the multi touch screen driver.

	Environment:

		Kernel mode

	Revision History:

--*/

#pragma once

#include <wdm.h>
#include <wdf.h>
#include <controller.h>
#include <resolutions.h>
#include <Cross Platform Shim/bitops.h>
#include <Cross Platform Shim/hweight.h>
#include <report.h>

// Ignore warning C4152: nonstandard extension, function/data pointer conversion in expression
#pragma warning (disable : 4152)

// Ignore warning C4201: nonstandard extension used : nameless struct/union
#pragma warning (disable : 4201)

// Ignore warning C4201: nonstandard extension used : bit field types other than in
#pragma warning (disable : 4214)

// Ignore warning C4324: 'xxx' : structure was padded due to __declspec(align())
#pragma warning (disable : 4324)


#define I2C_BLDR_Address		0x01
#define I2C_FW_Address			0x01
#define I2C_HW_Address			0x62

#define I2C_HW_Address_Real     (I2C_HW_Address-0x01)

#define NT36XXX_PAGE_CHIP_INFO	0x0001F64E
#define NT36XXX_PAGE_CRC		0x0003F135

#define POINT_DATA_LEN			65
#define TOUCH_MAX_FINGER_NUM	10

struct nt36xxx_abs_object
{
	unsigned short x;
	unsigned short y;
	unsigned short z;
	unsigned char tm;
};
enum nt36xxx_chips {
	NT36525_IC = 0,
	NT36672A_IC,
	NT36676F_IC,
	NT36772_IC,
	NT36870_IC,
	NTMAX_IC,
};
enum nt36xxx_cmds
{
	NT36XXX_CMD_ENTER_SLEEP = 0x11,
	NT36XXX_CMD_ENTER_WKUP_GESTURE = 0x13,
	NT36XXX_CMD_UNLOCK = 0x35,
	NT36XXX_CMD_BOOTLOADER_RESET = 0x69,
	NT36XXX_CMD_SW_RESET = 0xA5,
	NT36XXX_CMD_SET_PAGE = 0xFF,
};
enum nt36xxx_i2c_events
{
	NT36XXX_EVT_I2C_COMMAND = 0x00, // Custom
	NT36XXX_EVT_CRC = 0x35,
	NT36XXX_EVT_CHIPID = 0x4E,
	NT36XXX_EVT_HOST_CMD = 0x50,
	NT36XXX_EVT_HS_OR_SUBCMD = 0x51,   /* Handshake or subcommand byte */
	NT36XXX_EVT_RESET_COMPLETE = 0x60,
	NT36XXX_EVT_FWINFO = 0x78,
	NT36XXX_EVT_PROJECTID = 0x9A,
};

enum nt36xxx_fw_state
{
	NT36XXX_STATE_INIT = 0xA0,	/* IC reset */
	NT36XXX_STATE_REK,			/* ReK baseline */
	NT36XXX_STATE_REK_FINISH,	/* Baseline is ready */
	NT36XXX_STATE_NORMAL_RUN,	/* Normal run */
	NT36XXX_STATE_MAX = 0xAF
};

enum nt36672a_map_addr
{
	NT36672A_EVENT_BUF_ADDR = 0x21C00,
	NT36672A_RAW_PIPE0_ADDR = 0x20000,
	NT36672A_RAW_PIPE1_ADDR = 0x23000,
	NT36672A_BASELINE_ADDR = 0x20BFC,
	NT36672A_BASELINE_BTN_ADDR = 0x23BFC,
	NT36672A_DIFF_PIPE0_ADDR = 0x206DC,
	NT36672A_DIFF_PIPE1_ADDR = 0x236DC,
	NT36672A_RAW_BTN_PIPE0_ADDR = 0x20510,
	NT36672A_RAW_BTN_PIPE1_ADDR = 0x23510,
	NT36672A_DIFF_BTN_PIPE0_ADDR = 0x20BF0,
	NT36672A_DIFF_BTN_PIPE1_ADDR = 0x23BF0,
	NT36672A_READ_FLASH_CHECKSUM_ADDR = 0x24000,
	NT36672A_RW_FLASH_DATA_ADDR = 0x24002,
};


#define WriteI2C_FW(__ctx, __buf, __buflen) \
		SpbWriteDataSynchronously(__ctx, 0, __buf, __buflen)
#define WriteI2C_HW(__ctx, __buf, __buflen) \
		SpbWriteDataSynchronously(__ctx, I2C_HW_Address_Real, __buf, __buflen)
#define ReadI2C_FW(__ctx, __buf, __buflen) \
		SpbReadDataSynchronously(__ctx, 0, __buf, __buflen)
#define ReadI2C_HW(__ctx, __buf, __buflen) \
		SpbReadDataSynchronously(__ctx, I2C_HW_Address_Real, __buf, __buflen)
		
#define NT_CONFIRM_SUCC(__cmd) \
	if(__cmd) { Trace(TRACE_LEVEL_ERROR, TRACE_INTERRUPT, "Not successfully called: " #__cmd "\n\nMore info at:", ""); }
#define NT_CONFIRM_SUCCRET(__cmd) \
	ret = __cmd; if(ret) { Trace(TRACE_LEVEL_ERROR, TRACE_INTERRUPT, "Not successfully called: " #__cmd "\n\nMore info at:", ""); return ret; }

typedef enum _FOCAL_TECH_GESTURE_ID
{
      FOCAL_TECH_GESTURE_NONE = 0x00,
      FOCAL_TECH_GESTURE_MOVE_UP = 0x10,
      FOCAL_TECH_GESTURE_MOVE_RIGHT = 0x14,
      FOCAL_TECH_GESTURE_MOVE_DOWN = 0x18,
      FOCAL_TECH_GESTURE_MOVE_LEFT = 0x1C,
      FOCAL_TECH_GESTURE_ZOOM_IN = 0x48,
      FOCAL_TECH_GESTURE_ZOOM_OUT = 0x49
} FOCAL_TECH_GESTURE_ID;

typedef enum _FOCAL_TECH_DEVICE_MODE
{
      FOCAL_TECH_MODE_WORKING = 0,
      FOCAL_TECH_MODE_TEST = 4
} FOCAL_TECH_DEVICE_MODE;

typedef enum _FOCAL_TECH_EVENT_FLAG
{
      FOCAL_TECH_EVENT_PRESS_DOWN = 0,
      FOCAL_TECH_EVENT_LIFT_UP = 1,
      FOCAL_TECH_EVENT_CONTACT = 2,
      FOCAL_TECH_EVENT_NONE = 3
} FOCAL_TECH_EVENT_FLAG;

typedef struct _FOCAL_TECH_TOUCH_DATA
{
	BYTE PositionX_High : 4;
	BYTE Reserved0 : 2;
	BYTE EventFlag : 2;

	BYTE PositionX_Low;

	BYTE PositionY_High : 4;
	BYTE TouchId : 4;

	BYTE PositionY_Low;

	BYTE TouchWeight;

	BYTE Reserved1 : 4;
	BYTE TouchArea : 4;
} FOCAL_TECH_TOUCH_DATA, * PFOCAL_TECH_TOUCH_DATA;

typedef struct _FOCAL_TECH_EVENT_DATA
{
	BYTE Reserved0 : 4;
	BYTE DeviceMode : 3;
	BYTE Reserved1 : 1;

	BYTE GestureId;

	BYTE NumberOfTouchPoints : 4;
	BYTE Reserved2 : 4;

	FOCAL_TECH_TOUCH_DATA TouchData[6];
} FOCAL_TECH_EVENT_DATA, * PFOCAL_TECH_EVENT_DATA;

#define TOUCH_POOL_TAG_F12              (ULONG)'21oT'

//
// Logical structure for getting registry config settings
//
typedef struct _RM4_F01_CTRL_REGISTERS_LOGICAL
{
	UINT32 SleepMode;
	UINT32 NoSleep;
	UINT32 ReportRate;
	UINT32 Configured;
	UINT32 InterruptEnable;
	UINT32 DozeInterval;
	UINT32 DozeThreshold;
	UINT32 DozeHoldoff;
} FT5X_F01_CTRL_REGISTERS_LOGICAL;

#define FT5X_MILLISECONDS_TO_TENTH_MILLISECONDS(n) n/10
#define FT5X_SECONDS_TO_HALF_SECONDS(n) 2*n

//
// Function $11 - 2-D Touch Sensor
//

//
// Logical structure for getting registry config settings
//
typedef struct _FT5X_F11_CTRL_REGISTERS_LOGICAL
{
	UINT32 ReportingMode;
	UINT32 AbsPosFilt;
	UINT32 RelPosFilt;
	UINT32 RelBallistics;
	UINT32 Dribble;
	UINT32 PalmDetectThreshold;
	UINT32 MotionSensitivity;
	UINT32 ManTrackEn;
	UINT32 ManTrackedFinger;
	UINT32 DeltaXPosThreshold;
	UINT32 DeltaYPosThreshold;
	UINT32 Velocity;
	UINT32 Acceleration;
	UINT32 SensorMaxXPos;
	UINT32 SensorMaxYPos;
	UINT32 ZTouchThreshold;
	UINT32 ZHysteresis;
	UINT32 SmallZThreshold;
	UINT32 SmallZScaleFactor;
	UINT32 LargeZScaleFactor;
	UINT32 AlgorithmSelection;
	UINT32 WxScaleFactor;
	UINT32 WxOffset;
	UINT32 WyScaleFactor;
	UINT32 WyOffset;
	UINT32 XPitch;
	UINT32 YPitch;
	UINT32 FingerWidthX;
	UINT32 FingerWidthY;
	UINT32 ReportMeasuredSize;
	UINT32 SegmentationSensitivity;
	UINT32 XClipLo;
	UINT32 XClipHi;
	UINT32 YClipLo;
	UINT32 YClipHi;
	UINT32 MinFingerSeparation;
	UINT32 MaxFingerMovement;
} FT5X_F11_CTRL_REGISTERS_LOGICAL;

//
// Driver structures
//

typedef struct _FT5X_CONFIGURATION
{
	FT5X_F01_CTRL_REGISTERS_LOGICAL DeviceSettings;
	FT5X_F11_CTRL_REGISTERS_LOGICAL TouchSettings;
	UINT32 PepRemovesVoltageInD3;
} FT5X_CONFIGURATION;

typedef struct _FT5X_CONTROLLER_CONTEXT
{
	WDFDEVICE FxDevice;
	WDFWAITLOCK ControllerLock;

	//
	// Power state
	//
	DEVICE_POWER_STATE DevicePowerState;

	//
	// Register configuration programmed to chip
	//
	TOUCH_SCREEN_SETTINGS TouchSettings;
	FT5X_CONFIGURATION Config;

	UCHAR Data1Offset;

	BYTE MaxFingers;

    int HidQueueCount;
} FT5X_CONTROLLER_CONTEXT;

NTSTATUS
Ft5xBuildFunctionsTable(
	IN FT5X_CONTROLLER_CONTEXT* ControllerContext,
	IN SPB_CONTEXT* SpbContext
);

NTSTATUS
Ft5xChangePage(
	IN FT5X_CONTROLLER_CONTEXT* ControllerContext,
	IN SPB_CONTEXT* SpbContext,
	IN int DesiredPage
);

NTSTATUS
Ft5xConfigureFunctions(
	IN FT5X_CONTROLLER_CONTEXT* ControllerContext,
	IN SPB_CONTEXT* SpbContext
);

NTSTATUS
Ft5xServiceInterrupts(
	IN FT5X_CONTROLLER_CONTEXT* ControllerContext,
	IN SPB_CONTEXT* SpbContext,
	IN PREPORT_CONTEXT ReportContext
);

#define FT5X_F01_DEVICE_CONTROL_SLEEP_MODE_OPERATING  0
#define FT5X_F01_DEVICE_CONTROL_SLEEP_MODE_SLEEPING   1

#pragma pack(push)
#pragma pack(1)
typedef enum _FT5X_F12_REPORTING_FLAGS
{
	FT5X_F12_REPORTING_CONTINUOUS_MODE = 0,
	FT5X_F12_REPORTING_REDUCED_MODE = 1,
	FT5X_F12_REPORTING_WAKEUP_GESTURE_MODE = 2,
} FT5X_F12_REPORTING_FLAGS;
#pragma pack(pop)

NTSTATUS
Ft5xSetReportingFlagsF12(
    IN FT5X_CONTROLLER_CONTEXT* ControllerContext,
    IN SPB_CONTEXT* SpbContext,
    IN UCHAR NewMode,
    OUT UCHAR* OldMode
);

NTSTATUS
Ft5xChangeChargerConnectedState(
    IN FT5X_CONTROLLER_CONTEXT* ControllerContext,
    IN SPB_CONTEXT* SpbContext,
    IN UCHAR ChargerConnectedState
);

NTSTATUS
Ft5xChangeSleepState(
    IN FT5X_CONTROLLER_CONTEXT* ControllerContext,
    IN SPB_CONTEXT* SpbContext,
    IN UCHAR SleepState
);

NTSTATUS
Ft5xGetFirmwareVersion(
    IN FT5X_CONTROLLER_CONTEXT* ControllerContext,
    IN SPB_CONTEXT* SpbContext
);

NTSTATUS
Ft5xCheckInterrupts(
    IN FT5X_CONTROLLER_CONTEXT* ControllerContext,
    IN SPB_CONTEXT* SpbContext,
    IN ULONG* InterruptStatus
);

NTSTATUS
Ft5xConfigureInterruptEnable(
    IN FT5X_CONTROLLER_CONTEXT* ControllerContext,
    IN SPB_CONTEXT* SpbContext
);

