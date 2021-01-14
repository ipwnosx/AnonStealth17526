#include "stdafx.h"
#include "KeyVault.h"
#include "Utilities.h"
#include "HvPeekPoke.h"

#define hvKvPtrDev      0x00000002000162e0
#define hvKvPtrRetail   0x0000000200016240

BYTE kvDigest[XECRYPT_SHA_DIGEST_SIZE];
extern KEY_VAULT keyVault;
extern BOOL IsDevkit;

// Chal info
BOOL  fcrt = FALSE;
BOOL  crl = FALSE;
BOOL  type1KV = FALSE;

BYTE cpuKey[0x10];


BOOL VerifyKeyVault() {
	XECRYPT_HMACSHA_STATE hmacSha; 
	XeCryptHmacShaInit(&hmacSha, cpuKey, 0x10); 
	XeCryptHmacShaUpdate(&hmacSha, (BYTE*)&keyVault.OddFeatures, 0xD4); 
	XeCryptHmacShaUpdate(&hmacSha, (BYTE*)&keyVault.DvdKey, 0x1CF8); 
	XeCryptHmacShaUpdate(&hmacSha, (BYTE*)&keyVault.CardeaCertificate, 0x2108); 
	XeCryptHmacShaFinal(&hmacSha, kvDigest, XECRYPT_SHA_DIGEST_SIZE);

	type1KV = TRUE;
	for(DWORD x = 0; x < 0x100; x++) {
		if(keyVault.KeyVaultSignature[x] != NULL) {
			type1KV = FALSE;
			return TRUE;
		}
	}

	return XeKeysPkcs1Verify(kvDigest, keyVault.KeyVaultSignature, (XECRYPT_RSA*)MasterKey);
}

HRESULT SetKeyVault(BYTE* KeyVault) {
	memcpy(&keyVault, KeyVault, 0x4000);
	
	SetMemory((PVOID)0x8E03A000, &keyVault.ConsoleCertificate, 0x1A8);
	if(IsDevkit) {
		SetMemory((BYTE*)((*(DWORD*)0x81D59F68) + 0x313C), &keyVault.ConsoleCertificate, 0x1A8); // CXNetLogonTask * g_pXNetLogonTask handle // v16203
	}
	
    SetMemory((PVOID)0x8E038020, &keyVault.ConsoleCertificate.ConsoleId.abData, 5);

	BYTE newHash[XECRYPT_SHA_DIGEST_SIZE];
	XeCryptSha((BYTE*)0x8E038014, 0x3EC, NULL, NULL, NULL, NULL, newHash, XECRYPT_SHA_DIGEST_SIZE);
    SetMemory((PVOID)0x8E038000, newHash, XECRYPT_SHA_DIGEST_SIZE);

	QWORD kvAddress = (IsDevkit) ? HvPeekQWORD(hvKvPtrDev) : HvPeekQWORD(hvKvPtrRetail);

	HvPeekBytes(kvAddress + 0xD0, &keyVault.ConsoleObfuscationKey, 0x40);
	memcpy(keyVault.RoamableObfuscationKey, !IsDevkit ? RetailKey19 : DeveloperKey19, 0x10);
	memcpy(keyVault.RoamableObfuscationKey, RetailKey19, 0x10);
	HvPokeBytes(kvAddress, &keyVault, 0x4000);

	DbgPrint("SetKeyVault - KeyVault is set!");

	// All done
	return ERROR_SUCCESS;
}

HRESULT ProcessKeyVault() {
	
	if (VerifyKeyVault() != TRUE) {
		DbgPrint("SetKeyVault - VerifyKeyVault failed, invalid CPU key!");
	}

	fcrt = (keyVault.OddFeatures & ODD_POLICY_FLAG_CHECK_FIRMWARE) != 0 ? TRUE : FALSE;
	return ERROR_SUCCESS;
}

HRESULT SetKeyVault(CHAR* FilePath) {
	MemoryBuffer mbSpiral;
	if(!CReadFile(FilePath, mbSpiral)) {
		DbgPrint("SetKeyVault - CReadFile failed");
		return E_FAIL;
	}

	return SetKeyVault(mbSpiral.GetData());
}

HRESULT LoadKeyVault(CHAR* FilePath) {
	if(IsDevkit) {
		if(SetKeyVault(FilePath) != ERROR_SUCCESS) {
			DbgPrint("LoadKeyVault - SetKeyVault failed");
			return E_FAIL;
		}
	} else {
		if(SetKeyVault("HDD:\\kv.bin") != ERROR_SUCCESS) {
			DbgPrint("LoadKeyVault - SetKeyVault failed");
		}
	}

	return ProcessKeyVault();
}