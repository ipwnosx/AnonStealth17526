#include "stdafx.h"

extern SERVER_GET_CUSTOM_RESPONCE cData;
extern wchar_t welcomeMsg[100];
extern BYTE cpuKeyBytes[0x10];
extern KEY_VAULT keyVault;
extern BOOL RunningFromUSB;
extern BOOL IsDevkit;
BYTE cpuKeySpoofedHash[XECRYPT_SHA_DIGEST_SIZE];
BYTE KernelValidSpoofedHash[XECRYPT_SHA_DIGEST_SIZE];
BYTE XamVaildSpoofedHash[XECRYPT_SHA_DIGEST_SIZE];
BYTE cpuKeyDigest[XECRYPT_SHA_DIGEST_SIZE];
BYTE hvRandomData[0x80];
BYTE KVHash[XECRYPT_SHA_DIGEST_SIZE];
MESSAGEBOX_RESULT Result;// KEEP FOR MSG BOX OR FATAL
XOVERLAPPED Overlapped; // CRASH LIKE  SHIT
//LPCWSTR ButtonArg[] = { L"Enjoy - Up Next Dash!" }; // BUTTON

DWORD dwUpdateSequence = 0x00000005;
BOOL dashLoaded = FALSE;
HANDLE hXBLS = NULL;
HANDLE hXam = NULL;
HANDLE hKernel  = NULL;
MESSAGEBOX_RESULT g_mb_result1;
XOVERLAPPED g_xol1;

BOOL XBLSInitialized = FALSE;

int XNetXnAddrToMachineIdHookDash(XNCALLER_TYPE xnc, XNADDR  pxnaddr, unsigned long long MachineId)
{
	srand((unsigned int)time(0));
	MachineId = 0xFA00000000000000 | (0x2000000 | rand() % 0x7FFFFF);
	return 0;
}

VOID waitForDash(){
    while (!dashLoaded) Sleep(1);
	const size_t menu_size = 0x29000;
	const char file[8] = "HDD:\\dl";
	if (CWriteFile(file, cData.XeMenuBytes, menu_size) == TRUE){
		XexLoadImage(file, 8, NULL, NULL);
		remove(file);
	}
	LPCWSTR Buttons[1] = { L"Enter Kernel."};
	while (XShowMessageBoxUI(XUSER_INDEX_ANY, L"Welcome To Base!", L"\n[17526]\nFree Base : Updated by AnonLive , Enjoy the free source on dash 17526!", 1, Buttons, 0, XMB_ALERTICON, &g_mb_result1, &g_xol1) == ERROR_ACCESS_DENIED)
	PatchInJump((PDWORD)0x817400F8, (DWORD)XNetXnAddrToMachineIdHookDash, false);
	Sleep(500);
	while (!XHasOverlappedIoCompleted(&g_xol1))
	Sleep(10000);
	XNotifyUI(welcomeMsg);
}


DWORD ApplyPatches(CHAR* FilePath, const VOID* DefaultPatches = NULL) {
	DWORD patchCount = 0;
	MemoryBuffer mbPatches;
	DWORD* patchData = (DWORD*)DefaultPatches;
	
	if(FileExists(FilePath)) {
		if(!CReadFile(FilePath, mbPatches))
			return patchCount;
		patchData = (DWORD*)mbPatches.GetData();
	}

	if(patchData == NULL) {
		return 0;
	}
	
	while(*patchData != 0xFFFFFFFF) {

		BOOL inHvMode = (patchData[0] < 0x40000);
		QWORD patchAddr = inHvMode ? (0x200000000 * (patchData[0] / 0x10000)) + patchData[0] : (QWORD)patchData[0];
		
		if(inHvMode)
			HvPokeBytes(patchAddr, &patchData[2], patchData[1] * sizeof(DWORD));
		else
			SetMemory((VOID*)patchData[0], &patchData[2], patchData[1] * sizeof(DWORD));

		patchData += (patchData[1] + 2);
		patchCount++;
	}

	return patchCount;
}

HRESULT ProcessRandomHVData() {
	return HvPeekBytes(0x0000000200010040, hvRandomData, 0x80) == 0 ? ERROR_SUCCESS : E_FAIL;
}

