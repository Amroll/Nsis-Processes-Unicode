#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "pluginapi.h"


//-------------------------------------------------------------------------------------------
// PSAPI function pointers
typedef BOOL	(WINAPI *lpfEnumProcesses)			( DWORD *, DWORD, DWORD * );
typedef BOOL	(WINAPI *lpfEnumProcessModules)		( HANDLE, HMODULE *, DWORD, LPDWORD );
typedef DWORD	(WINAPI *lpfGetModuleBaseName)		( HANDLE, HMODULE, LPTSTR, DWORD );
typedef BOOL	(WINAPI *lpfEnumDeviceDrivers)		( LPVOID *, DWORD, LPDWORD );
typedef BOOL	(WINAPI *lpfGetDeviceDriverBaseName)( LPVOID, LPTSTR, DWORD );






//-------------------------------------------------------------------------------------------
// Internal use routines
bool	LoadPSAPIRoutines( void );
bool	FreePSAPIRoutines( void );

bool	FindProc( TCHAR *szProcess );
bool	KillProc( TCHAR *szProcess );

bool	FindDev( TCHAR *szDriverName );





//-------------------------------------------------------------------------------------------
// Exported routines
extern "C" __declspec(dllexport) void	FindProcess( HWND		hwndParent, 
													 int		string_size,
													 TCHAR		*variables,
													 stack_t	**stacktop,
													 extra_parameters *extra );

extern "C" __declspec(dllexport) void	KillProcess( HWND		hwndParent, 
													 int		string_size,
													 TCHAR		*variables,
													 stack_t	**stacktop,
													 extra_parameters *extra );

extern "C" __declspec(dllexport) void	FindDevice(  HWND		hwndParent, 
													 int		string_size,
													 TCHAR		*variables,
													 stack_t	**stacktop,
													 extra_parameters *extra );
