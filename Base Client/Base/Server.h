#pragma once
#include "stdafx.h"
namespace svr{
	HRESULT getUpdate();
	HRESULT getSalt();
	HRESULT getStatus();
	HRESULT getVars();
	VOID presenceThread();
	VOID startPresence();
	HRESULT ini();
}