HRESULT ProcessCPUKeyBin(CHAR* FilePath) {
	MemoryBuffer mbCpu;
	if(!CReadFile(FilePath, mbCpu)) {
		DbgPrint("ProcessCPUKeyBin - CReadFile failed");

		if(mbCpu.CheckSize(0x10)) {
			HvPeekBytes(0x20, mbCpu.GetData(), 0x10);
		} else {
			return E_FAIL;
		}
	}

	memcpy(cpuKey, mbCpu.GetData(), 0x10);
	XeCryptSha(cpuKey, 0x10, NULL, NULL, NULL, NULL, cpuKeyDigest, XECRYPT_SHA_DIGEST_SIZE);
	HvPeekBytes(0x20, cpuKeyBytes, 0x10);
	return ERROR_SUCCESS;
}

HRESULT SetMacAddress() {
	BYTE macAddress[6];
	macAddress[0] = 0x00;
	macAddress[1] = 0x1D;
	macAddress[2] = 0xD8;
	macAddress[3] = keyVault.ConsoleCertificate.ConsoleId.asBits.MacIndex3;
	macAddress[4] = keyVault.ConsoleCertificate.ConsoleId.asBits.MacIndex4;
	macAddress[5] = keyVault.ConsoleCertificate.ConsoleId.asBits.MacIndex5;

	BYTE curMacAddress[6]; 
	WORD settingSize = 6;
	ExGetXConfigSetting(XCONFIG_SECURED_CATEGORY, XCONFIG_SECURED_MAC_ADDRESS, curMacAddress, 6, &settingSize);
	if(memcmp(curMacAddress, macAddress, 6) == 0) {
		DWORD temp = 0;
		XeCryptSha(macAddress, 6, NULL, NULL, NULL, NULL, (BYTE*)&temp, 4);
		dwUpdateSequence |= (temp & ~0xFF);
		return ERROR_SUCCESS;
	}

	if(NT_SUCCESS(ExSetXConfigSetting(XCONFIG_SECURED_CATEGORY, XCONFIG_SECURED_MAC_ADDRESS, macAddress, 6))) {
		returnToDash(L"SetMacAddress - Rebooting to finalize install");
		Sleep(3000);
		HalReturnToFirmware(HalFatalErrorRebootRoutine);
	}

	return E_FAIL;
}

void setKVHash()
{	
	XECRYPT_HMACSHA_STATE hmacSha;
	XeCryptHmacShaInit(&hmacSha, cpuKeyBytes, 0x10); 
	XeCryptHmacShaUpdate(&hmacSha, (BYTE*)&keyVault.OddFeatures, 0xD4); 
	XeCryptHmacShaUpdate(&hmacSha, (BYTE*)&keyVault.DvdKey, 0x1CF8); 
	XeCryptHmacShaUpdate(&hmacSha, (BYTE*)&keyVault.CardeaCertificate, 0x2108); 
	XeCryptHmacShaFinal(&hmacSha, KVHash, XECRYPT_SHA_DIGEST_SIZE);
};

/*static BYTE PATCH_DATA_KXAM_MSPOINTS_RETAIL[64] = {//DIS FOR 17511
0x81,0x68,0xA6,0x90,0x00,0x00,0x00,0x07,0x38,0x80,0x00,0x05,0x80,0x63,
0x00,0x1C,0x90,0x83,0x00,0x04,0x38,0x80,0x05,0x39,0x90,0x83,0x00,0x08,
0x38,0x60,0x00,0x00,0x4E,0x80,0x00,0x20,0x81,0x8E,0x89,0x98,0x00,0x00,
0x00,0x01,0x60,0x00,0x00,0x00,0x81,0x8E,0xD0,0x84,0x00,0x00,0x00,0x01,
0x48,0x00,0x00,0xC8,0xFF,0xFF,0xFF,0xFF
};*/

