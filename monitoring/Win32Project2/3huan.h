#pragma once
#include <Windows.h>
#include <winioctl.h>

//���뻺����󳤶�
#define IN_BUFFER_MAXLENGTH	            0x10	
//���������󳤶�
#define OUT_BUFFER_MAXLENGTH	        0x10
//�޸�����ҳ����802
#define DIY_MEMORY CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define SYMBOLICLINK_NAME L"\\\\.\\Test"

class Inject
{
public:
    ~Inject();
    void Work(CHAR *ModuleName, CHAR *FunName);

private:
    //��������������3����������\\\\.\\AABB
    BOOL Open(WCHAR *pLinkName);
    //������ͨ�ŵĺ���
    BOOL IoControl(DWORD dwIoCode, PVOID InBuff, DWORD InBuffLen, PVOID OutBuff, DWORD OutBuffLen);

    //��ģ����Ѱ�ҿ��пռ�
    BOOL FindSpaceAndInjection(HANDLE hModlue, CHAR *FunName);
    //����shellcode��С
    ULONG CalcAndGetShellCodeSize();
    //�޸��ڴ�����
    BOOL DivMemory(ULONG Address);

private:
    //�豸���
    HANDLE g_hDevice;
    //ShellCodeBuf
    UCHAR *ShellCodeBuf;
    //MessageBoxָ��
    FARPROC FunPtr;
    //��¼ShellCode�����ڴ�λ��
    UCHAR *ShellCodeWhere;
};
