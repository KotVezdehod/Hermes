#include "smb2.h"

void smb2::ListCatalog(tVariant* paParams, tVariant* pvarRetValue, const long lSizeArray)
{
	
	if (TV_VT(&paParams[0]) != VTYPE_PWSTR)
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"Первый параметр: строка - имя пользователя");
		return;
	}

	if (TV_VT(&paParams[1]) != VTYPE_PWSTR)
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"Второй параметр: строка - пароль пользователя (если есть)");
		return;
	}

	if (TV_VT(&paParams[2]) != VTYPE_PWSTR)
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"Третий параметр: строка - хост (ip/имя узла)");
		return;
	}

	if (!isNumericParameter(&paParams[3]))
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"Четвертый параметр: целое, неотрицательное - порт (если ноль, то будет использован 445 порт)");
		return;
	}

	double port = numericValue(&paParams[4]);

	if (static_cast<int>(port) != port)
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"Пятый параметр: целое, неотрицательное - порт (если ноль, то будет использован 445 порт)");
		return;
	}

	if (TV_VT(&paParams[5]) != VTYPE_PWSTR)
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"Шестой параметр: строка - домен (не обязателен - если указана рабочая группа)");
		return;
	}

	if (TV_VT(&paParams[6]) != VTYPE_PWSTR)
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"Седьмой параметр: строка - рабочая группа (не обязателен - если указан домен)");
		return;
	}

	if (TV_VT(&paParams[7]) != VTYPE_PWSTR)
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"Восьмой параметр: строка - имя общего каталога (обязателен)");
		return;
	}

	if (TV_VT(&paParams[8]) != VTYPE_PWSTR)
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"Девятый параметр: строка - путь, относительный имени общего каталога (может быть пустой строкой - если копать глубоко не надо)");
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
