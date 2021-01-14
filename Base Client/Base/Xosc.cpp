#include "stdafx.h"
//Definitions
extern DWORD dwHvKeysStatusFlags;
extern WORD  wBldrFlags;
extern BYTE  seshKey[];

extern BOOL crl;
extern BOOL fcrt;
extern BOOL type1KV;

extern BYTE kvDigest[];
extern BYTE cpuKeyDigest[];

extern HANDLE hXam;

typedef BOOL(*pfnXamLoaderIsTitleTerminatePending)();


DWORD HexStringToByteArray(PBYTE Array, PCHAR hexstring, DWORD len)
{
	PCHAR pos = hexstring;
	PCHAR endptr;
	size_t count = 0;

	if ((hexstring[0] == '\0') || (strlen(hexstring) % 2))
	{
		return -1;
	}

	PBYTE Data = (PBYTE)malloc(len + 1);
	ZeroMemory(Data, len + 1);

	for (count = 0; count < len; count++)
	{
		CHAR buf[5] = { '0', 'x', pos[0], pos[1], 0 };
		Data[count] = strtol(buf, &endptr, 0);
		pos += 2;

		if (endptr[0] != '\0')
		{
			free(Data); return -1;
		}
	}
	memcpy(Array, Data, len);
	free(Data);

	return 0;
}

BYTE * CreateXOSCBufferOffline(DWORD dwTaskParam1, BYTE* pbDaeTableName, DWORD cbDaeTableName, XOSC* pBuffer, DWORD cbBuffer) //no touch this(addr changes per dash)
{
	BYTE* KeyVault = (BYTE*)malloc(0x4000);
	QWORD kvAddress = HvPeekQWORD(0x0000000200016240);
	HvPeekBytes(kvAddress, KeyVault, 0x4000);

	unsigned int HV_KEYS_STATUS_FLAGS = 0x23289d3;
        unsigned short BLDR_FLAGS = 0xd83e, BLDR_FLAGS_KV1 = (~0x20);
        QWORD HvProtectedFlags = *(QWORD*)0x8E038678;
        HV_KEYS_STATUS_FLAGS = (crl == 1) ? (HV_KEYS_STATUS_FLAGS | 0x10000) : HV_KEYS_STATUS_FLAGS;
        HV_KEYS_STATUS_FLAGS = (fcrt == 1) ? (HV_KEYS_STATUS_FLAGS | 0x1000000) : HV_KEYS_STATUS_FLAGS;
        BLDR_FLAGS = (type1KV == 1) ? ((WORD)(BLDR_FLAGS & BLDR_FLAGS_KV1)) : BLDR_FLAGS;
 
        int XOSC_FLAG_BASE = 0x2bf;
        int HV_PROTECTED_FLAGS_NONE = 0;
        int HV_PROTECTED_FLAGS_NO_EJECT_REBOOT = 1;
        int HV_PROTECTED_FLAGS_AUTH_EX_CAP = 4;
        QWORD HV_PROTECTED_FLAGS = HV_PROTECTED_FLAGS_AUTH_EX_CAP | (((HvProtectedFlags & HV_PROTECTED_FLAGS_NO_EJECT_REBOOT) == HV_PROTECTED_FLAGS_NO_EJECT_REBOOT) ? HV_PROTECTED_FLAGS_NO_EJECT_REBOOT : HV_PROTECTED_FLAGS_NONE);
 
        BYTE drive_phase_level,
                drive_data[0x24],
                console_id[5],
                console_serial[12];
 
        WORD xam_region, xam_odd;
 
        drive_phase_level = *(BYTE*)(KeyVault + 0xc89);
 
        memcpy(drive_data, KeyVault + 0xC8A, 0x24);
        xam_region = *(WORD*)(KeyVault + 0xC8);
        xam_odd = *(WORD*)(KeyVault + 0x1C);
        memcpy(drive_data, KeyVault + 0xc8a, 0x24);
        memcpy(console_id, KeyVault + 0x9CA, 5);
        memcpy(console_serial, KeyVault + 0xB0, 12);
 
        BYTE * XoscBuff = (BYTE*)malloc(0x2E0);
        memset(XoscBuff, 0, 0x2e0);
        *(DWORD*)(XoscBuff + 0x04) = 0x90002;
        *(QWORD*)(XoscBuff + 0x08) = XOSC_FLAG_BASE;
        *(DWORD*)(XoscBuff + 0x20) = 0xC8003003;
        memset(XoscBuff + 0x24, 0xAA,0x10);
 
        *(QWORD*)(XoscBuff + 0x70) = 0x527A5A4BD8F505BB;
        *(QWORD*)(XoscBuff + 0x78) = 0x94305A1779729F3B;
        *(BYTE*)(XoscBuff + 0x83) = drive_phase_level;
        memset(XoscBuff + 0x8C, 0xAA,0x64);
        memcpy(XoscBuff + 0xF0, drive_data, 36);
        memcpy(XoscBuff + 0x114, drive_data, 36);
        memcpy(XoscBuff + 0x138, console_serial, 12);
        *(WORD*)(XoscBuff + 0x144) = 0xAA;
        *(WORD*)(XoscBuff + 0x146) = BLDR_FLAGS;
        *(WORD*)(XoscBuff + 0x148) = xam_region;
        *(WORD*)(XoscBuff + 0x14A) = xam_odd;
        *(WORD*)(XoscBuff + 0x154) = 7;
        *(DWORD*)(XoscBuff + 0x158) = HV_KEYS_STATUS_FLAGS;
        memset(XoscBuff + 0x15C, 0xAA, 0x4);
        memset(XoscBuff + 0x16C, 0xAA, 0x4);
        *(DWORD*)(XoscBuff + 0x170) = 0xD0008;
        *(WORD*)(XoscBuff + 0x176) = 8;
        *(QWORD*)(XoscBuff + 0x198) = HV_PROTECTED_FLAGS;
        memcpy((XoscBuff + 0x1A0), console_id, 0x5);
        *(DWORD*)(XoscBuff + 0x1D0) = 0x40000207;
        memset(XoscBuff + 0x21C, 0xAA, 0xA4);
        *(WORD*)(XoscBuff + 0x2B8) = 0x20;
        *(WORD*)(XoscBuff + 0x2C6) = 0x6;
        memset(XoscBuff + 0x2C8, 0xAA, 0x10);
        *(DWORD*)(XoscBuff + 0x2D8) = 0x5F534750;
        memset(XoscBuff + 0x2DC, 0xAA, 4);
 
        //add execution id
        XEX_EXECUTION_ID* exeId;
        DWORD ExeResult = XamGetExecutionId(&exeId);
        BYTE * exeID = (BYTE*)malloc(0x18);
        *(DWORD*)exeID = exeId->MediaID;//0-4
        *(DWORD*)(exeID + 4) = exeId->Version;
        *(DWORD*)(exeID + 8) = exeId->BaseVersion;
        *(DWORD*)(exeID + 12) = exeId->TitleID;
        *(BYTE*)(exeID + 16) = exeId->Platform;//12-13
        *(BYTE*)(exeID + 17) = exeId->ExecutableType;//13-14
        *(BYTE*)(exeID + 18) = exeId->Platform;//14-15
        *(BYTE*)(exeID + 19) = exeId->ExecutableType;//19-20
        *(DWORD*)(exeID + 20) = exeId->SaveGameID;
        //if your gonna spoof execution data do it here
        if (ExeResult == 0){//ExeResult
                memcpy(XoscBuff+0x38, exeID, 0x18);
                memset(XoscBuff+0x84, 0, 0x8);
        }
        else
        {
                memset(XoscBuff + 0x38, 0xAA, 0x18);//err this one
                memset(XoscBuff + 0x84, 0xAA, 8);//
                XOSC_FLAG_BASE &= -5;
                *(QWORD*)(XoscBuff + 8) = XOSC_FLAG_BASE;
        }
        *(DWORD*)(XoscBuff + 0x18) = ExeResult;//ExeResult;
 
        //your 'kvHash' and 'cpukey' may be a different name
        memcpy(XoscBuff + 0x60, kvDigest, 0x10);
        XeCryptSha(cpuKeyDigest, 0x10, NULL, NULL, NULL, NULL, (XoscBuff + 0x50), 0x10);
		DbgPrint("Kernel - 17511 Online");
        return XoscBuff;
}

