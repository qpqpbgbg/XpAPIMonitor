#include "ShellCode.h"
#include <windows.h>
#include <Psapi.h>

//监控API被调用，传回调用的线程ID，进程ID，挂起当前线程


void Entry()
{
    ULONG uEsp;
    __asm
    {
        //保存栈顶，用于查询参数
        mov uEsp, ebp;
    }
    //初始化
    ENVIRONMENT InitPtr;
    InitFunPtr(&InitPtr);
    PENVIRONMENT FunPtr = &InitPtr;
    char WindowName[] = { 'M','o','n','i','t','o','r','i','n','g','\0' };
    //找监控API的窗口
    HWND hMonitoring = FunPtr->pfnFindWindowA(NULL, WindowName);

    //获取线程ID
    ULONG ProcessId = FunPtr->pfnGetCurrentProcessId();
    //信息标号 WM_USER + 1 = 0X401
    FunPtr->pfnPostMessageA(hMonitoring, 0x401, uEsp, ProcessId);
    //挂起自己
    FunPtr->pfnSleep(800);
    ULONG Message = (ULONG)(FunPtr->pfnMessageBoxA) + 5;
    __asm
    {
        mov eax, Message;
        //恢复环境
        mov esp, ebp;
        pop ebp;
        //回复环境
        push ebp;
        mov ebp, esp;
        //跳回原流程
        jmp eax;
    }
}


