#pragma once
#include "ShellCode.h"

//需要的函数和模块在这里填写
void InitFunPtr(PENVIRONMENT FunPtr)
{
    //dll名
    char szKernel32[] = { 'K', 'e', 'r', 'n', 'e', 'l', '3', '2','.', 'd', 'l', 'l', '\0' };
    char szUser32[] = { 'U','s','e','r','3','2','.','d','l','l','\0' };

    //函数名
    char szLoadLibraryA[] = { 'L', 'o', 'a', 'd', 'L', 'i', 'b', 'r', 'a', 'r', 'y', 'A', '\0' };
    char szLoadLibraryW[] = { 'L', 'o', 'a', 'd', 'L', 'i', 'b', 'r', 'a', 'r', 'y', 'W', '\0' };
    char szFindWindowA[] = { 'F','i','n','d','W','i','n','d','o','w','A','\0' };
    char szGetCurrentProcessId[] = { 'G','e','t','C','u','r','r','e','n','t','P','r','o','c','e','s','s','I','d','\0' };
    char szGetCurrentThreadId[] = { 'G','e','t','C','u','r','r','e','n','t','T','h','r','e','a','d','I','d','\0' };
    char szPostMessage[] = { 'P','o','s','t','M','e','s','s','a','g','e','A','\0' };
    char szResumeThread[] = { 'R','e','s','u','m','e','T','h','r','e','a','d','\0' };
    char szGetCurrentProcess[] = { 'G','e','t','C','u','r','r','e','n','t','P','r','o','c','e','s','s','\0' };
    char szGetMessageBoxA[] = { 'M','e','s','s','a','g','e','B','o','x','A','\0' };
    char szSleep[] = { 'S','l','e','e','p','\0' };

    //拿Kernel32模块
    HMODULE hKernel32 = MyGetModuleHandle(szKernel32);
    //拿load函数地址
    FunPtr->pfnLoadLibraryA = (PFN_LoadLibraryA)MyGetProAddress(hKernel32, szLoadLibraryA);
    FunPtr->pfnLoadLibraryW = (PFN_LoadLibraryW)MyGetProAddress(hKernel32, szLoadLibraryW);

    //加载其他模块
    HMODULE hUser32 = FunPtr->pfnLoadLibraryA(szUser32);

    //拿其他函数地址
    FunPtr->pfnFindWindowA = (PFN_FindWindowA)MyGetProAddress(hUser32, szFindWindowA);
    FunPtr->pfnGetCurrentProcessId = (PFN_GetCurrentProcessId)MyGetProAddress(hKernel32, szGetCurrentProcessId);
    FunPtr->pfnGetCurrentThreadId = (PFN_GetCurrentThreadId)MyGetProAddress(hKernel32, szGetCurrentThreadId);
    FunPtr->pfnPostMessageA = (PFN_PostMessageA)MyGetProAddress(hUser32, szPostMessage);
    FunPtr->pfnSuspendThread = (PFN_SuspendThread)MyGetProAddress(hKernel32, szResumeThread);
    FunPtr->pfnGetCurrentThread = (PFN_GetCurrentThread)MyGetProAddress(hKernel32, szGetCurrentProcess);
    FunPtr->pfnMessageBoxA = (PFN_MessageBoxA)MyGetProAddress(hUser32, szGetMessageBoxA);
    FunPtr->pfnSleep = (PFN_Sleep)MyGetProAddress(hKernel32, szSleep);
}

