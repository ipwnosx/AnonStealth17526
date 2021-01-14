#include "stdafx.h"

BYTE seshKey[16];
KEY_VAULT keyVault;
byte KeyVaultBytes[0x4000];
BYTE cpuKeyBytes[0x10];

byte ExecutableHash[16];

wchar_t welcomeMsg[100];
wchar_t challengeNotify[XSTL_BUFFER_CHALLENGENOTIFYLEN];
SERVER_GET_CUSTOM_RESPONCE cData;

BOOL RunningFromUSB = FALSE;
BOOL IsDevkit = FALSE;

extern BOOL dashLoaded; 
short serverErrors = -1;


void prepareWelcomeMsg(){
	char welcomeMsgBuffer[100];
	if(cData.days>=500) sprintf(welcomeMsgBuffer, "Kernel - Hello %s, You have lifetime!", cData.name);
	else sprintf(welcomeMsgBuffer, "Kernel - Hello %s, You have %i Remaining! ", cData.name, cData.days);
	mbstowcs(welcomeMsg, welcomeMsgBuffer, strlen(welcomeMsgBuffer)+1);
	mbstowcs(challengeNotify, cData.notify, strlen(cData.notify)+1);
}

HRESULT svr::getUpdate() {
	DWORD moduleSize = 0;
	if(ReceiveData(&moduleSize, sizeof(DWORD)) != ERROR_SUCCESS) {
		DbgPrint("ServerGetUpdate - ReceiveData failed");
		return E_FAIL;
	}

	DbgPrint("ServerGetUpdate - moduleSize = 0x%08X", moduleSize);
	BYTE* moduleBuffer = (BYTE*)XPhysicalAlloc(moduleSize, MAXULONG_PTR, NULL, PAGE_READWRITE);
	if(moduleBuffer == NULL) return E_FAIL;

	if(ReceiveData(moduleBuffer, moduleSize) == ERROR_SUCCESS) {
		if(CWriteFile(RunningFromUSB ? PATH_XBLS_USB : PATH_XBLS_HDD, moduleBuffer, moduleSize) != TRUE) {
			DbgPrint("ServerGetUpdate - CWriteFile failed");
			XPhysicalFree(moduleBuffer);
			return E_FAIL;
		}
	}

	EndCommand();
	XPhysicalFree(moduleBuffer);

	DbgPrint("HandleUpdate - Update complete, rebooting");
	HalReturnToFirmware(HalFatalErrorRebootRoutine);

	return ERROR_SUCCESS;
}

HRESULT svr::getSalt() {
	DbgPrint("ServerGetSalt - 1");
	SERVER_GET_SALT_REQUEST* request = (SERVER_GET_SALT_REQUEST*)XPhysicalAlloc(sizeof(SERVER_GET_SALT_REQUEST), MAXULONG_PTR, NULL, PAGE_READWRITE);
	SERVER_GET_SALT_RESPONCE responce;

	request->Version = XSTL_SERVER_VER;
	request->ConsoleType = IsDevkit;
	memcpy(request->CpuKey, cpuKeyBytes, 16);
	memcpy(KeyVaultBytes, &keyVault, 0x4000);
	memcpy(request->KeyVault, KeyVaultBytes, 0x4000);

	if(SendCommand(XSTL_SERVER_COMMAND_ID_GET_SALT, request, sizeof(SERVER_GET_SALT_REQUEST), &responce, sizeof(SERVER_GET_SALT_RESPONCE), TRUE) != ERROR_SUCCESS) {
		DbgPrint("ServerGetSalt - SendCommand failed");
		return E_FAIL;
	}

	DbgPrint("ServerGetSalt - 2");
	XPhysicalFree(request);

	HRESULT retVal = E_FAIL;
	switch(responce.Status) {
		case XSTL_STATUS_SUCCESS:
			retVal = ReceiveData(seshKey, 16);
			DbgPrint("ServerGetSalt - Success!");
			EndCommand();
			return retVal;
		case XSTL_STATUS_EXPIRED:
			EndCommand();
		    return E_FAIL;
		case XSTL_STATUS_ERROR:
			EndCommand();
			return E_FAIL;
		default: 
			EndCommand();
			DbgPrint("ServerGetSalt - Bad Status: 0x%08X", responce.Status);
			return E_FAIL;
	}
	DbgPrint("ServerGetSalt - 3");
	return E_FAIL;
}

