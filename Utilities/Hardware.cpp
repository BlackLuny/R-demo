#pragma once
#include"Hardware.h"
#include"Registry.h"
#include"Utilities.h"
#include<atlconv.h>

void GuruLib::Hardware::GetMachineGuid(std::wstring &guid)
{
	GuruLib::Registry reg(HKEY_LOCAL_MACHINE);
	if (reg.SetKey(L"SOFTWARE\\Microsoft\\Cryptography"))
	{
		reg.ReadString(L"MachineGuid", L"", guid);
	}
}

unsigned long GuruLib::Hardware::CPU::GetCoreCount()
{
	SYSTEM_INFO sysinfo;
	::GetNativeSystemInfo(&sysinfo);
	return sysinfo.dwNumberOfProcessors;
}

void GuruLib::Hardware::CPU::GetModelName(std::string &model_name)
{
	int CPUInfo[4] = { -1 };
	char CPUBrandString[0x40];
	__cpuid(CPUInfo, 0x80000000);
	unsigned int nExIds = CPUInfo[0];

	memset(CPUBrandString, 0, GuruLib::ArraySize(CPUBrandString));

	// Get the information associated with each extended ID.
	for (int i = 0x80000000; i <= nExIds; ++i)
	{
		__cpuid(CPUInfo, i);
		// Interpret CPU brand string.
		if (i == 0x80000002)
			memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
		else if (i == 0x80000003)
			memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
		else if (i == 0x80000004)
			memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
	}
	model_name = CPUBrandString;
}

unsigned long long GuruLib::Hardware::Memory::GetSizeInKiloBytes()
{
	unsigned long long size;
	if(!::GetPhysicallyInstalledSystemMemory(&size))
	{
		return 0;
	}
	return size;
}


#ifndef MACRO_T_DEVICE_PROPERTY  
#define MACRO_T_DEVICE_PROPERTY  

#define PROPERTY_MAX_LEN    128 // �����ֶ���󳤶�  
typedef struct _T_DEVICE_PROPERTY
{
	TCHAR szProperty[PROPERTY_MAX_LEN];
} T_DEVICE_PROPERTY;
#endif  

#define WMI_QUERY_TYPENUM   7   // WMI��ѯ֧�ֵ�������  
#pragma comment (lib, "comsuppw.lib")  
#pragma comment (lib, "wbemuuid.lib")  

typedef struct _T_WQL_QUERY
{
	CHAR*   szSelect;       // SELECT���  
	WCHAR*  szProperty;     // �����ֶ�  
} T_WQL_QUERY;

// WQL��ѯ���  
const T_WQL_QUERY szWQLQuery[] = {
	// ����ԭ��MAC��ַ  
	"SELECT * FROM Win32_NetworkAdapter WHERE (MACAddress IS NOT NULL) AND (NOT (PNPDeviceID LIKE 'ROOT%'))",
	L"PNPDeviceID",

	// Ӳ�����к�  
	"SELECT * FROM Win32_DiskDrive WHERE (SerialNumber IS NOT NULL) AND (MediaType LIKE 'Fixed hard disk%')",
	L"SerialNumber",

	// �������к�  
	"SELECT * FROM Win32_BaseBoard WHERE (SerialNumber IS NOT NULL)",
	L"SerialNumber",

	// ������ID  
	"SELECT * FROM Win32_Processor WHERE (ProcessorId IS NOT NULL)",
	L"ProcessorId",

	// BIOS���к�  
	"SELECT * FROM Win32_BIOS WHERE (SerialNumber IS NOT NULL)",
	L"SerialNumber",

	// �����ͺ�  
	"SELECT * FROM Win32_BaseBoard WHERE (Product IS NOT NULL)",
	L"Product",

	// ������ǰMAC��ַ  
	"SELECT * FROM Win32_NetworkAdapter WHERE (MACAddress IS NOT NULL) AND (NOT (PNPDeviceID LIKE 'ROOT%'))",
	L"MACAddress",
};


static BOOL WMI_DoWithProperty(INT iQueryType, TCHAR *szProperty, UINT uSize)
{
	BOOL isOK = TRUE;

	switch (iQueryType)
	{
	case 6:     // ������ǰMAC��ַ  
				// ȥ��ð��  
		std::remove(szProperty, szProperty + _tcslen(szProperty) + 1, L':');
		break;
	default:
		// ȥ���ո�  
		std::remove(szProperty, szProperty + _tcslen(szProperty) + 1, L' ');
	}

	return isOK;
}

