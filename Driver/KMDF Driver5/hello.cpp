extern "C"
{
#include <ntddk.h>
}

#define DEVICE_NAME L"\\Device\\MyDevice"
#define SYMBOL_LINK_NAME L"\\??\\Test"
#define OPER0 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

extern "C"
{
void UnloadDriver(_DRIVER_OBJECT *DriverObject)
{
    //删除设备，需要注意设备这里指向的是一个链表，多个设备需要遍历链表挨个删除
    IoDeleteDevice(DriverObject->DeviceObject);

    //删除符号链接
    UNICODE_STRING SymoblName;
    RtlInitUnicodeString(&SymoblName, SYMBOL_LINK_NAME);
    IoDeleteSymbolicLink(&SymoblName);

    KdPrint(("unInstall Driver"));
}

//修改物理页属性
void  ModifyHigh2GMemory(ULONG VirtualAddress)
{
    ULONG PTEBase = 0;
    ULONG PDEBase = 0;

    KdPrint(("VirtualAddress = %08X\n", VirtualAddress));
    //DbgBreakPoint();
    PDEBase = 0xC0600000 + ((VirtualAddress >> 18) & 0x3ff8);
    PTEBase = 0xC0000000 + ((VirtualAddress >> 9) & 0x7ffff8);

    KdPrint(("PDEBase = %08X\n", PDEBase));
    KdPrint(("PTEBase = %08X\n", PTEBase));

    KdPrint(("*(ULONG *)PDEBase = %08X\n", *(ULONG *)PDEBase));
    KdPrint(("*(ULONG *)PTEBase = %08X\n", *(ULONG *)PTEBase));

    //PDE R/W位置1
    *(ULONG *)PDEBase = (*(ULONG *)PDEBase) | 0x6;
    //PTE R/W位置1
    *(ULONG *)PTEBase = (*(ULONG *)PTEBase) | 0x6;

    KdPrint(("*(ULONG *)PDEBase = %08X\n", *(ULONG *)PDEBase));
    KdPrint(("*(ULONG *)PTEBase = %08X\n", *(ULONG *)PTEBase));

    KdPrint(("\n\n"));
}

//创建
NTSTATUS IrpCreateProc(struct _DEVICE_OBJECT *DeviceObject,  struct _IRP *Irp)
{
    KdPrint(("DispatchCreate ...\n"));
    //  getlasterror()得到的就是这个值
    Irp->IoStatus.Status = STATUS_SUCCESS;
    //返回给三环的数据，没有就0
    Irp->IoStatus.Information = 0;
    //向下传递，这个派遣只是这个流程中的一员，后面可能还会有处理IRP包的，比如多级驱动就需要向下传递
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

//控制
NTSTATUS IrpCloseProc(struct _DEVICE_OBJECT *DeviceObject, struct _IRP *Irp)
{
    KdPrint(("DispatchClose...\n"));
    //  getlasterror()得到的就是这个值
    Irp->IoStatus.Status = STATUS_SUCCESS;
    //返回给三环的数据，没有就0
    Irp->IoStatus.Information = 0;
    //向下传递，这个派遣只是这个流程中的一员，后面可能还会有处理IRP包的，比如多级驱动就需要向下传递
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS IrpDeviceContrlProc(struct _DEVICE_OBJECT *DeviceObject, struct _IRP *Irp)
{
    //临时变量；
    ULONG uRead = 0;
    ULONG uWrite = 0x12345678;
    PIO_STACK_LOCATION pIrpStack;
    ULONG uIoControlCode;
    PVOID pIoBuffer;
    ULONG uInLength;
    ULONG uOutLength;
    NTSTATUS nStatus;
    
    //获取IRP数据
    pIrpStack = IoGetCurrentIrpStackLocation(Irp);
    //获取控制码
    uIoControlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;
    //获取缓冲区地址（输入和输出缓冲区都是一个地址）
    pIoBuffer = Irp->AssociatedIrp.SystemBuffer;
    //RING3 发送数据的长度，由三环指定的长度
    uInLength = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;
    //RING0 发送数据的长度,也是由三环指定的长度
    uOutLength = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;

    switch (uIoControlCode)
    {
    case OPER0:
    {
        //读取线性地址
        memcpy(&uRead, pIoBuffer, 4);
        //修改这个线性地址对应的物理页属性为可读可写
        ModifyHigh2GMemory(uRead);
        uWrite = 0x9999;
        memcpy(pIoBuffer, &uWrite, 4);
        Irp->IoStatus.Information = 4;
        nStatus = STATUS_SUCCESS;
        break;
    }
    }

    //设置返回状态
    Irp->IoStatus.Status = nStatus;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return nStatus;
}

NTSTATUS DriverEntry(__in _DRIVER_OBJECT *DriverObject, PSTRING RegisterPath)
{
    DriverObject->DriverUnload = &UnloadDriver;
    UNICODE_STRING DeviceName;
    PDEVICE_OBJECT DeviceObject;
    UNICODE_STRING SymbolLink;
    NTSTATUS Status = 0;
    RtlInitUnicodeString(&DeviceName, DEVICE_NAME);
    DbgBreakPoint();

    //创建设备对象，同时绑定驱动对象
    Status = IoCreateDevice(DriverObject, 0, &DeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, TRUE, &DeviceObject);
    if (!NT_SUCCESS(Status))
    {
        DbgPrint("设备创建失败\n");
        return Status;
    }
    
    //设置数据交互方式,以复制缓冲区的方式拷贝
    DeviceObject->Flags |= DO_BUFFERED_IO;

    RtlInitUnicodeString(&SymbolLink, SYMBOL_LINK_NAME);
    //创建符号链接
    Status = IoCreateSymbolicLink(&SymbolLink, &DeviceName);
    if (!NT_SUCCESS(Status))
    {
        IoDeleteDevice(DeviceObject);
        KdPrint(("创建链接失败\n"));
        return Status;
    }

    //注册派遣函数
    DriverObject->MajorFunction[IRP_MJ_CREATE] = IrpCreateProc;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = IrpCloseProc;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IrpDeviceContrlProc;

    return STATUS_SUCCESS;
}
}


