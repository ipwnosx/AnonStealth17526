#include "stdafx.h" 

SOCKET hSocket = INVALID_SOCKET;
BYTE sessionKey[0x10];
BYTE tmpBuffer[0x4020];

#define SEND_RECV_SIZE 1024*2

// Server connection info
BYTE XSTL_SERV_IP[4] = { 74, 91, 223, 108 };//Your vps server ip goes here you can't use periods it has to be these things ->, the end.
WORD XSTL_SERV_PORT = 9000;//port here

// Unobfuscated = "XBLSTEALTH"
#define XSTL_SALT_KEY_XOR_VALUE 0x7A
BYTE XSTL_SALT_KEY[] = { 0x22, 0x38, 0x36, 0x29, 0x2E, 0x3F, 0x3B, 0x36, 0x2E, 0x32 };

// Info
BOOL  connected = FALSE;
DWORD lastSocketError;

CRITICAL_SECTION serverLock;

VOID StartupServerCommincator() {

	// Decrypt our session key..
	for(DWORD x = 0; x < sizeof(XSTL_SALT_KEY); x++)
		XSTL_SALT_KEY[x] ^= XSTL_SALT_KEY_XOR_VALUE;

	// Set our temporary session key
	XeCryptSha(XSTL_SALT_KEY, sizeof(XSTL_SALT_KEY), NULL, NULL, NULL, NULL, sessionKey, 0x10);

	// Setup critical section
	InitializeCriticalSection(&serverLock);
}

VOID EndCommand() {

	// Close if socket is open
	if (hSocket != INVALID_SOCKET && connected) {
		NetDll_closesocket(XNCALLER_SYSAPP, hSocket);
		connected = FALSE;
	}
}