// ����Windows Management Instrumentation��Windows����淶��  
INT WMI_DeviceQuery(INT iQueryType, T_DEVICE_PROPERTY *properties, INT iSize)
{
	HRESULT hres;
	INT iTotal = 0;

	// �жϲ�ѯ�����Ƿ�֧��  
	if ((iQueryType < 0) || (iQueryType >= sizeof(szWQLQuery) / sizeof(T_WQL_QUERY)))
	{
		return -1;  // ��ѯ���Ͳ�֧��  
	}

	// ��ʼ��COM  
	hres = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hres))
	{
		return -2;
	}

	// ����COM�İ�ȫ��֤����  
	hres = CoInitializeSecurity(
		NULL,
		-1,
		NULL,
		NULL,
		RPC_C_AUTHN_LEVEL_DEFAULT,
		RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL,
		EOAC_NONE,
		NULL
	);
	if (FAILED(hres))
	{
		CoUninitialize();
		return -2;
	}

	// ���WMI����COM�ӿ�  
	IWbemLocator *pLoc = NULL;
	hres = CoCreateInstance(
		CLSID_WbemLocator,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator,
		reinterpret_cast<LPVOID*>(&pLoc)
	);
	if (FAILED(hres))
	{
		CoUninitialize();
		return -2;
	}

	// ͨ�����ӽӿ�����WMI���ں˶�����"ROOT\\CIMV2"  
	IWbemServices *pSvc = NULL;
	hres = pLoc->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"),
		NULL,
		NULL,
		NULL,
		0,
		NULL,
		NULL,
		&pSvc
	);
	if (FAILED(hres))
	{
		pLoc->Release();
		CoUninitialize();
		return -2;
	}

	// �����������İ�ȫ����  
	hres = CoSetProxyBlanket(
		pSvc,
		RPC_C_AUTHN_WINNT,
		RPC_C_AUTHZ_NONE,
		NULL,
		RPC_C_AUTHN_LEVEL_CALL,
		RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL,
		EOAC_NONE
	);
	if (FAILED(hres))
	{
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return -2;
	}

	// ͨ�������������WMI��������  
	IEnumWbemClassObject *pEnumerator = NULL;
	hres = pSvc->ExecQuery(
		bstr_t("WQL"),
		bstr_t(szWQLQuery[iQueryType].szSelect),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&pEnumerator
	);
	if (FAILED(hres))
	{
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return -3;
	}

	// ѭ��ö�����еĽ������    
	while (pEnumerator)
	{
		IWbemClassObject *pclsObj = NULL;
		ULONG uReturn = 0;

		if ((properties != NULL) && (iTotal >= iSize))
		{
			break;
		}

		pEnumerator->Next(
			WBEM_INFINITE,
			1,
			&pclsObj,
			&uReturn
		);

		if (uReturn == 0)
		{
			break;
		}

		if (properties != NULL)
		{   // ��ȡ����ֵ  
			VARIANT vtProperty;

			VariantInit(&vtProperty);
			pclsObj->Get(szWQLQuery[iQueryType].szProperty, 0, &vtProperty, NULL, NULL);
			USES_CONVERSION; StringCchCopy(properties[iTotal].szProperty, PROPERTY_MAX_LEN, W2T(vtProperty.bstrVal));
			VariantClear(&vtProperty);

			// ������ֵ����һ���Ĵ���  
			if (WMI_DoWithProperty(iQueryType, properties[iTotal].szProperty, PROPERTY_MAX_LEN))
			{
				iTotal++;
			}
		}
		else
		{
			iTotal++;
		}

		pclsObj->Release();
	} // End While  

	  // �ͷ���Դ  
	pEnumerator->Release();
	pSvc->Release();
	pLoc->Release();
	CoUninitialize();

	return iTotal;
}
void GuruLib::Hardware::Baseboard::GetSerialNumber(std::string &serial_number)
{
#pragma comment (lib, "comsuppw.lib")  
#pragma comment (lib, "wbemuuid.lib")  

	T_DEVICE_PROPERTY mac;
	WMI_DeviceQuery(2, &mac, 2);

	std::wstring serial_number_w = mac.szProperty;
	TypeConverter::WStringToString(serial_number_w, serial_number);

}
