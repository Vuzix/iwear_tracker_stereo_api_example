
// 412MA0002 Test App.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CMy412MA0002TestAppApp:
// See 412MA0002 Test App.cpp for the implementation of this class
//

class CMy412MA0002TestAppApp : public CWinApp
{
public:
	CMy412MA0002TestAppApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CMy412MA0002TestAppApp theApp;