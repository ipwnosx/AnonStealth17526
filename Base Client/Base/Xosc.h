#pragma once
#include "stdafx.h"

#define XOSC_FOOTER_MAGIC 0x5F534750

typedef enum _XOSC_FLAGS : QWORD {	
    XOSC_FLAGS_TASK_SHOULD_EXIT  = 0x2000000000000000,
    XOSC_FLAGS_TITLE_TERMINATED  = 0x4000000000000000
} XOSC_FLAGS;

typedef enum _SV_PROTECTED_FLAGS { 
	FLAG_SV_PROTECTED_NONE = 0x0,
    FLAG_SV_PROTECTED_NO_EJECT_REBOOT = 0x1, //Set on dash and such. Means the box doesn't change titles if the disc tray is ejected.
    FLAG_SV_PROTECTED_DISC_AUTHENTICATED = 0x2, //Is set when a disc is put in the tray and completely verified.
    FLAG_SV_PROTECTED_AUTH_EX_CAPABLE = 0x4
} SV_PROTECTED_FLAGS;

#pragma pack(1)
typedef struct _XOSC {
        DWORD                   dwResult;                       // 0x00 - 0x04
        BYTE                    Unknown1[0x4];                  // 0x04 - 0x08
        QWORD                   qwOperations;                   // 0x08 - 0x10
        BYTE                    Unknown2[0x28];                 // 0x10 - 0x38
        XEX_EXECUTION_ID        xexExecutionId;                 // 0x38 - 0x50
        BYTE                    bCpuKeyHash[0x10];              // 0x50 - 0x60
        BYTE                    bKvHash[0x10];                  // 0x60 - 0x70
        BYTE                    Unknwon3[0x268];                // 0x70 - 0x2D8
        DWORD                   dwFooterMagic;                  // 0x2D8 - 0x2DC
        DWORD                   dwUnknown9;                     // 0x2DC - 0x2E0
		BYTE					pad2[0x10];
		BYTE					pad3[0x10];
		BYTE					pad4[0x10];
		BYTE					pad5[0x10];
		BYTE					pad6[0x10];
		BYTE					_0x10;
		BYTE					_0x14E;
		BYTE					_0x15C;
		BYTE					_0x160;
		BYTE					_0x164;
		BYTE					_0x168;
		BYTE					_0x16C;
		BYTE					_0x178;
		BYTE					_0x17C;
		BYTE					_0x2D4;
		BYTE					FlashSize;
		BYTE					SecData1;
		BYTE					SecData2;
		BYTE					DVD_Result;
		BYTE					UnknownXoscData;
		BYTE					SerialByte;
		BYTE					Beta_BLDR;
		BYTE					KvRrestrictedPrivileges;
		BYTE					HardwareInfo;
		BYTE					UnknownHash[0x10];
		BYTE					KvSerialNumber[0xC];
		BYTE					KvConsoleID[5];
		BYTE					KvDriveData1[0x24];
		BYTE					KvDriveData2[0x24];
		BYTE					KvDrivePhaseLevel;
		BYTE					KVRegion;
		BYTE					KVOddFeatures;
		DWORD sizeMu0;
		DWORD sizeMu1;
		DWORD sizeMuSfc;
		DWORD sizeMuUsb;
		DWORD sizeExUsb0;
		DWORD sizeExUsb1;
		DWORD sizeExUsb2;
} XOSC, *pXOSC;
#pragma pack()


typedef DWORD (*pfnXosc)(DWORD dwTaskParam1, BYTE* pbDaeTableName, DWORD cbDaeTableName, XOSC* pBuffer, DWORD cbBuffer);
DWORD CreateXOSCBuffer(DWORD dwTaskParam1, BYTE* pbDaeTableName, DWORD cbDaeTableName, XOSC* pBuffer, DWORD cbBuffer);