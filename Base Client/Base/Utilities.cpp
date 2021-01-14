#include "stdafx.h"

// Resolve set memory
pDmSetMemory DevSetMemory = NULL;
BOOL dbgInit = FALSE;
extern BOOL RunningFromUSB;

#ifdef _DEBUG

CRITICAL_SECTION dbgLock;

VOID DbgPrint( const CHAR* strFormat, ... ) {

	if(dbgInit == FALSE) {
		InitializeCriticalSection(&dbgLock);
		dbgInit = TRUE;
	}	

	CHAR buffer[ 1000 ];

	va_list pArgList;
	va_start( pArgList, strFormat );
	vsprintf_s( buffer, 1000, strFormat, pArgList );
	va_end(pArgList);

    printf("Kernel %s\n", buffer);

	EnterCriticalSection(&dbgLock);
	std::ofstream writeLog;
	writeLog.open(RunningFromUSB ? PATH_LOG_USB : PATH_LOG_HDD, std::ofstream::app);
	if (writeLog.is_open())	
	{
		writeLog.write(buffer, strlen(buffer));
		writeLog.write("\r\n", 2);
	}
	writeLog.close();
	LeaveCriticalSection(&dbgLock);
}

#endif


VOID returnToDashThread(){
	Sleep(3000);
	XSetLaunchData(NULL, 0);
	XamLoaderLaunchTitleEx( XLAUNCH_KEYWORD_DEFAULT_APP, NULL, NULL, 0);
}


static WCHAR sysMsgBuffer[100];
VOID sysMsgThreadDelay(WCHAR* msg){
	//while(!dashLoaded){
	//	Sleep(2500);
	//}
	Sleep(8000);
	XNotifyUI(msg);
}

VOID sysMsgThread(WCHAR* msg){
	XNotifyUI(msg);
}

VOID launchSysMsg(WCHAR* msg, int delay){
	memcpy(sysMsgBuffer, msg, (wcslen(msg)*sizeof(WCHAR))+2);
	HANDLE hThread;
	DWORD dwThreadId;
	if(delay!=60000 && delay>0){
		Sleep(delay);
		ExCreateThread(&hThread, 0, &dwThreadId, (VOID*) XapiThreadStartup , (LPTHREAD_START_ROUTINE)sysMsgThread, sysMsgBuffer, 0x2);
	}else
		ExCreateThread(&hThread, 0, &dwThreadId, (VOID*) XapiThreadStartup , (LPTHREAD_START_ROUTINE)sysMsgThreadDelay, sysMsgBuffer, 0x2);

	XSetThreadProcessor( hThread, 4 );
	ResumeThread(hThread);
	CloseHandle(hThread);
}

VOID returnToDash(WCHAR* msg){
	HANDLE hThread;
	DWORD dwThreadId;
	hThread = CreateThread( 0, 0, (LPTHREAD_START_ROUTINE)returnToDashThread, 0, CREATE_SUSPENDED, &dwThreadId );
	XSetThreadProcessor(hThread, 4);
	SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
	ResumeThread(hThread);
	CloseHandle(hThread);
	if(msg) launchSysMsg(msg, 0);
	Sleep(500);
}


PWCHAR charToWChar(__in LPCSTR c_buffer) {
	int wchars_num = MultiByteToWideChar(CP_ACP, 0, c_buffer, -1, NULL, 0);
	PWCHAR c_wbuffer = new WCHAR[wchars_num];
	MultiByteToWideChar(CP_ACP, 0, c_buffer, -1, (LPWSTR)c_wbuffer, wchars_num);
	return c_wbuffer;
}

int toWCHAR(char* input, WCHAR* output){
	if(!input || !output) return 0;
	int len = strlen(input);

	memset(output, 0, (len*2)+2); //convert to wide string because xdk functions don't fucking work
	for(int i=1, b=0; b<len; i+=2){
		((char*)output)[i]=input[b];
		b++;
	}
	return len;
}

