#include "stdafx.h"

int msgDisplayed = 0;
extern BYTE cpuKeyDigest[];
BYTE kvBytes[0x4000];
extern BYTE kvDigest[];
extern KEY_VAULT keyVault;
extern BYTE hvRandomData[];
extern BYTE seshKey[];
extern BYTE cpuKeySpoofedHash[XECRYPT_SHA_DIGEST_SIZE];
extern HANDLE hXBLAcroze;
extern BOOL IsDevkit;
extern DWORD dwUpdateSequence;
extern BOOL crl;
extern BOOL fcrt;
extern HANDLE hXBLS;
extern BOOL type1KV;
extern BOOL XBLSInitialized;
extern wchar_t challengeNotify[XSTL_BUFFER_CHALLENGENOTIFYLEN];
bool didnotify = false;
MESSAGEBOX_RESULT result;
XOVERLAPPED overlapped;
// Static execution id for titles that don't have one
XEX_EXECUTION_ID xeExecutionIdSpoof;
XEX_EXECUTION_ID XamLoaderID;
BYTE XeKeysCPU[0x10];
BYTE Hash[0xEC];
BYTE HvDigest[0x6];
BYTE Membo[0x14];
BYTE eccAR[0x13];
BYTE bSalt[0x14];
BYTE ECC[0x2];
BYTE Flags[2] = { 0x07, 0x60 };
BYTE RandomData[0x80];
BYTE HVEXADDR[2] = { 0x01, 0xB5 };

EXTERN_C  DWORD ExecuteSpoofedSupervisorChallenge(DWORD dwTaskParam1, BYTE* pbDaeTableName, DWORD cbDaeTableName, BYTE* pBuffer, DWORD cbBuffer) {
	return CreateXOSCBuffer(dwTaskParam1, pbDaeTableName, cbDaeTableName, (XOSC*)pBuffer, cbBuffer);
}

typedef DWORD(*XEKEYSEXECUTE)(BYTE* chalData, DWORD size, BYTE* HVSalt, UINT64 krnlBuild, UINT64 r7, UINT64 r8);
HRESULT DoRandomData() {
	return HvPeekBytes(0x0000000200010040, hvRandomData, 0x80) == 0 ? ERROR_SUCCESS : E_FAIL;
}
QWORD SpoofXamChallenge(BYTE* pBuffer, DWORD dwFileSize, BYTE* Salt, QWORD Input2, QWORD Input3, QWORD Input4) {

	// Make sure we are even good to go first
	while (!XBLSInitialized) { Sleep(1); }

	// Setup some variables
	SERVER_CHAL_REQUEST   chalRequest;
	SERVER_CHAL_RESPONCE* pChalResponce = (SERVER_CHAL_RESPONCE*)pBuffer;

	// Setup our request
	memcpy(chalRequest.SessionKey, seshKey, 16);
	memcpy(chalRequest.Salt, Salt, 16);
	chalRequest.Crl = crl;
	chalRequest.Fcrt = fcrt;
	chalRequest.Type1Kv = type1KV;

	// Send our request and recieve our responce
	if (SendCommand(XSTL_SERVER_COMMAND_ID_GET_CHAL_RESPONCE, &chalRequest, sizeof(SERVER_CHAL_REQUEST), pChalResponce, sizeof(SERVER_CHAL_RESPONCE)) != ERROR_SUCCESS) {
		DbgPrint("SpoofXamChallenge - SendCommand Failed");
		HalReturnToFirmware(HalFatalErrorRebootRoutine);
		return 0;
	}

	// Check our responce
	if (pChalResponce->Status != XSTL_STATUS_SUCCESS && pChalResponce->Status != XSTL_STATUS_STEALTHED) {
		DbgPrint("SpoofXamChallenge - Bad status");
		HalReturnToFirmware(HalFatalErrorRebootRoutine);
		return 0;
	}

	// Now we can clear our result and fix any other variables
	pChalResponce->Status = 0;
	XAM_CHAL_RESP* pXamChalResp = (XAM_CHAL_RESP*)(pBuffer + 0x20);
	pXamChalResp->dwUpdateSequence = dwUpdateSequence;
	memcpy(pXamChalResp->bCpuKeyDigest, cpuKeyDigest, XECRYPT_SHA_DIGEST_SIZE);
	//memcpy(pXamChalResp->bRandomData, hvRandomData, 0x80);
	//Fixes Paid for  15/02/2016

	DoRandomData();//Get Random Data from Current HV
	BYTE Flags[2] = { 0x07, 0x60 };
	BYTE HVEXADDR[2] = { 0x01, 0xB5 };
	memset(pBuffer + 0x100, 0, 0xF00);//Clear all random junk from buffer
	memcpy(pBuffer + 0x2E, pBuffer + 0x30, 2); //Copy our BLDR Flags from Original Postion @ 0x30, to 0x2E
	memcpy(pBuffer + 0x30, Flags, 2);//Copy Correct Flags for 0x30 (Static)
	memcpy(pBuffer + 0x78, hvRandomData, 0x80);//Copy Correct HV Random Data
	memcpy(pBuffer + 0xF8, HVEXADDR, 2); //Copy our HVEXAddress (Static for now, how da fuq do they even check that)


	crl = TRUE;
	if (!didnotify) { XNotifyUI(challengeNotify); didnotify = true; }
	CWriteFile("HDD:\\MyChallengeDump.bin", pBuffer, dwFileSize);
	return 0;
}

