#include "stdafx.h"
#include "HvPeekPoke.h"
#pragma warning(push)
#pragma warning(disable:4826) // Get rid of the sign-extended warning

#define HvxCall QWORD _declspec(naked)
static HvxCall HvxExpansionInstall(DWORD PhysicalAddress, DWORD CodeSize) {
	__asm {
		li			r0, 0x70
		sc
		blr
	}
}
static HvxCall HvxExpansionCall(DWORD ExpansionId, QWORD Param1 = 0, QWORD Param2 = 0, QWORD Param3 = 0, QWORD Param4 = 0) {
	__asm {
		li			r0, 0x71
		sc
		blr
	}
}
	
HRESULT InitializeHvPeekPoke() {

	// Allocate physcial memory for this expansion
	VOID* pPhysExp = XPhysicalAlloc(0x1000, MAXULONG_PTR, 0, PAGE_READWRITE);
	DWORD physExpAdd = (DWORD)MmGetPhysicalAddress(pPhysExp);

	// Copy over our expansion data
	ZeroMemory(pPhysExp, 0x1000);
	memcpy(pPhysExp, HvPeekPokeExp, sizeof(HvPeekPokeExp));

	// Now we can install our expansion
	HRESULT result = (HRESULT)HvxExpansionInstall(physExpAdd, 0x1000);

	// Free our allocated data
	XPhysicalFree(pPhysExp);

	// Return our install result
	return result;
}
	
BYTE    HvPeekBYTE(QWORD Address) {
	return (BYTE)HvxExpansionCall(HvPeekPokeExpID, PEEK_BYTE, Address);
}
WORD    HvPeekWORD(QWORD Address) {
	return (WORD)HvxExpansionCall(HvPeekPokeExpID, PEEK_WORD, Address);
}
DWORD   HvPeekDWORD(QWORD Address) {
	return (DWORD)HvxExpansionCall(HvPeekPokeExpID, PEEK_DWORD, Address);
}
QWORD   HvPeekQWORD(QWORD Address) {
	return HvxExpansionCall(HvPeekPokeExpID, PEEK_QWORD, Address);
}
	
HRESULT HvPeekBytes(QWORD Address, PVOID Buffer, DWORD Size) {	
	
	// Create a physical buffer to peek into
	VOID* data = XPhysicalAlloc(Size, MAXULONG_PTR, 0, PAGE_READWRITE);
	ZeroMemory(data, Size);
	
	HRESULT result = (HRESULT)HvxExpansionCall(HvPeekPokeExpID, 
		PEEK_BYTES, Address, (QWORD)MmGetPhysicalAddress(data), Size);

	// If its successful copy it back
	if(result == S_OK) memcpy(Buffer, data, Size);

	// Free our physical data and return our result
	XPhysicalFree(data);
	return result;
}

HRESULT HvPokeBYTE(QWORD Address, BYTE Value) {
	return (HRESULT)HvxExpansionCall(HvPeekPokeExpID, POKE_BYTE, Address, Value);
}
HRESULT HvPokeWORD(QWORD Address, WORD Value) {
	return (HRESULT)HvxExpansionCall(HvPeekPokeExpID, POKE_WORD, Address, Value);
}
HRESULT HvPokeDWORD(QWORD Address, DWORD Value) {
	return (HRESULT)HvxExpansionCall(HvPeekPokeExpID, POKE_DWORD, Address, Value);
}
HRESULT HvPokeQWORD(QWORD Address, QWORD Value) {
	return (HRESULT)HvxExpansionCall(HvPeekPokeExpID, POKE_QWORD, Address, Value);
}
HRESULT HvPokeBytes(QWORD Address, const void* Buffer, DWORD Size) {

	// Create a physical buffer to poke from
	VOID* data = XPhysicalAlloc(Size, MAXULONG_PTR, 0, PAGE_READWRITE);
	memcpy(data, Buffer, Size);
	
	HRESULT result = (HRESULT)HvxExpansionCall(HvPeekPokeExpID, 
		POKE_BYTES, Address, (QWORD)MmGetPhysicalAddress(data), Size);

	// Free our physical data and return our result
	XPhysicalFree(data);
	return result;
}
QWORD HvGetFuseLine(BYTE fuseIndex){
	if (fuseIndex > 11) return 0;
	return HvPeekQWORD(0x8000020000020000 + (fuseIndex * 0x200));
}
#pragma warning(pop)