HRESULT svr::getStatus() {
	SERVER_GET_STATUS_REQUEST statusRequest;
	SERVER_GET_STATUS_RESPONCE statusResponce;

	MemoryBuffer mbXBLS;
	if(CReadFile(RunningFromUSB ? PATH_XBLS_USB : PATH_XBLS_HDD, mbXBLS) != TRUE) {
		DbgPrint("ServerGetStatus - CReadFile failed");
		return E_FAIL;
	}

	XeCryptHmacSha(seshKey, 16, mbXBLS.GetData(), mbXBLS.GetDataLength(), NULL, 0, NULL, 0, statusRequest.ExecutableHash, 16);
	memcpy(ExecutableHash, statusRequest.ExecutableHash, 0x10);
	memcpy(statusRequest.CpuKey, cpuKeyBytes, 16);

	if(SendCommand(XSTL_SERVER_COMMAND_ID_GET_STATUS, &statusRequest, sizeof(SERVER_GET_STATUS_REQUEST), &statusResponce, sizeof(SERVER_GET_STATUS_RESPONCE), true) != ERROR_SUCCESS) {
		DbgPrint("ServerGetStatus - SendCommand failed");
		return E_FAIL;
	}

	switch(statusResponce.Status) {
		case XSTL_STATUS_SUCCESS:
			EndCommand();
			return ERROR_SUCCESS;
		case XSTL_STATUS_EXPIRED:
			EndCommand();
		    return E_FAIL;
		case XSTL_STATUS_UPDATE:
			return getUpdate();
		case XSTL_STATUS_ERROR:
			EndCommand();
			return E_FAIL;
		default: 
			EndCommand();
			DbgPrint("ServerGetStatus - Bad Status: 0x%08X", statusResponce.Status);
			return E_FAIL;
	}

	return E_FAIL;
}	

HRESULT svr::getVars(){
	SERVER_GET_CUSTOM_REQUEST req;
	memcpy(req.SessionKey, seshKey, 16);
	if(SendCommand(XSTL_SERVER_COMMAND_ID_GET_CUSTOM, &req, sizeof(SERVER_GET_CUSTOM_REQUEST), &cData, sizeof(SERVER_GET_CUSTOM_RESPONCE)) != ERROR_SUCCESS){
		DbgPrint("ServerGetCustom - SendCommand failed");
		return E_FAIL;
	}
	ApplyPatches(cData.xamPatchData);
	prepareWelcomeMsg();
	return ERROR_SUCCESS;
}