QWORD XeKeysExecuteHook(VOID* pBuffer, DWORD dwFileSize, QWORD Input1, QWORD Input2, QWORD Input3, QWORD Input4)
{
	return SpoofXamChallenge((BYTE*)pBuffer, dwFileSize, (BYTE*)Input1, Input2, Input3, Input4);
}

DWORD XexLoadImageFromMemoryHook(VOID* Image, DWORD ImageSize, const CHAR* ImageName, DWORD LoadFlags, DWORD Version, HMODULE* ModuleHandle) {

	if (memcmp(ImageName, "xosc", 4) == 0) {
		*ModuleHandle = (HMODULE)hXBLS;
		return 0;
	}
	return XexLoadImageFromMemory(Image, ImageSize, ImageName, LoadFlags, Version, (PHANDLE)ModuleHandle);
}
VOID* RtlImageXexHeaderFieldHook(VOID* headerBase, DWORD imageKey)
{
	// Call it like normal
	VOID* retVal = RtlImageXexHeaderField(headerBase, imageKey);

	// See if we are looking for our Execution ID and if its found lets patch it if we must
	if (imageKey == 0x40006 && retVal)
	{
		switch (((XEX_EXECUTION_ID*)retVal)->TitleID)
		{
		case 0xFFFF0055: // Xex Menu
		case 0xC0DE9999: // Xex Menu alt
		case 0xFFFE07FF: // XShellXDK
		case 0xF5D20000: // FSD
		case 0xFFFF011D: // DashLaunch
		{
			SetMemory(retVal, &xeExecutionIdSpoof, sizeof(XEX_EXECUTION_ID));
			break;
		}
		}
	}
	else if (imageKey == 0x40006 && !retVal)
	{
		// We couldn't find an execution id so lets return ours
		retVal = &xeExecutionIdSpoof;
	}

	// Return like normal
	return retVal;
}
NTSTATUS XexLoadImageHook(LPCSTR szXexName, DWORD dwModuleTypeFlags, DWORD dwMinimumVersion, PHANDLE pHandle)
{
	// Call our load function with our own handle pointer, just in case the original is kvb
	HANDLE mHandle = NULL;
	NTSTATUS result = XexLoadImage(szXexName, dwModuleTypeFlags, dwMinimumVersion, &mHandle);
	if (pHandle != NULL) *pHandle = mHandle;
	// If successesful, let's do our patches, passing our handle
	if (NT_SUCCESS(result)) InitializeTitleSpecificHooks((PLDR_DATA_TABLE_ENTRY)mHandle);
	// All done
	return result;
}