//通过名字获得模块句柄
HMODULE MyGetModuleHandle(LPCTSTR modulename)
{
    HMODULE hret = NULL;
    //判断两种情况，如果参数为空,那就直接获得自己的，无需遍历环形链表中其他的
    if (modulename == NULL)
    {
        __asm
        {
            //获取回指自身的self指针
            mov eax, dword ptr fs : [0x18]
            //获得PEB(进程环境块)指针
            mov eax, dword ptr[eax + 0x30]
            //获得ImageBase
            mov eax, dword ptr[eax + 0x8]
            mov hret, eax
        }
    }
    //如果要找其余模块，遍历环形链表,ASCII转成UNICODE
    else
    {
        //环形链表地址
        DWORD plst = NULL;
        //环形链表起始地址
        DWORD plstbegin = NULL;
        //环形链表下一个地址
        DWORD pnextlst = NULL;
        //当前遍历的
        DWORD pcurrentlst = NULL;
        //要比较的字符串地址
        DWORD pdstname = NULL;
        //要比较的字符串的长度
        DWORD pdstnamelen = NULL;
        //源字符串缓冲区
        char srcnamebuf[MAX_PATH];
        //取出UNIDOE串转成ASCII串的缓冲区
        char dstnamebuf[MAX_PATH];
        //是否模块名相同的标识
        BOOL ismodule = FALSE;

        //数据区初始化为0
        for (int i = 0; i < MAX_PATH; ++i)
        {
            srcnamebuf[i] = 0;
            dstnamebuf[i] = 0;
        }

        //参数大写统一小写,大写字母ASCII增32 A+32 = a
        for (int i = 0; i < modulename[i] != '\0'; i++)
        {
            srcnamebuf[i] = modulename[i];
            if (srcnamebuf[i] >= 'A'&& srcnamebuf[i] <= 'Z')
            {
                srcnamebuf[i] += 32;
            }
        }

        //汇编获得基本信息
        __asm
        {
            //获取回指自身的self指针
            mov eax, dword ptr fs : [0x18]
            //获得PEB(进程环境块)指针
                mov eax, dword ptr[eax + 0x30]
                //获得装载模块LDR信息
                mov eax, dword ptr[eax + 0xc]
                //获得环形链表 
                mov eax, dword ptr[eax + 0xc]
                //保存环形链表
                mov plst, eax
                //保存头结点
                mov plstbegin, eax
                //保存当前的，遍历使用
                mov pcurrentlst, eax
        }
        //循环遍历名称相同的匹配项
        while (1)
        {
            //指针指向模块名区域取出size和长度 高16位size 低16位len
            pdstnamelen = *((int*)(pcurrentlst + 0x2c));
            //求出长度
            pdstnamelen = pdstnamelen & 0x0000ffff;
            //获得指向目标的字符串地址
            pdstname = *((int*)(pcurrentlst + 0x30));

            //UNICODE转ASCII                                        
            for (ULONG i = 0; i < pdstnamelen; i++)
            {
                dstnamebuf[i / 2] = *((char*)(pdstname + i));
                //算上for循环的，每次下标+2
                i++;
            }

            //目标字符串大写转小写
            for (int i = 0; dstnamebuf[i] != '\0'; i++)
            {
                if (dstnamebuf[i] >= 'A' && dstnamebuf[i] <= 'Z')
                {
                    dstnamebuf[i] += 32;
                }
            }

            //以目标字符串长度为准,并且源字符串循环判断结束
            for (int i = 0; dstnamebuf[i] != '\0'; i++)
            {
                if (srcnamebuf[i] != dstnamebuf[i])
                {
                    ismodule = FALSE;
                    break;
                }
                //要查询的名称不等于目标名称
                if (srcnamebuf[i] == '\0' && dstnamebuf[i] != '\0')
                {
                    ismodule = FALSE;
                    break;
                }
                ismodule = TRUE;
            }
            //要查找的字符串完全相同
            if (ismodule == TRUE)
            {
                hret = (HMODULE)*((int*)(pcurrentlst + 0x18));
                break;
            }

            //遍历到下一次之前，请空本次的目标比较字符串缓冲区
            for (int i = 0; dstnamebuf[i] != '\0'; i++)
            {
                dstnamebuf[i] = '\0';
            }

            //如果正好一圈,退出循环
            if (pnextlst == plstbegin)
            {
                break;
            }
            pnextlst = *((int*)(pcurrentlst));
            pcurrentlst = pnextlst;
        }

    }
    return hret;
}