BOOL XeKeysPkcs1Verify(const BYTE* pbHash, const BYTE* pbSig, XECRYPT_RSA* pRsa) {
	BYTE scratch[256];
	DWORD val = pRsa->cqw << 3;
	if (val <= 0x200) {
		XeCryptBnQw_SwapDwQwLeBe((QWORD*)pbSig, (QWORD*)scratch, val >> 3);
		if (XeCryptBnQwNeRsaPubCrypt((QWORD*)scratch, (QWORD*)scratch, pRsa) == 0) return FALSE;
		XeCryptBnQw_SwapDwQwLeBe((QWORD*)scratch, (QWORD*)scratch, val >> 3);
		return XeCryptBnDwLePkcs1Verify((const PBYTE)pbHash, scratch, val);
	}
	else return FALSE;
}

VOID PatchInJump(DWORD* Address, DWORD Destination, BOOL Linked) {
	Address[0] = 0x3D600000 + ((Destination >> 16) & 0xFFFF);
	if(Destination & 0x8000) Address[0] += 1;
	Address[1] = 0x396B0000 + (Destination & 0xFFFF);
	Address[2] = 0x7D6903A6;
	Address[3] = Linked ? 0x4E800421 : 0x4E800420;
}

VOID PatchInBranch(DWORD* Address, DWORD Destination, BOOL Linked) {
	Address[0] = (0x48000000 + ((Destination - (DWORD)Address) & 0x3FFFFFF));
	if(Linked) Address[0] += 1;
}

FARPROC ResolveFunction(CHAR* ModuleName, DWORD Ordinal) {
	HMODULE mHandle = GetModuleHandle(ModuleName);
	return (mHandle == NULL) ? NULL : GetProcAddress(mHandle, (LPCSTR)Ordinal);
}
DWORD PatchModuleImport(CHAR* Module, CHAR* ImportedModuleName, DWORD Ordinal, DWORD PatchAddress) {
	LDR_DATA_TABLE_ENTRY* moduleHandle = (LDR_DATA_TABLE_ENTRY*)GetModuleHandle(Module);
	return (moduleHandle == NULL) ? S_FALSE : PatchModuleImport(moduleHandle, ImportedModuleName, Ordinal, PatchAddress);
}
DWORD PatchModuleImport(PLDR_DATA_TABLE_ENTRY Module, CHAR* ImportedModuleName, DWORD Ordinal, DWORD PatchAddress) {

	// First resolve this imports address
	DWORD address = (DWORD)ResolveFunction(ImportedModuleName, Ordinal);
	if(address == NULL)
		return S_FALSE;

	// Get our header field from this module
	VOID* headerBase = Module->XexHeaderBase;
	PXEX_IMPORT_DESCRIPTOR importDesc = (PXEX_IMPORT_DESCRIPTOR)RtlImageXexHeaderField(headerBase, 0x000103FF);
	if(importDesc == NULL)
		return S_FALSE;

	// Our result
	DWORD result = 2; // No occurances patched

	// Get our string table position
	CHAR* stringTable = (CHAR*)(importDesc + 1);
	
	// Get our first entry
	XEX_IMPORT_TABLE_ORG* importTable = (XEX_IMPORT_TABLE_ORG*)(stringTable + importDesc->NameTableSize);

	// Loop through our table
	for(DWORD x = 0; x < importDesc->ModuleCount; x++) {
		
		// Go through and search all addresses for something that links
		DWORD* importAdd = (DWORD*)(importTable + 1);
		for(DWORD y = 0; y < importTable->ImportTable.ImportCount; y++) {

			// Check the address of this import
			DWORD value = *((DWORD*)importAdd[y]);
			if(value == address) {

				// We found a matching address address
				SetMemory((DWORD*)importAdd[y], &PatchAddress, 4);
				DWORD newCode[4];
				PatchInJump(newCode, PatchAddress, FALSE);
				SetMemory((DWORD*)importAdd[y + 1], newCode, 16);

				// We patched at least one occurence
				result = S_OK;
			}
		}

		// Goto the next table
		importTable = (XEX_IMPORT_TABLE_ORG*)(((BYTE*)importTable) + importTable->TableSize);
	}

	// Return our result
	return result;
}

