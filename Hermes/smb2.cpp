#include "smb2.h"

void smb2::ListCatalog(tVariant* paParams, tVariant* pvarRetValue, const long lSizeArray)
{
	
	if (TV_VT(&paParams[0]) != VTYPE_PWSTR)
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"������ ��������: ������ - ��� ������������");
		return;
	}

	if (TV_VT(&paParams[1]) != VTYPE_PWSTR)
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"������ ��������: ������ - ������ ������������ (���� ����)");
		return;
	}

	if (TV_VT(&paParams[2]) != VTYPE_PWSTR)
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"������ ��������: ������ - ���� (ip/��� ����)");
		return;
	}

	if (!isNumericParameter(&paParams[3]))
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"��������� ��������: �����, ��������������� - ���� (���� ����, �� ����� ����������� 445 ����)");
		return;
	}

	double port = numericValue(&paParams[4]);

	if (static_cast<int>(port) != port)
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"����� ��������: �����, ��������������� - ���� (���� ����, �� ����� ����������� 445 ����)");
		return;
	}

	if (TV_VT(&paParams[5]) != VTYPE_PWSTR)
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"������ ��������: ������ - ����� (�� ���������� - ���� ������� ������� ������)");
		return;
	}

	if (TV_VT(&paParams[6]) != VTYPE_PWSTR)
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"������� ��������: ������ - ������� ������ (�� ���������� - ���� ������ �����)");
		return;
	}

	if (TV_VT(&paParams[7]) != VTYPE_PWSTR)
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"������� ��������: ������ - ��� ������ �������� (����������)");
		return;
	}

	if (TV_VT(&paParams[8]) != VTYPE_PWSTR)
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"������� ��������: ������ - ����, ������������� ����� ������ �������� (����� ���� ������ ������� - ���� ������ ������� �� ����)");
		return;
	}




}

void smb2::ListCatalogIndirect(tVariant* par, tVariant* pvarRetValue, const long lSizeArray)
{

}

void smb2::GetFileData(tVariant* par, tVariant* pvarRetValue, const long lSizeArray)
{

}

void smb2::GetFileDataIndirect(tVariant* par, tVariant* pvarRetValue, const long lSizeArray)
{

}

bool smb2::isNumericParameter(tVariant* par)
{
	return par->vt == VTYPE_I4 || par->vt == VTYPE_UI4 || par->vt == VTYPE_R8;
}

long smb2::numericValue(tVariant* par)
{
	long ret = 0;
	switch (par->vt)
	{
	case VTYPE_I4:
		ret = par->lVal;
		break;
	case VTYPE_UI4:
		ret = par->ulVal;
		break;
	case VTYPE_R8:
		ret = par->dblVal;
		break;
	case VTYPE_R4:
		ret = par->dblVal;
		break;
	}
	return ret;
}