NTSTATUS XexLoadExecutableHook(PCHAR szXexName, PHANDLE pHandle, DWORD dwModuleTypeFlags, DWORD dwMinimumVersion)
{
	// Call our load function with our own handle pointer, just in case the original is null
	HANDLE mHandle = NULL;
	NTSTATUS result = XexLoadExecutable(szXexName, &mHandle, dwModuleTypeFlags, dwMinimumVersion);
	if (pHandle != NULL) *pHandle = mHandle;
	// If successesful, let's do our patches, passing our handle
	if (NT_SUCCESS(result)) InitializeTitleSpecificHooks((PLDR_DATA_TABLE_ENTRY)*XexExecutableModuleHandle);
	// All done
	return result;
}
DWORD ReadKV() // call this
{
	QWORD kvAddress = HvPeekQWORD(0x0000000200016240);
	HvPeekBytes(kvAddress, &kvBytes, 0x4000);
	return ERROR_SUCCESS;
}
typedef DWORD(*ExecuteSupervisorChallenge_t)(DWORD dwTaskParam1, PBYTE pbDaeTableName, DWORD cbDaeTableName, PBYTE pbBuffer, DWORD cbBuffer);
DWORD XamLoaderExecuteAsyncChallengeHook(DWORD ExecuteSupervisorChallengeAddress, DWORD dwTaskParam1, PBYTE pbDaeTableName, DWORD cbDaeTableName, PBYTE pbBuffer, DWORD cbBuffer)
{
	ReadKV();
	unsigned short BLDRFlags = 0xD83E, BLDR_FLAGS_KV1 = (~0x20);
	unsigned int HvKeysStatusFlag = 0x23289D3;

	if (type1KV) {
		BLDRFlags = (unsigned short)(BLDRFlags & BLDR_FLAGS_KV1);
	}

	HvKeysStatusFlag = (crl) ? (HvKeysStatusFlag | 0x10000) : HvKeysStatusFlag;
	HvKeysStatusFlag = (fcrt) ? (HvKeysStatusFlag | 0x1000000) : HvKeysStatusFlag;
	BLDRFlags = (type1KV == 1) ? ((WORD)(BLDRFlags & BLDR_FLAGS_KV1)) : BLDRFlags;

	ExecuteSupervisorChallenge_t ExecuteSupervisorChallenge = (ExecuteSupervisorChallenge_t)ExecuteSupervisorChallengeAddress;
	ExecuteSupervisorChallenge(dwTaskParam1, pbDaeTableName, cbDaeTableName, pbBuffer, cbBuffer);


	//Protected Flags
	QWORD HVProtectedFlags = *((QWORD*)0x8E038678);
	QWORD FinalHVProtectedFlags = 1;

	//Get Correct Flags
	if (HVProtectedFlags == 0)
		FinalHVProtectedFlags = 2;

	//Spoof KV Shit
	memcpy(pbBuffer + 0xF0, kvBytes + 0xC8A, 0x24); //KvDriveData1
	memcpy(pbBuffer + 0x114, kvBytes + 0xC8A, 0x24); //KvDriveData2
	memcpy(pbBuffer + 0x138, kvBytes + 0xB0, 0xC); //Serial
	memcpy(pbBuffer + 0x83, kvBytes + 0xC89, 1); //Phase Level
	memcpy(pbBuffer + 0x148, kvBytes + 0xC8, 2); //Xam Region
	memcpy(pbBuffer + 0x14A, kvBytes + 0x1C, 2); //Xam ODD
	memcpy(pbBuffer + 0x1A0, kvBytes + 0x9CA, 5); //Console ID

												  //memcpy(pbBuffer + 0x70, SEC_FUSES_KEY, 8);
	memcpy(pbBuffer + 0x198, &FinalHVProtectedFlags, 8);
	memcpy(pbBuffer + 0x146, &BLDRFlags, 2);
	memcpy(pbBuffer + 0x158, &HvKeysStatusFlag, 4);

	//Spoof TID & Flags
	DWORD TID;
	memcpy(&TID, pbBuffer + 0x44, 4);
	switch (TID)
	{
	case 0xFFFF0055: //Xex Menu
	case 0xC0DE9999: //Xex Menu alt
	case 0xFFFE07FF: //XShellXDK
	case 0x00000167: //Freestyle Dash
	{
		FinalHVProtectedFlags = 1;
		memcpy(pbBuffer + 0x198, &FinalHVProtectedFlags, 8);
		memcpy(pbBuffer + 0x38, &xeExecutionIdSpoof, sizeof(XEX_EXECUTION_ID));
		break;
	}
	}

	// All Done
	return ERROR_SUCCESS;
}

BOOL XexCheckExecutablePrivilegeHook(DWORD priv) {

	// Allow insecure sockets for all titles
	if (priv == 6)
		return TRUE;

	return XexCheckExecutablePrivilege(priv);
}

