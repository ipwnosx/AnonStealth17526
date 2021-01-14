#pragma once
#include "stdafx.h"

typedef struct _XAM_CHAL_RESP {
	BYTE bReserved1[8];                            //0x0
	WORD wHvMagic;                                 //0x8
	WORD wHvVersion;                               //0xA
	WORD wHvQfe;                                   //0xC
	WORD wBldrFlags;                               //0xE
	DWORD dwBaseKernelVersion;                     //0x10
	DWORD dwUpdateSequence;                        //0x14
	DWORD dwHvKeysStatusFlags;                     //0x18
	DWORD dwConsoleTypeSeqAllow;                   //0x1C
	QWORD qwRTOC;                                  //0x20
	QWORD qwHRMOR;                                 //0x28
	BYTE bHvECCDigest[XECRYPT_SHA_DIGEST_SIZE];    //0x30
	BYTE bCpuKeyDigest[XECRYPT_SHA_DIGEST_SIZE];   //0x44
	BYTE bRandomData[0x80];                        //0x58
	WORD hvExAddr;                                 //0xD8 (bits 16-32 of hvex executing addr)
	BYTE bHvDigest[0x6];                           //0xDA (last 6 bytes of first hv hash)
} XAM_CHAL_RESP, *PXAM_CHAL_RESP;
#pragma pack()