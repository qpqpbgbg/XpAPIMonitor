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
    //ɾ���豸����Ҫע���豸����ָ�����һ����������豸��Ҫ����������ɾ��
    IoDeleteDevice(DriverObject->DeviceObject);

    //ɾ����������
    UNICODE_STRING SymoblName;
    RtlInitUnicodeString(&SymoblName, SYMBOL_LINK_NAME);
    IoDeleteSymbolicLink(&SymoblName);

    KdPrint(("unInstall Driver"));
}

//�޸�����ҳ����
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

    //PDE R/Wλ��1
    *(ULONG *)PDEBase = (*(ULONG *)PDEBase) | 0x6;
    //PTE R/Wλ��1
    *(ULONG *)PTEBase = (*(ULONG *)PTEBase) | 0x6;

    KdPrint(("*(ULONG *)PDEBase = %08X\n", *(ULONG *)PDEBase));
    KdPrint(("*(ULONG *)PTEBase = %08X\n", *(ULONG *)PTEBase));

    KdPrint(("\n\n"));
}

//����
NTSTATUS IrpCreateProc(struct _DEVICE_OBJECT *DeviceObject,  struct _IRP *Irp)
{
    KdPrint(("DispatchCreate ...\n"));
    //  getlasterror()�õ��ľ������ֵ
    Irp->IoStatus.Status = STATUS_SUCCESS;
    //���ظ����������ݣ�û�о�0
    Irp->IoStatus.Information = 0;
    //���´��ݣ������ǲֻ����������е�һԱ��������ܻ����д���IRP���ģ�����༶��������Ҫ���´���
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

//����
NTSTATUS IrpCloseProc(struct _DEVICE_OBJECT *DeviceObject, struct _IRP *Irp)
{
    KdPrint(("DispatchClose...\n"));
    //  getlasterror()�õ��ľ������ֵ
    Irp->IoStatus.Status = STATUS_SUCCESS;
    //���ظ����������ݣ�û�о�0
    Irp->IoStatus.Information = 0;
    //���´��ݣ������ǲֻ����������е�һԱ��������ܻ����д���IRP���ģ�����༶��������Ҫ���´���
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS IrpDeviceContrlProc(struct _DEVICE_OBJECT *DeviceObject, struct _IRP *Irp)
{
    //��ʱ������
    ULONG uRead = 0;
    ULONG uWrite = 0x12345678;
    PIO_STACK_LOCATION pIrpStack;
    ULONG uIoControlCode;
    PVOID pIoBuffer;
    ULONG uInLength;
    ULONG uOutLength;
    NTSTATUS nStatus;
    
    //��ȡIRP����
    pIrpStack = IoGetCurrentIrpStackLocation(Irp);
    //��ȡ������
    uIoControlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;
    //��ȡ��������ַ��������������������һ����ַ��
    pIoBuffer = Irp->AssociatedIrp.SystemBuffer;
    //RING3 �������ݵĳ��ȣ�������ָ���ĳ���
    uInLength = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;
    //RING0 �������ݵĳ���,Ҳ��������ָ���ĳ���
    uOutLength = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;

    switch (uIoControlCode)
    {
    case OPER0:
    {
        //��ȡ���Ե�ַ
        memcpy(&uRead, pIoBuffer, 4);
        //�޸�������Ե�ַ��Ӧ������ҳ����Ϊ�ɶ���д
        ModifyHigh2GMemory(uRead);
        uWrite = 0x9999;
        memcpy(pIoBuffer, &uWrite, 4);
        Irp->IoStatus.Information = 4;
        nStatus = STATUS_SUCCESS;
        break;
    }
    }

    //���÷���״̬
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

    //�����豸����ͬʱ����������
    Status = IoCreateDevice(DriverObject, 0, &DeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, TRUE, &DeviceObject);
    if (!NT_SUCCESS(Status))
    {
        DbgPrint("�豸����ʧ��\n");
        return Status;
    }
    
    //�������ݽ�����ʽ,�Ը��ƻ������ķ�ʽ����
    DeviceObject->Flags |= DO_BUFFERED_IO;

    RtlInitUnicodeString(&SymbolLink, SYMBOL_LINK_NAME);
    //������������
    Status = IoCreateSymbolicLink(&SymbolLink, &DeviceName);
    if (!NT_SUCCESS(Status))
    {
        IoDeleteDevice(DeviceObject);
        KdPrint(("��������ʧ��\n"));
        return Status;
    }

    //ע����ǲ����
    DriverObject->MajorFunction[IRP_MJ_CREATE] = IrpCreateProc;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = IrpCloseProc;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IrpDeviceContrlProc;

    return STATUS_SUCCESS;
}
}


