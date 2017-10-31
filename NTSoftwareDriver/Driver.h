#pragma once
#include"PreInclude.h"

// ��Ӧ�ò����������һ���ַ�����
#define  CWK_DVC_SEND_STR \
	(ULONG)CTL_CODE( \
	FILE_DEVICE_UNKNOWN, \
	0x911,METHOD_BUFFERED, \
	FILE_WRITE_DATA)

// ��������ȡһ���ַ���
#define  CWK_DVC_RECV_STR \
	(ULONG)CTL_CODE( \
	FILE_DEVICE_UNKNOWN, \
	0x912,METHOD_BUFFERED, \
	FILE_READ_DATA)

typedef struct tagDeviceExtension
{
	PDEVICE_OBJECT pdevice;
	UNICODE_STRING device_name;
	UNICODE_STRING symbol_link_name;
}DeviceExtension, *pDeviceExtension;

#pragma INITCODE
NTSTATUS CreateDevice(IN PDRIVER_OBJECT pdriver_object);

#pragma PAGEDCODE
VOID OnDriverUnload(IN PDRIVER_OBJECT pdriver_object);

#pragma PAGEDCODE
NTSTATUS DispatchRoutine(IN PDEVICE_OBJECT _pdevice_object, IN PIRP pirp);