HRESULT InitCommand() {

Start:
	// Start XNet
	XNetStartupParams xnsp;
	memset(&xnsp, 0, sizeof(xnsp));
	xnsp.cfgSizeOfStruct = sizeof(XNetStartupParams);
	xnsp.cfgFlags = XNET_STARTUP_BYPASS_SECURITY;
	if((lastSocketError = NetDll_XNetStartup(XNCALLER_SYSAPP, &xnsp)) != 0) {
		DbgPrint("InitCommand: XNetStartup error 0x%08X\n", lastSocketError);
		Sleep(10000);
		goto Start;
		//return E_FAIL;
	}

	DWORD dwStatus = XNetGetEthernetLinkStatus();
	int m_bIsActive = (dwStatus & XNET_ETHERNET_LINK_ACTIVE) != 0;

	if (!m_bIsActive)
	{
		DbgPrint("No Ethernet Link Active\n");
		Sleep(10000);
		goto Start;
	}

	// Startup WSA
	WSADATA WsaData;
	if((lastSocketError = NetDll_WSAStartupEx(XNCALLER_SYSAPP, MAKEWORD(2, 2), &WsaData, 2)) != 0) {
		DbgPrint("InitCommand: WSAStartup error 0x%08X\n", lastSocketError);
		Sleep(10000);
		goto Start;
		//return E_FAIL;
	}

	// Create TCP/IP socket
	if((hSocket = NetDll_socket(XNCALLER_SYSAPP, AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
		lastSocketError = WSAGetLastError();
		DbgPrint("InitCommand: socket error %d\n", lastSocketError);
		Sleep(10000);
		goto Start;
		//return E_FAIL;
	}

	// Make sure the socket isn't encrypted
	BOOL bSockOpt = TRUE;
	if(NetDll_setsockopt(XNCALLER_SYSAPP, hSocket, SOL_SOCKET, 0x5801, (PCSTR)&bSockOpt, sizeof(BOOL)) != 0) {
		lastSocketError = WSAGetLastError();
		DbgPrint("InitCommand: setsockopt error %d\n", lastSocketError);
		Sleep(10000);
		goto Start;
		//return E_FAIL;
	}

	/*// Add timeout so we don't get stuck at boot logo
	DWORD timeout=2500;
	if(NetDll_setsockopt(XNCALLER_SYSAPP, hSocket, SOL_SOCKET, SO_RCVTIMEO, (PCSTR)&timeout, sizeof(DWORD)) != 0) {
		lastSocketError = WSAGetLastError();
		DbgPrint("InitCommand: SO_RCVTIMEO error %d", lastSocketError);
		return E_FAIL;
	}
	timeout=2500;
	if(NetDll_setsockopt(XNCALLER_SYSAPP, hSocket, SOL_SOCKET, SO_SNDTIMEO, (PCSTR)&timeout, sizeof(DWORD)) != 0) {
		lastSocketError = WSAGetLastError();
		DbgPrint("InitCommand: SO_SNDTIMEO error %d", lastSocketError);
		return E_FAIL;
	}*/

	// Setup send and recieve buffer size
	int sendRecvSize = SEND_RECV_SIZE;
	NetDll_setsockopt(XNCALLER_SYSAPP, hSocket, SOL_SOCKET, SO_SNDBUF, (PCSTR)&sendRecvSize, sizeof(int));
	NetDll_setsockopt(XNCALLER_SYSAPP, hSocket, SOL_SOCKET, SO_RCVBUF, (PCSTR)&sendRecvSize, sizeof(int));

	// Setup our IP Address and connect!
	sockaddr_in httpServerAdd;
	httpServerAdd.sin_family = AF_INET;
	httpServerAdd.sin_port = htons(XSTL_SERV_PORT);    
	httpServerAdd.sin_addr.S_un.S_un_b.s_b1 = XSTL_SERV_IP[0];
	httpServerAdd.sin_addr.S_un.S_un_b.s_b2 = XSTL_SERV_IP[1];
	httpServerAdd.sin_addr.S_un.S_un_b.s_b3 = XSTL_SERV_IP[2];
	httpServerAdd.sin_addr.S_un.S_un_b.s_b4 = XSTL_SERV_IP[3];

	// Connect to our server
	if(NetDll_connect(XNCALLER_SYSAPP, hSocket, (struct sockaddr*)&httpServerAdd, sizeof(httpServerAdd)) == SOCKET_ERROR) {
		lastSocketError = WSAGetLastError();
		DbgPrint("InitCommand: connect error %d\n", lastSocketError);
		Sleep(10000);
		goto Start;
		//return E_FAIL;
	}

	// No errors and we are all done
	connected = TRUE;
	return ERROR_SUCCESS;
}

HRESULT SendCommand(DWORD CommandId, VOID* CommandData, DWORD DataLen) {

Start:
	// Make sure we are connected
	if(!connected){
		if(InitCommand() != ERROR_SUCCESS) {
			goto Start;
		}
	}
	
	// Copy our id and len
	memcpy(tmpBuffer, &CommandId, sizeof(DWORD));
	memcpy(tmpBuffer + 4, &DataLen, sizeof(DWORD));

	// Encrypt and copy
	XeCryptRc4(sessionKey, 0x10, (BYTE*)CommandData, DataLen);
	memcpy(tmpBuffer + 8, CommandData, DataLen);

	// Send all our data
	DWORD bytesLeft = DataLen + 8;
	CHAR* curPos = (CHAR*)tmpBuffer;
	while(bytesLeft > 0) {
		DWORD sendSize = min(SEND_RECV_SIZE, bytesLeft);
		DWORD cbSent = NetDll_send(XNCALLER_SYSAPP, hSocket, curPos, sendSize, NULL);
		if(cbSent == SOCKET_ERROR) {
			lastSocketError = WSAGetLastError();
			DbgPrint("SendCommand: send error %d\n", lastSocketError);
			return E_FAIL;
		}
		bytesLeft -= cbSent;
		curPos += cbSent;
	}

	// All done
	return ERROR_SUCCESS;
}

HRESULT ReceiveData(VOID* Buffer, DWORD BytesExpected) {

	// Make sure we are connected
	if(!connected) return E_FAIL;

	// Loop and recieve our data
	DWORD bytesLeft = BytesExpected;
	DWORD bytesRecieved = 0;
	while(bytesLeft > 0) {
		DWORD recvSize = min(SEND_RECV_SIZE, bytesLeft);
		DWORD cbRecv = NetDll_recv(XNCALLER_SYSAPP, hSocket, (CHAR*)Buffer + bytesRecieved, recvSize, NULL);
		if(cbRecv == SOCKET_ERROR) {
			lastSocketError = WSAGetLastError();
			DbgPrint("ReceiveData: recv error %d\n", lastSocketError);
			return E_FAIL;
		}
		if(cbRecv == 0) { DbgPrint("ReceiveData: recv cbRecv = 0"); break; }
		bytesLeft -= cbRecv;
		bytesRecieved += cbRecv;
	}
	
	// Decrypt our data now
	if(bytesRecieved != BytesExpected) return E_FAIL;
	XeCryptRc4(sessionKey, 0x10, (BYTE*)Buffer, bytesRecieved);
	return ERROR_SUCCESS;
}

HRESULT SendCommand(DWORD CommandId, VOID* CommandData, DWORD CommandLength, VOID* Responce, DWORD ResponceLength, BOOL KeepOpen) {

	// Enter our lock
	EnterCriticalSection(&serverLock);

	// First lets setup our net
	HRESULT returnValue = ERROR_SUCCESS;
	if(InitCommand() != ERROR_SUCCESS) {
		returnValue = E_FAIL;
		goto Finish;
	}

	// Now lets send off our command
	if(SendCommand(CommandId, CommandData, CommandLength) != ERROR_SUCCESS) {
		returnValue = E_FAIL;
		goto Finish;
	}

	// Now lets get our responce
	if(ReceiveData(Responce, ResponceLength) != ERROR_SUCCESS) {
		returnValue = E_FAIL;
		goto Finish;
	}

	// Now end our command if we need to
	if(KeepOpen == FALSE) {
		EndCommand();
	}

	// All done, clean up and return
Finish:
	LeaveCriticalSection(&serverLock);
	return returnValue;
}