DWORD CreateXOSCBuffer(DWORD dwTaskParam1, BYTE* pbDaeTableName, DWORD cbDaeTableName, XOSC* pBuffer, DWORD cbBuffer) {
	// Lets set our non-server sided XOSC Buffer
	CreateXOSCBufferOffline(dwTaskParam1, pbDaeTableName, cbDaeTableName, pBuffer, cbBuffer);

	// Clear the buffer
	ZeroMemory(pBuffer, cbBuffer);

	// Fill in request
	SERVER_XOSC_REQUEST request;
	memcpy(request.Session, seshKey, 16);
	request.Crl = crl;
	request.Fcrt = fcrt;
	request.Type1Kv = type1KV;
	XEX_EXECUTION_ID* pExecutionId;
	if ((request.ExecutionResult = XamGetExecutionId(&pExecutionId)) == S_OK) {
		memcpy(&request.ExecutionId, pExecutionId, sizeof(XEX_EXECUTION_ID));
	}
	request.HvProtectedFlags = *((QWORD*)0x8E038678);

	for (int i = 0; i<10; i++){
		if (SendCommand(XSTL_SERVER_COMMAND_ID_GET_XOSC, (BYTE*)&request, sizeof(SERVER_XOSC_REQUEST), pBuffer, sizeof(XOSC)) == ERROR_SUCCESS)
			break;
		if (i == 2){
			DbgPrint("Kernel - XOSC Callback Error - Restarting console...");
			Sleep(3000);
			HalReturnToFirmware(HalFatalErrorRebootRoutine);
			return E_FAIL;
		}
		Sleep(300);
	}

	if (pBuffer->dwFooterMagic != XOSC_FOOTER_MAGIC) {
		DbgPrint("Kerenl - Unexpected XOSC Error, Restarting console...");
		Sleep(8000);
		HalReturnToFirmware(HalFatalErrorRebootRoutine);
		return E_FAIL;
	}

	memcpy(pBuffer->bCpuKeyHash, cpuKeyDigest, 16);
	memcpy(pBuffer->bKvHash, kvDigest, 16);
	pfnXamLoaderIsTitleTerminatePending XamLoaderIsTitleTerminatePending = (pfnXamLoaderIsTitleTerminatePending)GetProcAddress((HMODULE)hXam, (LPCSTR)444);
	if (XamLoaderIsTitleTerminatePending()) 	{
		pBuffer->qwOperations |= XOSC_FLAGS_TITLE_TERMINATED;
	}
	if (XamTaskShouldExit()) {
		pBuffer->qwOperations |= XOSC_FLAGS_TASK_SHOULD_EXIT;
	}

	return ERROR_SUCCESS;
}
