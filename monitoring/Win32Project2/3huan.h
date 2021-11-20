#pragma once
#include <Windows.h>
#include <winioctl.h>

//输入缓存最大长度
#define IN_BUFFER_MAXLENGTH	            0x10	
//输出缓存最大长度
#define OUT_BUFFER_MAXLENGTH	        0x10
//修改物理页号码802
#define DIY_MEMORY CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define SYMBOLICLINK_NAME L"\\\\.\\Test"

class Inject
{
public:
    ~Inject();
    void Work(CHAR *ModuleName, CHAR *FunName);

private:
    //打开驱动服务句柄，3环链接名：\\\\.\\AABB
    BOOL Open(WCHAR *pLinkName);
    //与驱动通信的函数
    BOOL IoControl(DWORD dwIoCode, PVOID InBuff, DWORD InBuffLen, PVOID OutBuff, DWORD OutBuffLen);

    //从模块中寻找空闲空间
    BOOL FindSpaceAndInjection(HANDLE hModlue, CHAR *FunName);
    //计算shellcode大小
    ULONG CalcAndGetShellCodeSize();
    //修改内存属性
    BOOL DivMemory(ULONG Address);

private:
    //设备句柄
    HANDLE g_hDevice;
    //ShellCodeBuf
    UCHAR *ShellCodeBuf;
    //MessageBox指针
    FARPROC FunPtr;
    //记录ShellCode所在内存位置
    UCHAR *ShellCodeWhere;
};
