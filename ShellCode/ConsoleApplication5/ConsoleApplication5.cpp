#include "ShellCode.h"
#include <windows.h>
#include <Psapi.h>

//���API�����ã����ص��õ��߳�ID������ID������ǰ�߳�


void Entry()
{
    ULONG uEsp;
    __asm
    {
        //����ջ�������ڲ�ѯ����
        mov uEsp, ebp;
    }
    //��ʼ��
    ENVIRONMENT InitPtr;
    InitFunPtr(&InitPtr);
    PENVIRONMENT FunPtr = &InitPtr;
    char WindowName[] = { 'M','o','n','i','t','o','r','i','n','g','\0' };
    //�Ҽ��API�Ĵ���
    HWND hMonitoring = FunPtr->pfnFindWindowA(NULL, WindowName);

    //��ȡ�߳�ID
    ULONG ProcessId = FunPtr->pfnGetCurrentProcessId();
    //��Ϣ��� WM_USER + 1 = 0X401
    FunPtr->pfnPostMessageA(hMonitoring, 0x401, uEsp, ProcessId);
    //�����Լ�
    FunPtr->pfnSleep(800);
    ULONG Message = (ULONG)(FunPtr->pfnMessageBoxA) + 5;
    __asm
    {
        mov eax, Message;
        //�ָ�����
        mov esp, ebp;
        pop ebp;
        //�ظ�����
        push ebp;
        mov ebp, esp;
        //����ԭ����
        jmp eax;
    }
}