//拿模块导出的函数地址
FARPROC MyGetProAddress(HMODULE hModule, LPCSTR lpProcName)
{
    //1.定位导出表
    IMAGE_DOS_HEADER* DosHeader = (IMAGE_DOS_HEADER*)hModule;
    IMAGE_NT_HEADERS* NtHeader = (IMAGE_NT_HEADERS *)((char*)hModule + DosHeader->e_lfanew);

    IMAGE_EXPORT_DIRECTORY* ExportTable = (IMAGE_EXPORT_DIRECTORY*)((char*)NtHeader->OptionalHeader.DataDirectory[0].VirtualAddress + (DWORD)hModule);
    //2.取出重要数据
    DWORD dwBase = ExportTable->Base;
    DWORD dwNumberOfFunctions = ExportTable->NumberOfFunctions;
    DWORD dwNumberOfNames = ExportTable->NumberOfNames;
    //导出地址表
    DWORD dwAddressOfFunctions = ExportTable->AddressOfFunctions;
    //导出名称表
    DWORD dwAddressOfNames = ExportTable->AddressOfNames;
    //导出序号表
    DWORD dwAddressOfNameOrdinals = ExportTable->AddressOfNameOrdinals;
    dwAddressOfFunctions += (DWORD)hModule;
    dwAddressOfNames += (DWORD)hModule;
    dwAddressOfNameOrdinals += (DWORD)hModule;
    //3.序号daochu
    DWORD dwIndex = 0;
    if ((DWORD)lpProcName <= 0x0ffff)
    {
        dwIndex = (DWORD)lpProcName - dwBase;
        if (dwIndex > dwNumberOfFunctions)
        {
            return NULL;
        }
    }
    //4.名称导出
    else
    {
        //求出输入的字符串长度
        DWORD dwProcNameLength = 0;
        for (int i = 0; ; i++)
        {
            if (lpProcName[i] == 0x0)
            {
                break;
            }
            dwProcNameLength++;
        }
        BOOL bFlag = FALSE;
        for (ULONG i = 0; i < dwNumberOfNames; i++)
        {
            if (bFlag == TRUE)
            {
                break;
            }
            DWORD lpNameAddress = dwAddressOfNames + (i * sizeof(DWORD));
            char* szCurrentProcName = (char*)* (DWORD*)lpNameAddress;
            szCurrentProcName += (DWORD)hModule;
            //循环比较字符串是否相等

            DWORD dwCurrentLengt = 0;
            //求出当前函数名长度
            for (int i = 0;; i++)
            {

                if (szCurrentProcName[i] == 0x0)
                {
                    break;
                }
                dwCurrentLengt++;
            }

            //长度相等
            if (dwCurrentLengt == dwProcNameLength)
            {
                for (ULONG j = 0; j < dwCurrentLengt; j++)
                {
                    if (szCurrentProcName[j] != lpProcName[j])
                    {
                        break;
                    }
                    //查询到了函数名
                    if ((szCurrentProcName[j] == lpProcName[j]) && j == dwCurrentLengt - 1)
                    {
                        //取出对应的序号
                        dwIndex = *(WORD*)(dwAddressOfNameOrdinals + sizeof(WORD)*i);
                        bFlag = TRUE;
                    }
                }
            }
            //未查询到
            if (i == (dwNumberOfNames - 1) && bFlag == FALSE)
            {
                return NULL;
            }
        }
    }
    dwAddressOfFunctions += (sizeof(DWORD) * dwIndex);
    DWORD dwQueryAddress = (DWORD)((char*)(*(DWORD*)dwAddressOfFunctions) + (DWORD)hModule);
    return (FARPROC)dwQueryAddress;
}

