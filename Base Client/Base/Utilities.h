#pragma once
#include "stdafx.h"

typedef HRESULT (*pDmSetMemory)(LPVOID lpbAddr, DWORD cb, LPCVOID lpbBuf, LPDWORD pcbRet);

//typedef struct _XNOTIFY_QUEUE_UI_INFO {
//	XNOTIFYQUEUEUI_TYPE exnq;
//	DWORD dwUserIndex;
//	ULONGLONG qwAreas;
//} XNOTIFY_QUEUE_UI_INFO, *PXNOTIFY_QUEUE_UI_INFO;

class MemoryBuffer
{
public:

	MemoryBuffer( DWORD dwSize = 512 )
	{
		m_pBuffer = NULL;
		m_dwDataLength = 0;
		m_dwBufferSize = 0;

		if( ( dwSize < UINT_MAX ) && ( dwSize != 0 ) )
		{
			m_pBuffer = ( BYTE* )malloc( dwSize + 1 );    // one more char, in case when using string funcions
			if( m_pBuffer )
			{
				m_dwBufferSize = dwSize;
				m_pBuffer[0] = 0;
			}
		}
	};

	~MemoryBuffer()
	{
		if( m_pBuffer )
			free( m_pBuffer );

		m_pBuffer = NULL;
		m_dwDataLength = 0;
		m_dwBufferSize = 0;
	};

    // Add chunk of memory to buffer
    BOOL    Add( const void* p, DWORD dwSize )
    {
        if( CheckSize( dwSize ) )
        {
            memcpy( m_pBuffer + m_dwDataLength, p, dwSize );
            m_dwDataLength += dwSize;
            *( m_pBuffer + m_dwDataLength ) = 0;    // fill end zero
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    };

    // Get the data in buffer
    BYTE* GetData() const
    {
        return m_pBuffer;
    };

    // Get the length of data in buffer
    DWORD   GetDataLength() const
    {
        return m_dwDataLength;
    };

    // Rewind the data pointer to the begining
    void    Rewind()
    {
        m_dwDataLength = 0; m_pBuffer[ 0 ] = 0;
    };

    // Automatically adjust increase buffer size if necessary
    BOOL    CheckSize( DWORD dwSize )
    {
        if( m_dwBufferSize >= ( m_dwDataLength + dwSize ) )
        {
            return TRUE;    // Enough space
        }
        else
        {
            // Try to double it
            DWORD dwNewSize = max( m_dwDataLength + dwSize, m_dwBufferSize * 2 );
            BYTE* pNewBuffer = ( UCHAR* )realloc( m_pBuffer, dwNewSize + 1 );        // one more char
            if( pNewBuffer )
            {
                m_pBuffer = pNewBuffer;
                m_dwBufferSize = dwNewSize;
                return TRUE;
            }
            else
            {
                // Failed
                return FALSE;
            }
        }
    }

	private:

	BYTE* m_pBuffer;

    DWORD m_dwDataLength;

    DWORD m_dwBufferSize;
};

#ifdef _DEBUG
VOID DbgPrint(const char * fmt, ...);
#else
#define DbgPrint
#endif

VOID returnToDash(WCHAR* msg);
VOID XNotifyUI(PWCHAR pwszStringParam);

HRESULT SetMemory(VOID* Destination, VOID* Source, DWORD Length);

HRESULT CreateSymbolicLink(CHAR* szDrive, CHAR* szDeviceName, BOOL System);
HRESULT DeleteSymbolicLink(CHAR* szDrive, BOOL System);

BOOL XeKeysPkcs1Verify(const BYTE* pbHash, const BYTE* pbSig, XECRYPT_RSA* pRsa);

VOID PatchInJump(DWORD* Address, DWORD Destination, BOOL Linked);
VOID PatchInBranch(DWORD* Address, DWORD Destination, BOOL Linked);
FARPROC ResolveFunction(CHAR* ModuleName, DWORD Ordinal);
DWORD PatchModuleImport(CHAR* Module, CHAR* ImportedModuleName, DWORD Ordinal, DWORD PatchAddress);
DWORD PatchModuleImport(PLDR_DATA_TABLE_ENTRY Module, CHAR* ImportedModuleName, DWORD Ordinal, DWORD PatchAddress);

BOOL CReadFile(const CHAR * FileName, MemoryBuffer &pBuffer);
BOOL CWriteFile(const CHAR* FilePath, const VOID* Data, DWORD Size);
BOOL FileExists(LPCSTR lpFileName);
PWCHAR charToWChar(__in LPCSTR c_buffer);
int toWCHAR(char* input, WCHAR* output);

BOOL IsTrayOpen();
VOID returnToDashThread();
VOID returnToDash(WCHAR* msg);

DWORD RelinkGPLR(DWORD SFSOffset, PDWORD SaveStubAddress, PDWORD OriginalAddress);
VOID HookFunctionStart(PDWORD Address, PDWORD SaveStub, DWORD Destination);
UINT32 resolveFunct(char* modname, UINT32 ord) ;
DWORD makeBranch(DWORD branchAddr, DWORD destination, BOOL linked);
DWORD ApplyPatches(const VOID* DefaultPatches);
DWORD ApplyPatches(CHAR* FilePath);
//VOID XNotifyUI(DWORD dwType, DWORD dwUserIndex, DWORD dwPriority, LPCWSTR pwszStringParam, ULONGLONG qwParam);