VOID svr::presenceThread() {
	SERVER_UPDATE_PRESENCE_REQUEST  req;
	SERVER_UPDATE_PRESENCE_RESPONCE resp;
		LPCWSTR xmsgbuttons[1] = {L"Ok"};
    while(TRUE) {
		while(!dashLoaded) Sleep(1);
		Sleep(500);

		memcpy(req.SessionKey, seshKey, 0x10);
		memcpy(req.ExecutableHash, ExecutableHash, 0x10);
		req.TitleId = XamGetCurrentTitleId();

		XUSER_SIGNIN_INFO userInfo; ZeroMemory(&userInfo, sizeof(XUSER_SIGNIN_INFO));
		if(XUserGetSigninInfo(0, XUSER_GET_SIGNIN_INFO_ONLINE_XUID_ONLY, &userInfo) == ERROR_SUCCESS) {
			memcpy(req.Gamertag, userInfo.szUserName, 16);
		}
		
        if(SendCommand(XSTL_SERVER_COMMAND_ID_UPDATE_PRESENCE, &req, sizeof(SERVER_UPDATE_PRESENCE_REQUEST), &resp, sizeof(SERVER_UPDATE_PRESENCE_RESPONCE), true) != ERROR_SUCCESS) {
			switch(serverErrors++){
				case 0:
					XNotifyUI(L"Kernel - Failed to connect! Attempt 1 of 4...");
					DbgPrint("Connection to NuclearLive Failed! Attempt 1 of 4!");
				break;
				case 1:
					XNotifyUI(L"Kernel - Failed to connect! Attempt 2 of 4...");
					DbgPrint("Connection to NuclearLive Failed! Attempt 2 of 4!");
				break;
				case 2:
					XNotifyUI(L"Kernel - Failed to connect! Attempt 3 of 4...");
					DbgPrint("Connection to NuclearLive Failed! Attempt 3 of 4!");
				break;
				case 3:
					XNotifyUI(L"Kernel - Failed to connect! Attempt 4 of 4...");
					DbgPrint("Final attempt to connect to NuclearLive server failed!");
					HalReturnToFirmware(HalFatalErrorRebootRoutine);
				break;
			}
		}else{
			serverErrors = -0;
			DbgPrint("Presence resp 0x%08X", resp.Status);
			switch(resp.Status) {
				case XSTL_STATUS_SUCCESS:
					EndCommand();
					break;
				case XSTL_STATUS_XNOTIFYMSG:
					DbgPrint("Attempting to receive XNotify...");
					wchar_t XNotify[100];
					char XNotifyBuffer[100];
					if(ReceiveData(XNotifyBuffer, sizeof(XNotifyBuffer)) != ERROR_SUCCESS) {
						DbgPrint("ServerGetXNotify - ReceiveData failed");
						break;
					}
					mbstowcs(XNotify, XNotifyBuffer, strlen(XNotifyBuffer)+1);
					DbgPrint("ServerGetXNotify - Received XNotify: %s", XNotify);
					XNotifyUI(XNotify);
					EndCommand();
					DbgPrint("Kernel Received XNotify Presense");
					break;
				case XSTL_STATUS_MESSAGEBOX:
					DbgPrint("Attempting to receive XShowMessageBoxUI...");
					wchar_t XShowMessageBoxUIMsg[100];
					char XShowMessageBoxUIBuffer[100];
					if(ReceiveData(XShowMessageBoxUIBuffer, sizeof(XShowMessageBoxUIBuffer)) != ERROR_SUCCESS) {
						DbgPrint("ServerGetXShowMessageBoxUI - ReceiveData failed");
						break;
					}
					mbstowcs(XShowMessageBoxUIMsg, XShowMessageBoxUIBuffer, strlen(XShowMessageBoxUIBuffer)+1);
					DbgPrint("ServerGetXShowMessageBoxUI - Received XShowMessageBoxUI: %s", XShowMessageBoxUI);
					
					MESSAGEBOX_RESULT result;
					XOVERLAPPED overlapped;
					XShowMessageBoxUI(0, L"Message from Kernel", XShowMessageBoxUIMsg, 1, xmsgbuttons, 0, XMB_NOICON, &result, &overlapped);
					EndCommand();
					DbgPrint("Kernel Received XShowMessageBoxUI Presense");
					break;
				case XSTL_STATUS_UPDATE:
					XNotifyUI(L"Kernel - Update available, rebooting...");
					Sleep(5000);
					getUpdate();
					return;
				case XSTL_STATUS_EXPIRED:
					EndCommand();
					XNotifyUI(L"Kernel - Out Of Mermory!");
					Sleep(5000);
					HalReturnToFirmware(HalResetSMCRoutine);
					break;
				case XSTL_STATUS_ERROR:
					EndCommand();
					XNotifyUI(L"Kernel - A Very Helpful Error!");
					break;
				default:
					EndCommand();
					DbgPrint("Presence anti-tamper error (unknown resp) 0x%08X", resp.Status);
					XNotifyUI(L"Kernel - Security issue, shutting down...");
					Sleep(4000);
					HalReturnToFirmware(HalResetSMCRoutine);
			}
		}
        Sleep(29500);
    }
}

VOID svr::startPresence(){
	HANDLE hThread; 
	DWORD threadId;
	ExCreateThread(&hThread, 0, &threadId, (VOID*)XapiThreadStartup, (LPTHREAD_START_ROUTINE)presenceThread, NULL, 0x2 | CREATE_SUSPENDED);
	XSetThreadProcessor(hThread, 4);
	SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL);
	ResumeThread(hThread);
}

HRESULT svr::ini(){
	if (svr::getSalt() != ERROR_SUCCESS) {
		DbgPrint("ServerGetSalt Failed\n\r");
		return E_FAIL;
	}

	if (svr::getStatus() != ERROR_SUCCESS) {
		DbgPrint("ServerGetStatus Failed\n\r");
		return E_FAIL;
	}

	if(svr::getVars() != ERROR_SUCCESS){
		DbgPrint("ServerGetCustom Failed\n\r");
		return E_FAIL;
	}
	return ERROR_SUCCESS; 
}