BOOL LoadPe(PENVIRONMENT pEnv, void* PeFile)
{
    IMAGE_DOS_HEADER* lpDosHead = (IMAGE_DOS_HEADER*)PeFile;
    IMAGE_NT_HEADERS* lpNtHead = (IMAGE_NT_HEADERS*)((char*)PeFile + lpDosHead->e_lfanew);
    IMAGE_FILE_HEADER* lpFileHead = &lpNtHead->FileHeader;//文件头地址
    IMAGE_OPTIONAL_HEADER32* lpOptionalHeader = &lpNtHead->OptionalHeader;//可选头地址
    IMAGE_DATA_DIRECTORY* lpDirectory = lpOptionalHeader->DataDirectory;//数据目录地址

    DWORD lpImportTable = lpOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;

    DWORD dwSizeofSection = lpFileHead->NumberOfSections;//节表数量
    DWORD dwSizeOfOptional = lpFileHead->SizeOfOptionalHeader;//可选头大小
    IMAGE_SECTION_HEADER* lpSection = (IMAGE_SECTION_HEADER*)((char*)lpOptionalHeader + dwSizeOfOptional);//节表首地址

    DWORD dwSizeOfImage = lpOptionalHeader->SizeOfImage;
    DWORD dwImageBase = lpOptionalHeader->ImageBase;//映像基址
    lpImportTable += dwImageBase;//导入表地址
    DWORD dwAddressOfEntryPoint = lpOptionalHeader->AddressOfEntryPoint;//程序入口点
    dwAddressOfEntryPoint += dwImageBase;
    DWORD dwSizeOfHead = lpOptionalHeader->SizeOfHeaders;
    DWORD dwOldProtect = 0;
    if (pEnv->pfnVirtualProtect((VOID*)dwImageBase, dwSizeOfImage, PAGE_EXECUTE_READWRITE, &dwOldProtect) == 0)
    {
        return false;
    }

    //拷贝头部
    mymemcpy((void*)dwImageBase, PeFile, dwSizeOfHead);
    //解析节表
    for (ULONG i = 0; i < dwSizeofSection; i++)
    {
        //修正内存节表起始地址
        DWORD dwVirAddress = lpSection->VirtualAddress;
        dwVirAddress += dwImageBase;
        //文件偏移
        DWORD dwFileOffset = lpSection->PointerToRawData;
        //大小
        DWORD dwSizeOfSection = lpSection->SizeOfRawData;
        //拷贝节数据
        mymemcpy((void*)dwVirAddress, ((char*)PeFile + dwFileOffset), dwSizeOfSection);
        lpSection++;
    }

    //填入IAT
    IMAGE_IMPORT_DESCRIPTOR  ZeroImport;
    mymemset(&ZeroImport, 0, sizeof(ZeroImport));
    while (mymemcmp(&ZeroImport, (void*)lpImportTable, sizeof(IMAGE_IMPORT_DESCRIPTOR)) != 0)
    {
        IMAGE_IMPORT_DESCRIPTOR* lpCuurentImport = (IMAGE_IMPORT_DESCRIPTOR*)lpImportTable;
        lpImportTable += sizeof(IMAGE_IMPORT_DESCRIPTOR);
        //修正
        DWORD lpIat = lpCuurentImport->FirstThunk;
        lpIat += dwImageBase;
        //检查是否是无效pe
        if (*(DWORD*)lpIat == 0)
        {
            continue;

        }
        DWORD lpInt = lpCuurentImport->OriginalFirstThunk;
        if (lpInt == NULL)
        {
            lpInt = lpCuurentImport->FirstThunk;
        }
        //修正INT
        lpInt += dwImageBase;
        //无效项
        if (lpInt == NULL)
        {
            return FALSE;
        }
        DWORD lpDllName = lpCuurentImport->Name;
        lpDllName += dwImageBase;
        HMODULE hModule = pEnv->pfnLoadLibraryA((char*)lpDllName);
        if (hModule == NULL)
        {
            return FALSE;
        }
        int i = 0;
        DWORD lpFun = 0;
        while (*(DWORD*)lpInt != 0)
        {
            //字符串导出
            if (((*(DWORD*)lpInt) & 0x80000000) == 0)
            {
                lpFun = (DWORD)((char*)(*(DWORD*)lpInt) + dwImageBase + 2);
            }
            //序号导出
            else
            {
                lpFun = *(DWORD*)lpInt & 0x0ffff;
            }

            DWORD lpPfnAddress = (DWORD)MyGetProAddress(hModule, (char*)lpFun);
            if (lpPfnAddress == NULL)
            {
                return FALSE;
            }
            *((DWORD*)lpIat + i) = lpPfnAddress;
            i++;
            lpInt = (DWORD)((DWORD*)lpInt + 1);
        }

    }
    __asm
    {
        jmp dwAddressOfEntryPoint;
    }
    return TRUE;
}

//自实现C库函数
int __cdecl mymemcmp(
    const void * buf1,
    const void * buf2,
    size_t count
    )
{
    if (!count)
        return(0);

    while (--count && *(char *)buf1 == *(char *)buf2)
    {
        buf1 = (char *)buf1 + 1;
        buf2 = (char *)buf2 + 1;
    }

    return(*((unsigned char *)buf1) - *((unsigned char *)buf2));
}


void * __cdecl mymemset(
    void *dst,
    int val,
    size_t count
    )
{
    void *start = dst;

    while (count--)
    {
        *(char *)dst = (char)val;
        dst = (char *)dst + 1;
    }

    return(start);
}

void * __cdecl mymemcpy(
    void * dst,
    const void * src,
    size_t count
    )
{
    void * ret = dst;

    /*
    * copy from lower addresses to higher addresses
    */
    while (count--)
    {
        *(char *)dst = *(char *)src;
        dst = (char *)dst + 1;
        src = (char *)src + 1;
    }

    return(ret);
}