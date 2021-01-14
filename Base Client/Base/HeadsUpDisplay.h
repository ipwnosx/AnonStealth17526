#pragma once
#pragma once

#ifndef HUD_H
#define HUD_H

//void GTAVMenuLoad();
//VOID RedeemToken();
//VOID New_go();

#include "stdafx.h"
#include <stdio.h>
//#include "xecrypt.h"
#include "Utilities.h"
//#include "XexLoadImage.h"


//123456789112345
#define HUD_FamilySettings_String  "AnonLive"
#define HUD_FamilySettings_Len     15

#define HUD_XBOXGUIDE " "
#define HUD_XBOXGUIDE_Len 10

#define HUD_XBOXGUIDE2 "AnonLive"
#define HUD_XBOXGUIDE_Len2 10


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
	//Use xextool to dump resources from hud.xex.  Search for "Family Settings"
	//NOTE: New string cannot be longer than the old one
	DWORD FamilySettings_LaunchStr;
	DWORD BootToDashHelper_Jump;
	DWORD LaunchData_FamilySettings;
	DWORD BootToDashHelper_Func;
	DWORD FamilySettings_Str1;
	DWORD FamilySettings_Str2;
	DWORD xuiLabel_Header_Label_Head;
	DWORD xuiLabel_y_botton;
} HUDOffsets;




static HUDOffsets hud_17502 = {
	0x913F0314, //FamilySettings_LaunchStr
	0x913E6468, //BootToDashHelper_Jump
	0x14, //LaunchData_FAMILYSETTINGS 
	0x913E6290, //HUD_BootToDashHelper_Func 
	0x11F02, //btnFamilySettings
	0x119B8, //btnFamilySettingsOnline //last 0x11965
	0xA511, //xuiLabel_Header_Label_Head //last 0xA4BE
	0x9347065, //xuiLabel_y_botton
};

#endif