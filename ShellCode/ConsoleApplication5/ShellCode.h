#pragma once
#include <windows.h>

//需要的函数在此自己声明
//C库函数源码
int __cdecl mymemcmp(
    const void * buf1,
    const void * buf2,
    size_t count
    );

void * __cdecl mymemset(
    void *dst,
    int val,
    size_t count
    );

void * __cdecl mymemcpy(
    void * dst,
    const void * src,
    size_t count
    );

//LoadLibraryA
typedef
HMODULE
(WINAPI* PFN_LoadLibraryA)(
    _In_ LPCSTR lpLibFileName
    );

//LoadLibraryW
typedef
HMODULE
(WINAPI* PFN_LoadLibraryW)(
    _In_ LPCWSTR lpLibFileName
    );

//VirtualAlloc
typedef
LPVOID
(WINAPI* PFN_VirtualAlloc)(
    _In_opt_ LPVOID lpAddress,
    _In_ SIZE_T dwSize,
    _In_ DWORD flAllocationType,
    _In_ DWORD flProtect
    ); 

//VirtualAllocEx
typedef
LPVOID
(WINAPI* PFN_VirtualAllocEx)(
    _In_ HANDLE hProcess,
    _In_opt_ LPVOID lpAddress,
    _In_ SIZE_T dwSize,
    _In_ DWORD flAllocationType,
    _In_ DWORD flProtect
    );

//VirtualProtect
typedef
BOOL
(WINAPI* PFN_VirtualProtect)(
    _In_ LPVOID lpAddress,
    _In_ SIZE_T dwSize,
    _In_ DWORD flNewProtect,
    _Out_ PDWORD lpflOldProtect
    );

typedef
BOOL
(WINAPI* PFN_VirtualProtect)(
    _In_ LPVOID lpAddress,
    _In_ SIZE_T dwSize,
    _In_ DWORD flNewProtect,
    _Out_ PDWORD lpflOldProtect
    );

//FindWindowA
typedef
HWND
(WINAPI *PFN_FindWindowA)(
    _In_opt_ LPCSTR lpClassName,
    _In_opt_ LPCSTR lpWindowName);

//GetCurrentProcessId
typedef
DWORD
(WINAPI *PFN_GetCurrentProcessId)(
    VOID
    );

//GetCurrentThreadId
typedef
DWORD
(WINAPI *PFN_GetCurrentThreadId)(
    VOID
    );

//GetCurrentThread
typedef
HANDLE
(WINAPI *PFN_GetCurrentThread)(
    VOID
    );

//PostMessageA
typedef
BOOL
(WINAPI *PFN_PostMessageA)(
    _In_opt_ HWND hWnd,
    _In_ UINT Msg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam);

//SuspendThread
typedef
DWORD
(WINAPI *PFN_SuspendThread)(
    _In_ HANDLE hThread
    );

//
typedef
HANDLE
(WINAPI *PFN_OpenThread)(
    _In_ DWORD dwDesiredAccess,
    _In_ BOOL bInheritHandle,
    _In_ DWORD dwThreadId
    );

//MessageBoxA
typedef
int
(WINAPI *PFN_MessageBoxA)(
    __in_opt HWND hWnd,
    __in_opt LPCSTR lpText,
    __in_opt LPCSTR lpCaption,
    __in UINT uType
    );

//GetLastError
typedef
DWORD
(WINAPI *PFN_GetLastError)(
    VOID
    );

//Sleep
typedef
VOID
(WINAPI *PFN_Sleep)(
    __in DWORD dwMilliseconds
    );

//ExitProcess
typedef
BOOL
(WINAPI* PFN_ExitProcess)(
    UINT uExitCode
    );

//所有的函数指针都集中在这个结构体中统一调用
typedef struct tagEnvironment
{
    PFN_LoadLibraryA pfnLoadLibraryA = NULL;
    PFN_LoadLibraryW pfnLoadLibraryW = NULL;
    PFN_VirtualAlloc pfnVirtualAlloc = NULL;
    PFN_VirtualAllocEx pfnVirtualAllocEx = NULL;
    PFN_VirtualProtect pfnVirtualProtect = NULL;
    PFN_ExitProcess pfnExitProcess = NULL;
    PFN_FindWindowA pfnFindWindowA = NULL;
    PFN_GetCurrentProcessId pfnGetCurrentProcessId = NULL;
    PFN_GetCurrentThreadId pfnGetCurrentThreadId = NULL;
    PFN_GetCurrentThread pfnGetCurrentThread = NULL;
    PFN_PostMessageA pfnPostMessageA = NULL;
    PFN_SuspendThread pfnSuspendThread = NULL;
    PFN_MessageBoxA pfnMessageBoxA = NULL;
    PFN_OpenThread pfnOpenThread = NULL;
    PFN_GetLastError pfnGetLastError = NULL;
    PFN_Sleep pfnSleep = NULL;
}ENVIRONMENT,*PENVIRONMENT;

//通过名字获得模块句柄
HMODULE MyGetModuleHandle(LPCTSTR modulename);
//拿模块导出的函数地址
FARPROC MyGetProAddress(HMODULE hModule, LPCSTR lpProcName);
//LoadPe
BOOL LoadPe(PENVIRONMENT pEnv, void* PeFile);

//初始化各种函数指针
void InitFunPtr(PENVIRONMENT FunPtr);