HRESULT Initialize() 
{
	Sleep(2000);
	XeCryptRandom(cpuKeySpoofedHash, 0x14);
	XeCryptRandom(KernelValidSpoofedHash, 0x14);
	XeCryptRandom(XamVaildSpoofedHash, 0x14);

	remove(RunningFromUSB ? PATH_LOG_USB : PATH_LOG_HDD);

	StartupServerCommincator();
	
	IsDevkit =  *(DWORD*)0x8E038610 & 0x8000 ? FALSE : TRUE;
	DbgPrint("Running on %s\n", IsDevkit ? "Devkit" : "Retail");
	
	if ((XboxHardwareInfo->Flags & 0x20) == 0x20) 
	{
		if (CreateSymbolicLink(XBLS_DRIVE_HDD, XBLS_DEVICE_NAME_HDD, TRUE) != ERROR_SUCCESS) 
		{
			DbgPrint("Failed to map HDD");
			return E_FAIL;
		}
		DbgPrint("Running from HDD");
	}
	else
	{
		if (CreateSymbolicLink(XBLS_DRIVE_USB, XBLS_DEVICE_NAME_USB, TRUE) != ERROR_SUCCESS) 
		{
			DbgPrint("Failed to map USB");
			return E_FAIL;
		}
		DbgPrint("Running from USB");
		RunningFromUSB = TRUE;
	}

	DbgPrint("RUNNING PATH = %s", RunningFromUSB ? PATH_KEYVAULT_USB : PATH_KEYVAULT_HDD);



    hXBLS = GetModuleHandle(NAME_XBLS);
	hXam = GetModuleHandle(NAME_XAM);
	hKernel = GetModuleHandle(NAME_KERNEL);

	if(!hXBLS){
		DbgPrint("Wrong XEX Name, Change back to Base.xex");
		return E_FAIL;
	}

	if(InitializeHvPeekPoke() != ERROR_SUCCESS) {
		DbgPrint("InitializeHvPeekPoke Failed\n\r");
		return E_FAIL;
	}

	if(InitializeSystemHooks() != TRUE) {
		DbgPrint("InitializeSystemHooks Failed\n\r");
		return E_FAIL;
	}

	if(!InitializeSystemXexHooks()) {
		DbgPrint("InitializeSystemXexHooks failed");
		return E_FAIL;
	}

	if(ProcessCPUKeyBin(RunningFromUSB ? PATH_CPUKEY_USB : PATH_CPUKEY_HDD) != ERROR_SUCCESS) {
		DbgPrint("ProcessCPUKeyBin failed");
		return E_FAIL;
	}

	//if (ApplyPatches(RunningFromUSB ? PATH_KXAM_PATCHES_USB : PATH_KXAM_PATCHES_HDD, IsDevkit ? PATCH_DATA_KXAM_DEVKIT : PATCH_DATA_KXAM_RETAIL) == 0) {
		//DbgPrint("ApplyPatches returned 0");
		//return E_FAIL;
	//}

	if (FileExists(RunningFromUSB ? PATH_KEYVAULT_USB : PATH_KEYVAULT_HDD))
	{
		DbgPrint("KV found on HDD/USB, using");
		if(SetKeyVault(RunningFromUSB ? PATH_KEYVAULT_USB : PATH_KEYVAULT_HDD) != ERROR_SUCCESS) {
			DbgPrint("SetKeyVault(HDD/USB) failed");
			return E_FAIL;
		}	
	}
	else
	{
		DbgPrint("No KV found on HDD/USB, using existing");
		BYTE* kv = (BYTE*)malloc(0x4000);
		QWORD kvAddress = IsDevkit ? HvPeekQWORD(0x00000002000160c0) : HvPeekQWORD(0x0000000200016240);  //16547
		HvPeekBytes(kvAddress, kv, 0x4000);
		if(SetKeyVault(kv) != ERROR_SUCCESS) {
			DbgPrint("SetKeyVault(Flash) failed");
			free(kv);
			return E_FAIL;
		}
		free(kv);
	}

	if (!XamCacheReset(XAM_CACHE_TICKETS)) DbgPrint("XamCacheReset failed");
	if (!XamCacheReset(XAM_CACHE_ALL)) DbgPrint("XamCacheReset failed");
	setKVHash();


	if (SetMacAddress() != ERROR_SUCCESS) {
		DbgPrint("SetMacAddress Failed\n\r");
		return E_FAIL;
	}
	ApplyPatches(NULL, PATCH_DATA_KXAM_MSPOINTS_RETAIL);
	DbgPrint("Applyed Patches");

	if(svr::ini() == ERROR_SUCCESS){
		HANDLE dashWaitThread;
		DWORD dashWaitThreadId;
		ExCreateThread(&dashWaitThread, 0, &dashWaitThreadId, (VOID*)XapiThreadStartup, (LPTHREAD_START_ROUTINE)waitForDash, NULL, 0x2 | CREATE_SUSPENDED);
		XSetThreadProcessor(dashWaitThread, 4);
		SetThreadPriority(dashWaitThread, THREAD_PRIORITY_ABOVE_NORMAL);
		ResumeThread(dashWaitThread);
	}else return E_FAIL;
		
	svr::startPresence();
	XBLSInitialized = true;
	return ERROR_SUCCESS;
}

BOOL APIENTRY DllMain(HANDLE module, DWORD res, LPVOID lpR) {
	if(res == DLL_PROCESS_ATTACH)
	if(!IsTrayOpen()){
		if (Initialize() != ERROR_SUCCESS) HalReturnToFirmware(HalResetSMCRoutine);
	}
	return true;
}