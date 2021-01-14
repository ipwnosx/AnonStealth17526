#pragma once
#include "stdafx.h"
#include "titleHooks.h"

BOOL InitializeSystemHooks();
BOOL InitializeSystemXexHooks();
NTSTATUS XexLoadExecutableHook(PCHAR szXexName, PHANDLE pHandle, DWORD dwModuleTypeFlags, DWORD dwMinimumVersion);
NTSTATUS XexLoadImageHook(LPCSTR szXexName, DWORD dwModuleTypeFlags, DWORD dwMinimumVersion, PHANDLE pHandle);
QWORD SpoofXamChallenge(BYTE* pBuffer, DWORD dwFileSize, BYTE* Salt, QWORD Input2, QWORD Input3, QWORD Input4);