#include "processes.h"
#include "string.h"
#include <stdio.h>




//-------------------------------------------------------------------------------------------
// global variables
lpfEnumProcesses			EnumProcesses;
lpfEnumProcessModules		EnumProcessModules;
lpfGetModuleBaseName		GetModuleBaseName;
lpfEnumDeviceDrivers		EnumDeviceDrivers;
lpfGetDeviceDriverBaseName	GetDeviceDriverBaseName;

HINSTANCE					g_hInstance;
HWND						g_hwndParent;
HINSTANCE					g_hInstLib;





//-------------------------------------------------------------------------------------------
// main DLL entry
BOOL WINAPI		_DllMainCRTStartup( HANDLE	hInst, 
                                    ULONG	ul_reason_for_call,
                                    LPVOID	lpReserved )
{
    g_hInstance		= (struct HINSTANCE__ *)hInst;

    return TRUE;
}





//-------------------------------------------------------------------------------------------
// loads the psapi routines
bool	LoadPSAPIRoutines( void )
{
    if( NULL == (g_hInstLib = LoadLibrary( _T("PSAPI.DLL") )) )
        return false;

    EnumProcesses			= (lpfEnumProcesses)			GetProcAddress( g_hInstLib, "EnumProcesses" );
    EnumProcessModules		= (lpfEnumProcessModules)		GetProcAddress( g_hInstLib, "EnumProcessModules" );
    
    EnumDeviceDrivers		= (lpfEnumDeviceDrivers)		GetProcAddress( g_hInstLib, "EnumDeviceDrivers" );
    
#ifdef _UNICODE
    GetModuleBaseName = (lpfGetModuleBaseName)GetProcAddress( g_hInstLib, "GetModuleBaseNameW" );
    GetDeviceDriverBaseName = (lpfGetDeviceDriverBaseName)GetProcAddress( g_hInstLib, "GetDeviceDriverBaseNameW" );
#else
    GetModuleBaseName = (lpfGetModuleBaseName)GetProcAddress( g_hInstLib, "GetModuleBaseNameA" );
    GetDeviceDriverBaseName = (lpfGetDeviceDriverBaseName)GetProcAddress( g_hInstLib, "GetDeviceDriverBaseNameA" );
#endif

    if( ( NULL == EnumProcesses ) ||
        ( NULL == EnumProcessModules ) ||
        ( NULL == EnumDeviceDrivers ) ||
        ( NULL == GetModuleBaseName ) ||
        ( NULL == GetDeviceDriverBaseName ) )
    {
        FreeLibrary( g_hInstLib );

        return false;
    }

    return true;
}





//-------------------------------------------------------------------------------------------
// free the psapi routines
bool	FreePSAPIRoutines( void )
{
    EnumProcesses		= NULL;
    EnumProcessModules	= NULL;
    GetModuleBaseName	= NULL;
    EnumDeviceDrivers	= NULL;

    if( FALSE == FreeLibrary( g_hInstLib ) )
        return false;

    return true;
}





//-------------------------------------------------------------------------------------------
// find a process by name
// return value:	true	- process was found
//					false	- process not found
bool	FindProc( TCHAR *szProcess )
{
    TCHAR		szProcessName[1024];
    TCHAR		szCurrentProcessName[1024];
    DWORD		dPID[ 1024*3 ];
    DWORD		dPIDSize( 1024 );
    DWORD		dSize( 1024 );
    HANDLE		hProcess;
    HMODULE		phModule[ 1024 ];

  
    //
    // make the name lower case
    //
    memset( szProcessName, 0, 1024 * sizeof( TCHAR ) );
    _stprintf( szProcessName, _T( "%s"), szProcess );
    _tcslwr( szProcessName );

    //
    // load PSAPI routines
    //
    if( false == LoadPSAPIRoutines() )
        return false;

    //
    // enumerate processes names
    //
    if( FALSE == EnumProcesses( dPID, dSize, &dPIDSize ) )
    {
        FreePSAPIRoutines();

        return false;
    }

    //
    // walk trough and compare see if the process is running
    //
    for( int k( dPIDSize / sizeof( DWORD ) ); k >= 0; k-- )
    {
        memset( szCurrentProcessName, 0, 1024 * sizeof( TCHAR ) );

        if( NULL != ( hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_TERMINATE, FALSE, dPID[ k ] ) ) )
        {
            if( TRUE == EnumProcessModules( hProcess, phModule, sizeof(HMODULE)*1024, &dPIDSize ) )
                if( GetModuleBaseName( hProcess, phModule[ 0 ], szCurrentProcessName, 1024 ) > 0 )
                {
                    _tcslwr( szCurrentProcessName );

                    if( NULL != _tcsstr( szCurrentProcessName, szProcessName ) )
                    {
                        FreePSAPIRoutines();
                        CloseHandle( hProcess );

                        return true;
                    }
                }

            CloseHandle( hProcess );
        }
    }
    
    //
    // free PSAPI routines
    //
    FreePSAPIRoutines();

    return false;
}





