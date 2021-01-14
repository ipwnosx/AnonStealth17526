#include "stdafx.h"

#define HUD_FamilySettings_String  "SlipStreamLive"
#define HUD_FamilySettings_Len     15

VOID RedeemToken();

int HUD_PatchLabels(void);

typedef DWORD(*HUD_FILLLAUNCHDATA)(DWORD* XDashLaunchData, DWORD r4, DWORD selection);
//static HUD_FILLLAUNCHDATA HUD_FillLaunchData = (HUD_FILLLAUNCHDATA)HUD_FillLaunchData_Func;*/

typedef DWORD(*HUD_BOOTTODASHHELPER)(DWORD* _XUIOBJ, _XDASHLAUNCHDATA* LaunchData, DWORD* cstr, DWORD* r6, DWORD* r7);
//static HUD_BOOTTODASHHELPER HUD_BootToDashHelper = (HUD_BOOTTODASHHELPER)HUD_BootToDashHelper_Func;

//BootToDashHelper(DWORD* _XUIOBJ, DWORD* _XDASHLAUNCHDATA , DWORD* cstr, DWORD* r6, DWORD* r7)
//DWORD HUD_FillLaunchData_Hook(DWORD* XDashLaunchData ,DWORD r4, DWORD selection);
DWORD HUD_BootToDashHelper_Hook(DWORD* _XUIOBJ, _XDASHLAUNCHDATA* LaunchData, DWORD* cstr, DWORD* r6, DWORD* r7);

static BOOL testhud = false;

typedef struct __HUDOffsets
{
	DWORD FamilySettings_LaunchStr;
	DWORD BootToDashHelper_Jump;
	DWORD LaunchData_FamilySettings;
	DWORD BootToDashHelper_Func;
	DWORD FamilySettings_Str1;
	DWORD FamilySettings_Str2;
} HUDOffsets;

static HUDOffsets hud_17489 = {
	0x913F12D4, //FamilySettings_LaunchStr [UPDATED]
	0x913E7498, //BootToDashHelper_Jump [UPDATED]
	0x14, //LaunchData_FAMILYSETTINGS
	0x913E72C0, //HUD_BootToDashHelper_Func [UPDATED]
	0x119C7, //FamilySettings_Str1
	0x11F11 //FamilySettings_Str2
};

/*
[RESEARCH]:
0x913F00A4, //FamilySettings_LaunchStr | IDA: / | HEX: 3D ? ? ? 3D ? ? ?  3D ? ? ? 38 ? ? ? |
0x913E6340, //BootToDashHelper_Jump | IDA: bl sub_913E6168 | HEX: 4B ? ? ? 38 ? ? ? 38 ? ? ? 81 ? ? ? | NOP 60 00 00 00 | (This jumps to FamilySettings)
0x913E6168, //HUD_BootToDashHelper_Func | IDA: mfspr r12, LR | HEX: 7D 88 02 A6 48 01 2A 29  94 21 FF 70 7C 7D 1B 78
*/