HRESULT CreateSymbolicLink(CHAR* szDrive, CHAR* szDeviceName, BOOL System) {

	// Setup our path
	CHAR szDestinationDrive[MAX_PATH];
	sprintf_s(szDestinationDrive, MAX_PATH, System ? "\\System??\\%s" : "\\??\\%s", szDrive);

	// Setup our strings
	ANSI_STRING linkname, devicename;
	RtlInitAnsiString(&linkname, szDestinationDrive);
	RtlInitAnsiString(&devicename, szDeviceName);

	//check if already mapped
	if(FileExists(szDrive))
		return S_OK;

	// Create finally
	NTSTATUS status = ObCreateSymbolicLink(&linkname, &devicename);
	return (status >= 0) ? S_OK : S_FALSE;
}
HRESULT DeleteSymbolicLink(CHAR* szDrive, BOOL System) {

	// Setup our path
	CHAR szDestinationDrive[MAX_PATH];
	sprintf_s(szDestinationDrive, MAX_PATH, System ? "\\System??\\%s" : "\\??\\%s", szDrive);

	// Setup our string
	ANSI_STRING linkname;
	RtlInitAnsiString(&linkname, szDestinationDrive);
	
	// Delete finally
	NTSTATUS status = ObDeleteSymbolicLink(&linkname);
	return (status >= 0) ? S_OK : S_FALSE;
}
BOOL CReadFile(const CHAR * FileName, MemoryBuffer &pBuffer) {

	HANDLE hFile; DWORD dwFileSize, dwNumberOfBytesRead;
	hFile = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE) {
		DbgPrint("CReadFile - CreateFile failed");
		return FALSE;
	}
	dwFileSize = GetFileSize(hFile, NULL);
	PBYTE lpBuffer = (BYTE*)malloc(dwFileSize);
	if(lpBuffer == NULL) {
		CloseHandle(hFile);
		DbgPrint("CReadFile - malloc failed");
		return FALSE;
	}
	if(ReadFile(hFile, lpBuffer, dwFileSize, &dwNumberOfBytesRead, NULL) == FALSE) {
		free(lpBuffer);
		CloseHandle(hFile);
		DbgPrint("CReadFile - ReadFile failed");
		return FALSE;
	}
	else if (dwNumberOfBytesRead != dwFileSize) {
		free(lpBuffer);
		CloseHandle(hFile);
		DbgPrint("CReadFile - Failed to read all the bytes");
		return FALSE;
	}
	CloseHandle(hFile);
	pBuffer.Add(lpBuffer, dwFileSize);
	free(lpBuffer);
	return TRUE;
}
BOOL CWriteFile(const CHAR* FilePath, const VOID* Data, DWORD Size) {
	
	// Open our file
	HANDLE fHandle = CreateFile(FilePath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(fHandle == INVALID_HANDLE_VALUE) {
		DbgPrint("CWriteFile - CreateFile failed");
		return FALSE;
	}

	// Write our data and close
	DWORD writeSize = Size;
	if(WriteFile(fHandle, Data, writeSize, &writeSize, NULL) != TRUE) {
		DbgPrint("CWriteFile - WriteFile failed");
		return FALSE;
	}
	CloseHandle(fHandle);

	// All done
	return TRUE;
}
BOOL FileExists(LPCSTR lpFileName) {

	// Try and get the file attributes.
	if(GetFileAttributes(lpFileName) == -1) {
		DWORD lastError = GetLastError();
		if(lastError == ERROR_FILE_NOT_FOUND || lastError == ERROR_PATH_NOT_FOUND)
			return FALSE;
	}

	// The file must exist if we got this far..
	return TRUE;
}

BOOL IsTrayOpen() { 
	unsigned char msg[0x10]; 
	unsigned char resp[0x10]; 
	memset(msg, 0x0, 0x10); 
	msg[0] = 0xa; 
	HalSendSMCMessage(msg, resp); 

	if (resp[1] ==  0x60) 
		return true; 
	else 
		return false; 
}

BOOL pfShow = (BOOL)0xDEADBEEF;  //flag to init values
BOOL pfShowMovie;
BOOL pfPlaySound;
BOOL pfShowIPTV;

VOID toggleNotify(BOOL on){
	if((int)pfShow==0xDEADBEEF) //init values
		XNotifyUIGetOptions(&pfShow, &pfShowMovie, &pfPlaySound, &pfShowIPTV);
	
	if(!on){
		//XNotifyUISetOptions(false, false, false, true);
		XNotifyUISetOptions(pfShow, pfShowMovie, pfPlaySound, pfShowIPTV);  //set back original values
	}else{
		XNotifyUISetOptions(true, true, true, true);  //turn on notifications so XBLSE msgs always show..
	}
	Sleep(500);
}

VOID XNotifyDoQueueUI(PWCHAR pwszStringParam) {
	toggleNotify(true);
	XNotifyQueueUI(XNOTIFYUI_TYPE_PREFERRED_REVIEW, XUSER_INDEX_ANY, XNOTIFYUI_PRIORITY_HIGH, pwszStringParam, NULL);
	toggleNotify(false);
}

//Use this one!
VOID XNotifyUI(PWCHAR pwszStringParam)
{
	if (KeGetCurrentProcessType() != PROC_USER)
	{
		HANDLE th = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)XNotifyDoQueueUI, (LPVOID)pwszStringParam, CREATE_SUSPENDED, NULL);
		if (th == NULL) return;
		ResumeThread(th);
	}
	else
		XNotifyDoQueueUI(pwszStringParam);	
}

