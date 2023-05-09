/*++
      Copyright (c) Microsoft Corporation. All Rights Reserved.
      Sample code. Dealpoint ID #843729.

      Module Name:

            rmiinternal.c

      Abstract:

            Contains Synaptics initialization code

      Environment:

            Kernel mode

      Revision History:

--*/

#include <Cross Platform Shim\compat.h>
#include <spb.h>
#include <report.h>
#include <nt36xxx\ntinternal.h>
#include <nt36xxx\ntspecific.h>
#include <ntinternal.tmh>

NTSTATUS
Ft5xBuildFunctionsTable(
      IN FT5X_CONTROLLER_CONTEXT* ControllerContext,
      IN SPB_CONTEXT* SpbContext
)
{
      UNREFERENCED_PARAMETER(SpbContext);
      UNREFERENCED_PARAMETER(ControllerContext);

      return STATUS_SUCCESS;
}

NTSTATUS
Ft5xChangePage(
      IN FT5X_CONTROLLER_CONTEXT* ControllerContext,
      IN SPB_CONTEXT* SpbContext,
      IN int DesiredPage
)
{
      UNREFERENCED_PARAMETER(SpbContext);
      UNREFERENCED_PARAMETER(ControllerContext);
      UNREFERENCED_PARAMETER(DesiredPage);

      return STATUS_SUCCESS;
}

NTSTATUS
Ft5xConfigureFunctions(
      IN FT5X_CONTROLLER_CONTEXT* ControllerContext,
      IN SPB_CONTEXT* SpbContext
)
{
      UNREFERENCED_PARAMETER(SpbContext);
      UNREFERENCED_PARAMETER(ControllerContext);

      return STATUS_SUCCESS;
}

NTSTATUS
Ft5xGetObjectStatusFromControllerF12(
      IN VOID* ControllerContext,
      IN SPB_CONTEXT* SpbContext,
      IN DETECTED_OBJECTS* Data
)
/*++

Routine Description:

      This routine reads raw touch messages from hardware. If there is
      no touch data available (if a non-touch interrupt fired), the
      function will not return success and no touch data was transferred.

Arguments:

      ControllerContext - Touch controller context
      SpbContext - A pointer to the current i2c context
      Data - A pointer to any returned F11 touch data

Return Value:

      NTSTATUS, where only success indicates data was returned

--*/
{
    NTSTATUS status;
    FT5X_CONTROLLER_CONTEXT* controller;
    controller = (FT5X_CONTROLLER_CONTEXT*)ControllerContext;

    struct nt36xxx_abs_object objd = { 0, 0, 0, 0 };
    struct nt36xxx_abs_object* obj = &objd;

    //enable TchTranslateToDisplayCoordinates in report.c

    unsigned char input_id = 0;
    unsigned char point[POINT_DATA_LEN] = { 0 }; // Part 1: not doing +1 because ReadI2C reads the register mem starting from 0x02 (but FW is at 0x01 !!! )
    unsigned int ppos = 0;

    int max_x = 1080, max_y = 2246;

    status = ReadI2C_FW(SpbContext, point, sizeof(point));

    if (!NT_SUCCESS(status))
    {
        Trace(
            TRACE_LEVEL_ERROR,
            TRACE_INTERRUPT,
            "Error reading finger status data - 0x%08lX",
            status);

        goto exit;
    }

    if (!bTouchIsAwake)
    {
        input_id = point[0] >> 3;

        status = STATUS_SUCCESS;
        goto exit;
    }

    for (unsigned char i = 0; i < TOUCH_MAX_FINGER_NUM; i++)
    {
        ppos = 6 * i; // Part 2: not doing +1 because of ReadI2C reading from 0x02
        input_id = point[ppos + 0] >> 3;
        if ((input_id == 0) || (input_id > TOUCH_MAX_FINGER_NUM)) continue;

        if (((point[ppos] & 0x07) == 0x01) || ((point[ppos] & 0x07) == 0x02))
        {
            obj->x = (point[ppos + 1] << 4) + (point[ppos + 3] >> 4);
            obj->y = (point[ppos + 2] << 4) + (point[ppos + 3] & 0xF);
            if ((obj->x > max_x) || (obj->y > max_y)) continue;

            obj->tm = point[ppos + 4];
            if (obj->tm == 0) obj->tm = 1;

            obj->z = point[ppos + 5];

            Data->States[i] = OBJECT_STATE_FINGER_PRESENT_WITH_ACCURATE_POS;

            Trace(
                TRACE_LEVEL_ERROR,
                TRACE_INTERRUPT,
                "x: %d, y:%d",
                obj->x, obj->y);

            Data->Positions[i].X = obj->x;
            Data->Positions[i].Y = obj->y;
        }
    }

exit:
    return status;
}

