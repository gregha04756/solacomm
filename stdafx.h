// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#pragma once



// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

//#pragma comment (lib,"ws2_32.lib")
#pragma comment (lib,"ws2_32.lib")
#pragma comment (lib,"shell32.lib")
#pragma comment (lib,"Report_Error_DLL.lib")
#pragma comment (lib,"Sola_Auto_ID_DLL.lib")
#pragma comment (lib,"Sola_Testing_DLL.lib")

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <stdint.h>
#include <typeinfo.h>
#include <commctrl.h>
#include <commdlg.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <prsht.h>
#include <cderr.h>
#include <queue>
#include <list>
#include <new>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <locale>
#include <codecvt>

#ifdef _DEBUG
   #ifndef DBG_NEW
      #define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
      #define new DBG_NEW
   #endif
#endif  // _DEBUG

// C RunTime Header Files
#include <stdlib.h>
#include <crtdbg.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <process.h>
#include <strsafe.h>
#include <assert.h>
#include <shellapi.h>

// TODO: reference additional headers your program requires here
#include "Report_Error_DLL.h"
#include "Sola_Auto_ID_DLL.h"
#include "Sola_Testing_DLL.h"