HRESULT SetMemory(VOID* Destination, VOID* Source, DWORD Length) {

	// Try to resolve our function
	if(DevSetMemory == NULL)
		DevSetMemory = (pDmSetMemory)ResolveFunction("xbdm.xex", 40);
	
	// Now lets try to set our memory
	if(DevSetMemory == NULL) {
		memcpy(Destination, Source, Length);
		return ERROR_SUCCESS;
	} else {
		if(DevSetMemory(Destination, Length, Source, NULL) == MAKE_HRESULT(0, 0x2da, 0))
			return ERROR_SUCCESS;
	}

	// We have a problem..
	return E_FAIL;
}

VOID __declspec(naked) GLPR(VOID)
{
	__asm
	{
		std     r14, -0x98(sp)
		std     r15, -0x90(sp)
		std     r16, -0x88(sp)
		std     r17, -0x80(sp)
		std     r18, -0x78(sp)
		std     r19, -0x70(sp)
		std     r20, -0x68(sp)
		std     r21, -0x60(sp)
		std     r22, -0x58(sp)
		std     r23, -0x50(sp)
		std     r24, -0x48(sp)
		std     r25, -0x40(sp)
		std     r26, -0x38(sp)
		std     r27, -0x30(sp)
		std     r28, -0x28(sp)
		std     r29, -0x20(sp)
		std     r30, -0x18(sp)
		std     r31, -0x10(sp)
		stw     r12, -0x8(sp)
		blr
	}
}
DWORD RelinkGPLR(DWORD SFSOffset, PDWORD SaveStubAddress, PDWORD OriginalAddress)
{
	DWORD Instruction = 0, Replacing;
	PDWORD Saver = (PDWORD)GLPR;
	if(SFSOffset & 0x2000000)
	{
		SFSOffset = SFSOffset | 0xFC000000;
	}
	Replacing = OriginalAddress[SFSOffset / 4];
	for(int i = 0; i < 20; i++)
	{
		if(Replacing == Saver[i])
		{
			DWORD NewOffset = (DWORD)&Saver[i]-(DWORD)SaveStubAddress;
			Instruction = 0x48000001 | (NewOffset & 0x3FFFFFC);
		}
	}
	return Instruction;
}
VOID HookFunctionStart(PDWORD Address, PDWORD SaveStub, DWORD Destination)
{
	if((SaveStub != NULL) && (Address != NULL)) // Make sure they are not nothing.
	{
		DWORD AddressRelocation = (DWORD)(&Address[4]); // Replacing 4 instructions with a jump, this is the stub return address
		if(AddressRelocation & 0x8000)
		{
			SaveStub[0] = 0x3D600000 + (((AddressRelocation >> 16) & 0xFFFF) + 1); // lis r11, 0 | Load Immediate Shifted
		}
		else
		{
			SaveStub[0] = 0x3D600000 + ((AddressRelocation >> 16) & 0xFFFF); // lis r11, 0 | Load Immediate Shifted
		}
		SaveStub[1] = 0x396B0000 + (AddressRelocation & 0xFFFF); // addi r11, r11, (value of AddressRelocation & 0xFFFF) | Add Immediate
		SaveStub[2] = 0x7D6903A6; // mtspr CTR, r11 | Move to Special-Purpose Register CTR
		// Instructions [3] through [6] are replaced with the original instructions from the function hook
		// Copy original instructions over, relink stack frame saves to local ones
		for(int i = 0; i < 4; i++)
		{
			if((Address[i] & 0x48000003) == 0x48000001)
			{
				SaveStub[i + 3] = RelinkGPLR((Address[i] & ~0x48000003), &SaveStub[i + 3], &Address[i]);
			}
			else
			{
				SaveStub[i + 3] = Address[i];
			}
		}
		SaveStub[7] = 0x4E800420; // Branch unconditionally
		__dcbst(0, SaveStub); // Data Cache Block Store | Allows a program to copy the contents of a modified block to main memory.
		__sync(); // Synchronize | Ensure the dcbst instruction has completed.
		__isync(); // Instruction Synchronize | Refetches any instructions that might have been fetched prior to this instruction.
		PatchInJump(Address, Destination, FALSE); // Redirect Function to ours

		/*
		* So in the end, this will produce:
		*
		* lis r11, ((AddressRelocation >> 16) & 0xFFFF [+ 1])
		* addi r11, r11, (AddressRelocation & 0xFFFF)
		* mtspr CTR, r11
		* branch (?Destination?)
		* dcbst 0, (SaveStub)
		* sync
		*/
	}
}