NTSTATUS
TchServiceObjectInterrupts(
      IN FT5X_CONTROLLER_CONTEXT* ControllerContext,
      IN SPB_CONTEXT* SpbContext,
      IN PREPORT_CONTEXT ReportContext
)
{
      NTSTATUS status = STATUS_SUCCESS;
      DETECTED_OBJECTS data;

      RtlZeroMemory(&data, sizeof(data));

      //
      // See if new touch data is available
      //
      status = Ft5xGetObjectStatusFromControllerF12(
            ControllerContext,
            SpbContext,
            &data
      );

      if (!NT_SUCCESS(status))
      {
            Trace(
                  TRACE_LEVEL_VERBOSE,
                  TRACE_SAMPLES,
                  "No object data to report - 0x%08lX",
                  status);

            goto exit;
      }

      status = ReportObjects(
            ReportContext,
            data);

      if (!NT_SUCCESS(status))
      {
            Trace(
                  TRACE_LEVEL_VERBOSE,
                  TRACE_SAMPLES,
                  "Error while reporting objects - 0x%08lX",
                  status);

            goto exit;
      }

exit:
      return status;
}


NTSTATUS
Ft5xServiceInterrupts(
      IN FT5X_CONTROLLER_CONTEXT* ControllerContext,
      IN SPB_CONTEXT* SpbContext,
      IN PREPORT_CONTEXT ReportContext
)
{
      NTSTATUS status = STATUS_SUCCESS;

      TchServiceObjectInterrupts(ControllerContext, SpbContext, ReportContext);

      return status;
}

NTSTATUS
Ft5xSetReportingFlagsF12(
    IN FT5X_CONTROLLER_CONTEXT* ControllerContext,
    IN SPB_CONTEXT* SpbContext,
    IN UCHAR NewMode,
    OUT UCHAR* OldMode
)
{
      UNREFERENCED_PARAMETER(SpbContext);
      UNREFERENCED_PARAMETER(ControllerContext);
      UNREFERENCED_PARAMETER(NewMode);
      UNREFERENCED_PARAMETER(OldMode);

      return STATUS_SUCCESS;
}

NTSTATUS
Ft5xChangeChargerConnectedState(
    IN FT5X_CONTROLLER_CONTEXT* ControllerContext,
    IN SPB_CONTEXT* SpbContext,
    IN UCHAR ChargerConnectedState
)
{
      UNREFERENCED_PARAMETER(SpbContext);
      UNREFERENCED_PARAMETER(ControllerContext);
      UNREFERENCED_PARAMETER(ChargerConnectedState);

      return STATUS_SUCCESS;
}

NTSTATUS
Ft5xChangeSleepState(
    IN FT5X_CONTROLLER_CONTEXT* ControllerContext,
    IN SPB_CONTEXT* SpbContext,
    IN UCHAR SleepState
)
{
      UNREFERENCED_PARAMETER(SpbContext);
      UNREFERENCED_PARAMETER(ControllerContext);
      UNREFERENCED_PARAMETER(SleepState);

      switch (SleepState)
      {
      case FT5X_F01_DEVICE_CONTROL_SLEEP_MODE_SLEEPING:
          NVT_Suspend(SpbContext);
          return NVT_SetGPIOReset(ControllerContext, 1);
          //return STATUS_SUCCESS;

      case FT5X_F01_DEVICE_CONTROL_SLEEP_MODE_OPERATING:
          NVT_SetGPIOReset(ControllerContext, 0);
          return NVT_BootloaderReset(SpbContext);
          //return NVT_Resume(SpbContext);
      }

      return STATUS_SUCCESS;
}

NTSTATUS
Ft5xGetFirmwareVersion(
    IN FT5X_CONTROLLER_CONTEXT* ControllerContext,
    IN SPB_CONTEXT* SpbContext
)
{
      UNREFERENCED_PARAMETER(SpbContext);
      UNREFERENCED_PARAMETER(ControllerContext);

      return STATUS_SUCCESS;
}

NTSTATUS
Ft5xCheckInterrupts(
    IN FT5X_CONTROLLER_CONTEXT* ControllerContext,
    IN SPB_CONTEXT* SpbContext,
    IN ULONG* InterruptStatus
)
{
      UNREFERENCED_PARAMETER(SpbContext);
      UNREFERENCED_PARAMETER(ControllerContext);
      UNREFERENCED_PARAMETER(InterruptStatus);

      return STATUS_SUCCESS;
}

NTSTATUS
Ft5xConfigureInterruptEnable(
    IN FT5X_CONTROLLER_CONTEXT* ControllerContext,
    IN SPB_CONTEXT* SpbContext
)
{
      UNREFERENCED_PARAMETER(SpbContext);
      UNREFERENCED_PARAMETER(ControllerContext);

      return STATUS_SUCCESS;
}