//-------------------------------------------------------------------------------------------
// kills a process by name
// return value:	true	- process was found
//					false	- process not found
bool	KillProc( TCHAR *szProcess )
{
    TCHAR		szProcessName[1024];
    TCHAR		szCurrentProcessName[1024];
    DWORD		dPID[ 1024 ];
    DWORD		dPIDSize( 1024 );
    DWORD		dSize( 1024 );
    HANDLE		hProcess;
    HMODULE		phModule[ 1024 ];

  
    //
    // make the name lower case
    //
    memset( szProcessName, 0, 1024 * sizeof( TCHAR ) );
    _stprintf( szProcessName, _T("%s"), szProcess );
    _tcslwr( szProcessName );

    //
    // load PSAPI routines
    //
    if( false == LoadPSAPIRoutines() )
        return false;

    //
    // enumerate processes names
    //
    if( FALSE == EnumProcesses( dPID, dSize, &dPIDSize ) )
    {
        FreePSAPIRoutines();

        return false;
    }

    //
    // walk trough and compare see if the process is running
    //
    for( int k( dPIDSize / sizeof( DWORD ) ); k >= 0; k-- )
    {
        memset( szCurrentProcessName, 0, 1024 * sizeof( TCHAR ) );

        if( NULL != ( hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_TERMINATE, FALSE, dPID[ k ] ) ) )
        {
            if( TRUE == EnumProcessModules( hProcess, phModule, sizeof(HMODULE)*1024, &dPIDSize ) )
                if( GetModuleBaseName( hProcess, phModule[ 0 ], szCurrentProcessName, 1024 ) > 0 )
                {
                    _tcslwr( szCurrentProcessName );

                    if( NULL != _tcsstr( szCurrentProcessName, szProcessName ) )
                    {
                        FreePSAPIRoutines();

                        //
                        // kill process
                        //
                        if( false == TerminateProcess( hProcess, 0 ) )
                        {
                            CloseHandle( hProcess );

                            return true;
                        }

                        //
                        // refresh systray
                        //
                        UpdateWindow( FindWindow( NULL, _T("Shell_TrayWnd") ) );

                        //
                        // refresh desktop window
                        //
                        UpdateWindow( GetDesktopWindow() );

                        CloseHandle( hProcess );

                        return true;
                    }
                }

            CloseHandle( hProcess );
        }
    }
    
    //
    // free PSAPI routines
    //
    FreePSAPIRoutines();

    return false;
}





//-------------------------------------------------------------------------------------------
bool	FindDev( TCHAR *szDriverName )
{
    TCHAR		szDeviceName[1024];
    TCHAR		szCurrentDeviceName[1024];
    LPVOID		lpDevices[ 1024 ];
    DWORD		dDevicesSize( 1024 );
    DWORD		dSize( 1024 );
    TCHAR		tszCurrentDeviceName[ 1024 ];
    DWORD		dNameSize( 1024 );

  
    //
    // make the name lower case
    //
    memset( szDeviceName, 0, 1024 * sizeof( TCHAR ) );
    _stprintf( szDeviceName, _T( "%s" ), _tcslwr( szDriverName ) );

    //
    // load PSAPI routines
    //
    if( false == LoadPSAPIRoutines() )
        return false;

    //
    // enumerate devices
    //
    if( FALSE == EnumDeviceDrivers( lpDevices, dSize, &dDevicesSize ) )
    {
        FreePSAPIRoutines();

        return false;
    }

    //
    // walk trough and compare see if the device driver exists
    //
    for( int k( dDevicesSize / sizeof( LPVOID ) ); k >= 0; k-- )
    {
        memset( szCurrentDeviceName, 0, 1024 * sizeof( TCHAR ) );
        memset( tszCurrentDeviceName, 0, 1024*sizeof(TCHAR) );

        if( 0 != GetDeviceDriverBaseName( lpDevices[ k ], tszCurrentDeviceName, dNameSize ) )
        {
            _stprintf( szCurrentDeviceName, _T("%S"), tszCurrentDeviceName );

            if( 0 == _tcscmp( _tcslwr( szCurrentDeviceName ), szDeviceName ) )
            {
                FreePSAPIRoutines();

                return true;
            }
        }
    }
    
    //
    // free PSAPI routines
    //
    FreePSAPIRoutines();

    return false;
}





//-------------------------------------------------------------------------------------------
extern "C" __declspec(dllexport) void	FindProcess( HWND		hwndParent, 
                                                     int		string_size,
                                                     TCHAR		*variables,
                                                     stack_t	**stacktop,
                                                     extra_parameters *extra )
{
    TCHAR		szParameter[1024];


    g_hwndParent	= hwndParent;

    EXDLL_INIT();
    {
        popstring( szParameter );

        if( true == FindProc( szParameter ) )
            _stprintf( szParameter, _T("1") );
        else
            _stprintf( szParameter, _T("0") );

        setuservariable( INST_R0, szParameter );
    }
}





//-------------------------------------------------------------------------------------------
extern "C" __declspec(dllexport) void	KillProcess( HWND		hwndParent, 
                                                     int		string_size,
                                                     TCHAR		*variables,
                                                     stack_t	**stacktop,
                                                     extra_parameters *extra )
{
    TCHAR		szParameter[1024];


    g_hwndParent	= hwndParent;

    EXDLL_INIT();
    {
        popstring( szParameter );

        if( true == KillProc( szParameter ) )
            _stprintf( szParameter, _T("1") );
        else
            _stprintf( szParameter, _T("0") );

        setuservariable( INST_R0, szParameter );
    }
}





//-------------------------------------------------------------------------------------------
extern "C" __declspec(dllexport) void	FindDevice( HWND		hwndParent, 
                                                     int		string_size,
                                                     TCHAR		*variables,
                                                     stack_t	**stacktop,
                                                     extra_parameters *extra )
{
    TCHAR		szParameter[1024];


    g_hwndParent	= hwndParent;

    EXDLL_INIT();
    {
        popstring( szParameter );

        if( true == FindDev( szParameter ) )
            _stprintf( szParameter, _T("1") );
        else
            _stprintf( szParameter, _T("0") );

        setuservariable( INST_R0, szParameter );
    }
}

BOOL WINAPI DllMain( HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved )
{
    return TRUE;
}