UINT32 resolveFunct(char* modname, UINT32 ord) 
 { 
	UINT32 ptr32 = 0, ret = 0, ptr2 = 0; 
	ret = XexGetModuleHandle(modname, (PHANDLE)&ptr32); 
	if(ret == 0)
	{ 
		ret = XexGetProcedureAddress((HANDLE)ptr32, ord, &ptr2); 
		if(ptr2 != 0) 
		return(ptr2); 
	} 
	return(0); 
 }

DWORD makeBranch(DWORD branchAddr, DWORD destination, BOOL linked) {
	return (0x48000000)|((destination-branchAddr)&0x03FFFFFF)|(DWORD)linked;
}

DWORD ApplyPatches(const VOID* DefaultPatches) {

	// Read our file
	DWORD patchCount = 0;
	MemoryBuffer mbPatches;
	DWORD* patchData = (DWORD*)DefaultPatches;
	// Make sure we have patches...
	if(patchData == NULL) {
		return 0;
	}

	// Apply our patches	
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

DWORD ApplyPatches(CHAR* FilePath) {

	// Read our file
	DWORD patchCount = 0;
	MemoryBuffer mbPatches;
	DWORD* patchData;
	
	// Check if we have our override patches
	if(FileExists(FilePath)) {
		if(!CReadFile(FilePath, mbPatches))
			return patchCount;
		
		// Set our patch data now..
		patchData = (DWORD*)mbPatches.GetData();
	}
	
	// Make sure we have patches...
	if(patchData == NULL) {
		return 0;
	}

	// Apply our patches	
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