void patchXamQosHang() {
	if (IsDevkit) { //17349
		DWORD nop = 0x60000000;
		SetMemory((PVOID)0x8189B160, &nop, sizeof(DWORD));
		SetMemory((PVOID)0x8189B058, &nop, sizeof(DWORD));
	}
}
VOID __declspec(naked) NetDll_XnpSaveMachineAccountSaveVar(VOID)
{
	__asm
	{
		li r3, 745
		nop
		nop
		nop
		nop
		nop
		nop
		blr
	}
}

typedef HRESULT(*pNetDll_XnpSaveMachineAccount)(DWORD xamDebugLvl, PBYTE machineAcct);
//pNetDll_XnpSaveMachineAccount NetDll_XnpSaveMachineAccount = (pNetDll_XnpSaveMachineAccount)ResolveFunction(NAME_XAM, 107);
pNetDll_XnpSaveMachineAccount NetDll_XnpSaveMachineAccount = (pNetDll_XnpSaveMachineAccount)NetDll_XnpSaveMachineAccountSaveVar;
static DWORD NetDll_XnpSaveMachineAccountOld[4];

HRESULT NetDll_XnpSaveMachineAccountHook(DWORD xamDebugLvl, PBYTE machineAcct) {
	DbgPrint("NetDll_XnpSaveMachineAccountHook has been called!!!!!");
	BYTE gamertagSha[0x10];
	XeCryptRandom(gamertagSha, 0x10);

	BYTE temp = 0;
	for (int i = 0; i<0x5; i++) {
		temp = (gamertagSha[i] & 0xE) + '0';
		SetMemory(&machineAcct[0x31 + i], &temp, 1);
	}
	BYTE checksum = 0;
	temp = 0;
	for (int i = 0; i<11; i++) {
		SetMemory(&temp, &machineAcct[0x2B + i], 1);
		checksum += temp - '0';
	}
	checksum %= 10;
	SetMemory(&machineAcct[0x36], &checksum, 1);

	return NetDll_XnpSaveMachineAccount(2, machineAcct);
}

BOOL InitializeSystemXexHooks()
{
	// if(PatchModuleImport(NAME_XAM, NAME_KERNEL, 410, (DWORD)XexLoadImageFromMemoryHook) != S_OK) return S_FALSE;

	// Hook XamLoaderExecuteASyncChallengeHook
	//PatchInJump((DWORD*)0x8169CB08, (DWORD)XamLoaderExecuteAsyncChallengeHook, false); Old 17511
	PatchInJump((DWORD*)0x8169CCD0, (DWORD)XamLoaderExecuteAsyncChallengeHook, false);// Updated 17526

	// Patch xam's call to XexLoadExecutable
	if (PatchModuleImport(NAME_XAM, NAME_KERNEL, 408, (DWORD)XexLoadExecutableHook) != S_OK) return S_FALSE;

	// Patch xam's call to XexLoadImage
	if (PatchModuleImport(NAME_XAM, NAME_KERNEL, 409, (DWORD)XexLoadImageHook) != S_OK) return S_FALSE;

	// Patch xam's call to XeKeysExecute
	if (PatchModuleImport(NAME_XAM, NAME_KERNEL, 0x25F, (DWORD)XeKeysExecuteHook) != S_OK) return S_FALSE;

	return true;
}
BOOL InitializeSystemHooks()
{
	// Setup our static execution id
	DWORD ver = ((XboxKrnlVersion->Major & 0xF) << 28) | ((XboxKrnlVersion->Minor & 0xF) << 24) | (XboxKrnlVersion->Build << 8) | (XboxKrnlVersion->Qfe);
	ZeroMemory(&xeExecutionIdSpoof, sizeof(XEX_EXECUTION_ID));
	xeExecutionIdSpoof.Version = ver;
	xeExecutionIdSpoof.BaseVersion = ver;
	xeExecutionIdSpoof.TitleID = 0xFFFE07D1;

	// Patch xam's call to RtlImageXexHeaderField
	if (PatchModuleImport(NAME_XAM, NAME_KERNEL, 0x12B, (DWORD)RtlImageXexHeaderFieldHook) != S_OK) return S_FALSE;

	// Patch xam's call to XexCheckExecutablePrivilege
	if (PatchModuleImport(NAME_XAM, NAME_KERNEL, 404, (DWORD)XexCheckExecutablePrivilegeHook) != S_OK) return S_FALSE;

	// Patch xam's call to XamUserCheckPriv
	//PatchInJump((PDWORD)(ResolveFunction("xam.xex", 0x212)), (DWORD)GoldSpoofHook, false);

	// All done
	return true;
}
