#include <codecvt>
#include <dirent.h>
#include <chrono>
#include <thread>
#include <string>
#include <iostream>

#include "pch.h"
#include "AddInNative.h"
#include "ConversionWchar.h"
#include "wchar.h"
#include "../jni/jnienv.h"
#include "../include/IAndroidComponentHelper.h"
#include "Json.h"
#include "samba.h"
#include "RegExFor1c.h"
#include "valiJson.hpp"

using std::string;
using std::wstring;
using std::wstring_convert;
using std::codecvt_utf8_utf16;

static const wchar_t g_ComponentNameAddIn[] = L"Hermes";
static WcharWrapper s_ComponentClass(g_ComponentNameAddIn);
// This component supports 2.1 version
const long g_VersionAddIn = 2100;
static AppCapabilities g_capabilities = eAppCapabilitiesInvalid;


//---------------------------------------------------------------------------//
long GetClassObject(const WCHAR_T* wsName, IComponentBase** pInterface)
{
	if (!*pInterface)
	{
		*pInterface = new Hermes();
		return (long)*pInterface;
	}
	return 0;
}

//---------------------------------------------------------------------------//
AppCapabilities SetPlatformCapabilities(const AppCapabilities capabilities)
{
	g_capabilities = capabilities;
	return eAppCapabilitiesLast;
}

//---------------------------------------------------------------------------//
long DestroyObject(IComponentBase** pInterface)
{
	if (!*pInterface)
		return -1;

	delete* pInterface;
	*pInterface = 0;
	return 0;
}

//---------------------------------------------------------------------------//
const WCHAR_T* GetClassNames()
{
	return s_ComponentClass;
}

Hermes::Hermes() : m_iConnect(nullptr), m_iMemory(nullptr), isScreenOn(false)
{
	
}

Hermes::~Hermes()
{
	delete mFileWorks;
}

/////////////////////////////////////////////////////////////////////////////
// IInitDoneBase
//---------------------------------------------------------------------------//
bool Hermes::Init(void* pConnection)
{
	m_iConnect = (IAddInDefBaseEx*)pConnection;
	return m_iConnect != nullptr;
}

//---------------------------------------------------------------------------//
bool Hermes::setMemManager(void* mem)
{
	m_iMemory = (IMemoryManager*)mem;
	mFileWorks = new FileWorks(m_iMemory);
	return m_iMemory != nullptr;
}

//---------------------------------------------------------------------------//
long Hermes::GetInfo()
{
	return g_VersionAddIn;
}

//---------------------------------------------------------------------------//
void Hermes::Done()
{
	m_iConnect = nullptr;
	m_iMemory = nullptr;
}

/////////////////////////////////////////////////////////////////////////////
// ILanguageExtenderBase
//---------------------------------------------------------------------------//
bool Hermes::RegisterExtensionAs(WCHAR_T** wsExtensionName)
{
	const wchar_t* wsExtension = g_ComponentNameAddIn;
	uint32_t iActualSize = static_cast<uint32_t>(::wcslen(wsExtension) + 1);

	if (m_iMemory)
	{
		if (m_iMemory->AllocMemory((void**)wsExtensionName, iActualSize * sizeof(WCHAR_T)))
		{
			convToShortWchar(wsExtensionName, wsExtension, iActualSize);
			return true;
		}
	}

	return false;
}

//---------------------------------------------------------------------------//
long Hermes::GetNProps()
{
	// You may delete next lines and add your own implementation code here
	return ePropLast;
}

//---------------------------------------------------------------------------//
long Hermes::FindProp(const WCHAR_T* wsPropName)
{
	long plPropNum = -1;
	wchar_t* propName = 0;
	convFromShortWchar(&propName, wsPropName);

	plPropNum = findName(g_PropNames, propName, ePropLast);

	if (plPropNum == -1)
		plPropNum = findName(g_PropNamesRu, propName, ePropLast);

	delete[] propName;
	return plPropNum;
}

//---------------------------------------------------------------------------//
const WCHAR_T* Hermes::GetPropName(long lPropNum, long lPropAlias)
{
	if (lPropNum >= ePropLast)
		return NULL;

	wchar_t* wsCurrentName = NULL;
	WCHAR_T* wsPropName = NULL;

	switch (lPropAlias)
	{
	case 0: // First language (english)
		wsCurrentName = (wchar_t*)g_PropNames[lPropNum];
		break;
	case 1: // Second language (local)
		wsCurrentName = (wchar_t*)g_PropNamesRu[lPropNum];
		break;
	default:
		return 0;
	}

	uint32_t iActualSize = static_cast<uint32_t>(wcslen(wsCurrentName) + 1);

	if (m_iMemory && wsCurrentName)
	{
		if (m_iMemory->AllocMemory((void**)&wsPropName, iActualSize * sizeof(WCHAR_T)))
			convToShortWchar(&wsPropName, wsCurrentName, iActualSize);
	}

	return wsPropName;
}

//---------------------------------------------------------------------------//
bool Hermes::GetPropVal(const long lPropNum, tVariant* pvarPropVal)
{
	switch (lPropNum)
	{

	case ePropDevice:
	{

		IAndroidComponentHelper* helper = (IAndroidComponentHelper*)m_iConnect->GetInterface(eIAndroidComponentHelper);
		pvarPropVal->vt = VTYPE_EMPTY;
		if (helper)
		{
			WCHAR_T* className = nullptr;
			convToShortWchar(&className, L"android.os.Build");
			jclass ccloc = helper->FindClass(className);
			delete[] className;
			className = nullptr;
			wstring wData{};
			if (ccloc)
			{
				JNIEnv* env = getJniEnv();
				jfieldID fldID = env->GetStaticFieldID(ccloc, "MANUFACTURER", "Ljava/lang/String;");
				jstring	jManufacturer = (jstring)env->GetStaticObjectField(ccloc, fldID);
				wstring wManufacturer = ToWStringJni(jManufacturer);
				env->DeleteLocalRef(jManufacturer);
				fldID = env->GetStaticFieldID(ccloc, "MODEL", "Ljava/lang/String;");
				jstring	jModel = static_cast<jstring>(env->GetStaticObjectField(ccloc, fldID));
				wstring wModel = ToWStringJni(jModel);
				env->DeleteLocalRef(jModel);
				if (wManufacturer.length())
					wData = wManufacturer + L": " + wModel;
				else
					wData = wModel;
				env->DeleteLocalRef(ccloc);
			}
			if (wData.length())
				ToV8String(wData.c_str(), pvarPropVal, m_iMemory);

			wData.clear();
		}
	}
	return true;

	case ePropCurrentBroadcastsFilters:
	{
		wstring tmp_str = L"";
		m_SendData.GetCurrentFilter(&tmp_str);
		ToV8String(tmp_str.c_str(), pvarPropVal, m_iMemory);
	}
	return true;

	case ePropLastBroadcastExtra:
	{
		wstring loc_lastBK = L"";
		m_SendData.GetLastBroadcastExtra(&loc_lastBK);
		ToV8String(loc_lastBK.c_str(), pvarPropVal, m_iMemory);
	}
	return true;

	default:
		return false;
	}

}

//---------------------------------------------------------------------------//
bool Hermes::SetPropVal(const long lPropNum, tVariant* varPropVal)
{
	switch (lPropNum)
	{

	default:
		return false;
	}
}

//---------------------------------------------------------------------------//
bool Hermes::IsPropReadable(const long lPropNum)
{
	return true;
}

//---------------------------------------------------------------------------//
bool Hermes::IsPropWritable(const long lPropNum)
{
	switch (lPropNum)
	{
	default:
		return false;
	}
}

//---------------------------------------------------------------------------//
long Hermes::GetNMethods()
{
	return eMethLast;
}

//---------------------------------------------------------------------------//
long Hermes::FindMethod(const WCHAR_T* wsMethodName)
{
	long plMethodNum = -1;
	wchar_t* name = 0;
	convFromShortWchar(&name, wsMethodName);

	plMethodNum = findName(g_MethodNames, name, eMethLast);

	if (plMethodNum == -1)
		plMethodNum = findName(g_MethodNamesRu, name, eMethLast);

	delete[] name;

	return plMethodNum;
}

//---------------------------------------------------------------------------//
const WCHAR_T* Hermes::GetMethodName(const long lMethodNum, const long lMethodAlias)
{
	if (lMethodNum >= eMethLast)
		return NULL;

	wchar_t* wsCurrentName = NULL;
	WCHAR_T* wsMethodName = NULL;

	switch (lMethodAlias)
	{
	case 0: // First language (english)
		wsCurrentName = (wchar_t*)g_MethodNames[lMethodNum];
		break;
	case 1: // Second language (local)
		wsCurrentName = (wchar_t*)g_MethodNamesRu[lMethodNum];
		break;
	default:
		return 0;
	}

	uint32_t iActualSize = static_cast<uint32_t>(wcslen(wsCurrentName) + 1);

	if (m_iMemory && wsCurrentName)
	{
		if (m_iMemory->AllocMemory((void**)&wsMethodName, iActualSize * sizeof(WCHAR_T)))
			convToShortWchar(&wsMethodName, wsCurrentName, iActualSize);
	}

	return wsMethodName;
}

//---------------------------------------------------------------------------//
long Hermes::GetNParams(const long lMethodNum)
{
	switch (lMethodNum)
	{
	case eMethDelay:
		return 1;
	case eMethTest:
		return 1;
	case eMethStartBroadcastsInterception:
		return 1;
	case eMethStopBroadcastsInterception:
		return 0;
	case eMethDeviceID:
		return 0;
	case eMethOSAbis:
		return 0;
	case eMethStartHTTP:
		return 1;
	case eMethReplyHTTP:
		return 1;
	case eMethZLIBCompress:
		return 1;
	case eMethZLIBDecompress:
		return 2;
	case eMethZLIBCompressFile:
		return 2;
	case eMethZLIBDecompressFile:
		return 2;
	case eMethPHOTORefactorImage:
		return 5;
	case eMethDecodeBarcode:
		return 1;
	case eMethScanFolder:
		return 1;
	case eMethIsDirectory:
		return 1;
	case eMethDeleteFileOrDirectory:
		return 1;
	case eMethCreateFile:
		return 1;
	case eMethCreateDirectory:
		return 2;
	case eMethRenameFileOrDirectory:
		return 2;
	case eMethWriteDataToFile:
		return 2;
	case eMethFSOPresent:
		return 1;
	case eMethReadDataFromFile:
		return 1;
	case eMethGEOStartListening:
		return 2;
	case eMethGEOGetNow:
		return 2;
	case eMethSqlLiteOpenConnection:
		return 1;
	case eMethSqlLiteCloseConnection:
		return 1;
	case eMethSqlLiteGetDbDetails:
		return 1;
	case eMethSqlLiteExecuteStatement:
		return 2;
	case eMethSqlLiteExecuteStatement_v2:
		return 2;
	case eMethSqlLiteInsertBlobData:
		return 3;
	case eMethSqlLiteSelectBlobData:
		return 2;
	case eMethSmbListCatalog:
		return 2;
	case eMethSmbGetFile:
		return 5;
	case eMethSmbPutFile:
		return 4;
	case eMethRexMatch:
		return 2;
	case eMethRexReplace:
		return 3;
	case eMethGetQR:
		return 2;
	case eMethJvdValidate:
		return 2;
	default:
		return 0;
	}
}

//---------------------------------------------------------------------------//
bool Hermes::GetParamDefValue(const long lMethodNum, const long lParamNum, tVariant* pvarParamDefValue)
{
	switch (lMethodNum)
	{
	default:
		return false;
	}
}

//---------------------------------------------------------------------------//
bool Hermes::HasRetVal(const long lMethodNum)
{
	switch (lMethodNum)
	{
	case eMethTest:
	case eMethStartBroadcastsInterception:
	case eMethStopBroadcastsInterception:
	case eMethDeviceID:
	case eMethAndroidID:
	case eMethOSAbis:
	case eMethStartHTTP:
	case eMethStopHTTP:
	case eMethZLIBCompress:
	case eMethZLIBDecompress:
	case eMethZLIBCompressFile:
	case eMethZLIBDecompressFile:
	case eMethPHOTORefactorImage:
	case eMethVersion:
	case eMethDecodeBarcode:
	case eMethScanFolder:
	case eMethIsDirectory:
	case eMethDeleteFileOrDirectory:
	case eMethCreateFile:
	case eMethCreateDirectory:
	case eMethRenameFileOrDirectory:
	case eMethWriteDataToFile:
	case eMethFSOPresent:
	case eMethReadDataFromFile:
	case eMethGEOStartListening:
	case eMethGEOStopListening:
	case eMethGEOGetNow:
	case eMethSqlLiteOpenConnection:
	case eMethSqlLiteCloseConnection:
	case eMethSqlLiteGetOpenedConnectionsList:
	case eMethSqlLiteGetDbDetails:
	case eMethSqlLiteExecuteStatement:
	case eMethSqlLiteExecuteStatement_v2:
	case eMethSqlLiteInsertBlobData:
	case eMethSqlLiteSelectBlobData:
	case eMethSmbListCatalog:
	case eMethSmbGetFile:
	case eMethSmbPutFile:
	case eMethRexMatch:
	case eMethRexReplace:
	case eMethGetQR:
	case eMethJvdValidate:
		return true;
	default:
		return false;
	}
}

//---------------------------------------------------------------------------//
bool Hermes::CallAsProc(const long lMethodNum, tVariant* paParams, const long lSizeArray)
{
	switch (lMethodNum)
	{
	case eMethDelay:
	{
		long lDelay = numericValue(paParams);
		if (lDelay > 0)
			std::this_thread::sleep_for(std::chrono::milliseconds(lDelay));

	}
	return true;

	case eMethReplyHTTP:
	{
		m_SendData.Initialize(m_iConnect, m_iMemory);
		m_SendData.ReplyHTTP(paParams);
	}
	
	return true;

	default:
		return false;
	}
}

//---------------------------------------------------------------------------//
bool Hermes::CallAsFunc(const long lMethodNum, tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{

	switch (lMethodNum)
	{
	case eMethTest:
	{
		if (!lSizeArray || !paParams)
			return false;

	}

	return true;

	case eMethStartBroadcastsInterception:

		if (!lSizeArray || !paParams)
			return false;

		switch (TV_VT(paParams))
		{
		case VTYPE_PSTR:
		{
			/*string str_from_v8 = paParams->pstrVal;
			wstring wstr_from_v8(str_from_v8.begin(), str_from_v8.end());
			m_devState.SetCurrentFilter(wstr_from_v8);*/

			ToV8String(L"pstr", pvarRetValue, m_iMemory);

		}
		return true;

		case VTYPE_PWSTR:
		{
			if (!isScreenOn)
			{
				wstring std_wstr(paParams->pwstrVal, paParams->pwstrVal + paParams->strLen);
				m_SendData.SetCurrentFilter(&std_wstr);
				m_SendData.Initialize(m_iConnect, m_iMemory);
				m_SendData.Start(&std_wstr, m_iConnect);
				ToV8String(L"ok_en", pvarRetValue, m_iMemory);
				isScreenOn = true;
				return true;
			}
			else
			{
				ToV8String(L"Перехватчик уже запущен.", pvarRetValue, m_iMemory);
				return true;
			}
		}

		return true;


		default:
			break;
		}

		ToV8String(L"ok_en", pvarRetValue, m_iMemory);
		return true;

	case eMethStopBroadcastsInterception:
	{
		wstring empty_str = L"";
		m_SendData.SetCurrentFilter(&empty_str);

		if (isScreenOn)
		{
			m_SendData.Stop();
			ToV8String(L"ok_en", pvarRetValue, m_iMemory);
			isScreenOn = false;
			return true;
		}
		else
		{
			ToV8String(L"Перехватчик не запущен.", pvarRetValue, m_iMemory);
			return true;
		}

	}
	return true;

	case eMethDeviceID:
	{
		m_SendData.Initialize(m_iConnect, m_iMemory);

		m_SendData.GetID(pvarRetValue);
	}
	return true;

	case eMethAndroidID:
	{
		m_SendData.Initialize(m_iConnect, m_iMemory);

		m_SendData.GetAndroidID(pvarRetValue);
	}
	return true;

	case eMethOSAbis:
	{
		m_SendData.Initialize(m_iConnect, m_iMemory);

		m_SendData.GetOSAbis(pvarRetValue);
	}
	return true;

	case eMethStartHTTP:
	{
		if (!lSizeArray || !paParams)
		{
			ToV8String(L"Не указан порт прослушивания!", pvarRetValue, m_iMemory);
			return true;
		}

		if (!isNumericParameter(paParams))
		{
			ToV8String(L"Номер порта должен быть числом.", pvarRetValue, m_iMemory);
			return true;
		}

		m_SendData.Initialize(m_iConnect, m_iMemory);
		m_SendData.StartHTTP(pvarRetValue, numericValue(paParams));
	}

	return true;

	case eMethStopHTTP:
	{
		m_SendData.Initialize(m_iConnect, m_iMemory);
		m_SendData.StopHTTP(pvarRetValue);
	}

	return true;

	case eMethZLIBCompress:
	{
		if (!lSizeArray || !paParams)
		{
			ToV8String(L"Отсутствует параметр с двоичными данными для архивации!", pvarRetValue, m_iMemory);
		}
		else
		{
			if (TV_VT(paParams) != VTYPE_BLOB)
			{
				ToV8String(L"Входящий параметр - не двоичные данные!", pvarRetValue, m_iMemory);
			}
			else
			{
				Compressor m_Compressor(m_iMemory);
				m_Compressor.CompressBuffer(pvarRetValue, paParams);
			}
		}

	}
	return true;

	case eMethZLIBDecompress:
	{
		if (!paParams)
		{
			ToV8String(L"Отсутствует параметр с двоичными данными и параметр с ожидаемым размером распакованных данных!", pvarRetValue, m_iMemory);
		}
		else if (lSizeArray < 2)
		{
			ToV8String(L"Недостаточно параметров!", pvarRetValue, m_iMemory);

		}
		else
		{
			if (TV_VT(&paParams[0]) != VTYPE_BLOB)
			{
				ToV8String(L"Первый входящий параметр - не двоичные данные!", pvarRetValue, m_iMemory);
			}
			else
			{

				if (!isNumericParameter(&paParams[1]))
				{
					ToV8String(L"Второй входящий параметр - не целое число!", pvarRetValue, m_iMemory);
				}
				else
				{
					Compressor m_Compressor(m_iMemory);
					m_Compressor.DecompressBuffer(pvarRetValue, &paParams[0], &paParams[1]);
				}

			}
		}
	}
	return true;

	case eMethZLIBCompressFile:
	{

		if (!paParams)
		{
			ToV8String(L"Отсутствует параметр с файлом - источником и параметр с файлом назначения!", pvarRetValue, m_iMemory);
		}
		else if (lSizeArray < 2)
		{
			ToV8String(L"Недостаточно параметров!", pvarRetValue, m_iMemory);
		}
		else
		{
			if (TV_VT(&paParams[0]) != VTYPE_PWSTR)
			{
				ToV8String(L"Первый параметр - не строка!", pvarRetValue, m_iMemory);
			}
			else if (TV_VT(&paParams[1])!=VTYPE_PWSTR)
			{
				ToV8String(L"Второй параметр - не строка!", pvarRetValue, m_iMemory);
			}
			else
			{
				Compressor m_Compressor(m_iMemory);
				m_Compressor.CompressFile(pvarRetValue, &paParams[0], &paParams[1]);

			}
		}

	}
	return true;

	case eMethZLIBDecompressFile:
	{
		if (!paParams)
		{
			ToV8String(L"Отсутствует параметр с файлом - источником и параметр с файлом назначения!", pvarRetValue, m_iMemory);
		}
		else if (lSizeArray < 2)
		{
			ToV8String(L"Недостаточно параметров!", pvarRetValue, m_iMemory);
		}
		else
		{
			if (TV_VT(&paParams[0]) != VTYPE_PWSTR)
			{
				ToV8String(L"Первый параметр - не строка!", pvarRetValue, m_iMemory);
			}
			else if (TV_VT(&paParams[1]) != VTYPE_PWSTR)
			{
				ToV8String(L"Второй параметр - не строка!", pvarRetValue, m_iMemory);
			}
			else
			{
				Compressor m_Compressor(m_iMemory);
				m_Compressor.DecompressFile(pvarRetValue, &paParams[0], &paParams[1]);

			}
		}
	}
	return true;
	
	case eMethPHOTORefactorImage:
	{
		if (!paParams)
		{
			ToV8String(L"Отсутствуют параметры функции (Путь входного файла, Путь выходного файла, Разрешение X, Разрешение Y, Качество (от 0-100))!", pvarRetValue, m_iMemory);
		}
		else
		{
			if (lSizeArray < 5)
			{
				ToV8String(L"Отсутствуют параметры функции (Путь входного файла, Путь выходного файла, Разрешение X, Разрешение Y, Качество (от 0-100))!", pvarRetValue, m_iMemory);
			}
			else
			{
				if (!isNumericParameter(&paParams[2]))
				{
					ToV8String(L"Не верный тип параматра 3 (должно быть целое, положительное число)!", pvarRetValue, m_iMemory);
				}
				else
				{
					if (!isNumericParameter(&paParams[3]))
					{
						ToV8String(L"Не верный тип параматра 4 (должно быть целое, положительное число)!", pvarRetValue, m_iMemory);
					}
					else
					{
						if (!isNumericParameter(&paParams[4]))
						{
							ToV8String(L"Не верный тип параматра 5 (должно быть целое, положительное число от 0-100)!", pvarRetValue, m_iMemory);
						}
						else
						{
							double v0 = numericValue(&paParams[2]);
							double v1 = numericValue(&paParams[3]);
							double v2 = numericValue(&paParams[4]);

							if (v0 <= 0 || v0 - static_cast<int>(v0) != 0)
							{
								ToV8String(L"Не верный тип параматра 3 (должно быть целое, положительное число)!", pvarRetValue, m_iMemory);
							}
							else
							{
								if (v1 <= 0 || v1 - static_cast<int>(v1) != 0)
								{
									ToV8String(L"Не верный тип параматра 4 (должно быть целое, положительное число)!", pvarRetValue, m_iMemory);
								}
								else
								{
									if (v2 <= 0 || v2 - static_cast<int>(v2) != 0 || v2 > 100)
									{
										ToV8String(L"Не верный тип параматра 5 (должно быть целое, положительное число от 0-100)!", pvarRetValue, m_iMemory);
									}
									else
									{

										if (TV_VT(&paParams[0]) != VTYPE_PWSTR)
										{
											ToV8String(L"Не верный тип параматра 1 (должнf быть строка)!", pvarRetValue, m_iMemory);
										}
										else
										{

											if (TV_VT(&paParams[1]) != VTYPE_PWSTR)
											{
												ToV8String(L"Не верный тип параматра 2 (должна быть строка)!", pvarRetValue, m_iMemory);
											}
											else
											{
												
												wchar_t *wch_in_fn = nullptr;
												wchar_t* wch_out_fn = nullptr;
												
												convFromShortWchar(&wch_in_fn, (&paParams[0])->pwstrVal);
												convFromShortWchar(&wch_out_fn, (&paParams[1])->pwstrVal);

												m_SendData.Initialize(m_iConnect, m_iMemory);
												m_SendData.RefactorImage(wch_in_fn, wch_out_fn, v0,v1,v2, pvarRetValue);

												delete[] wch_in_fn;
												delete[] wch_out_fn;

											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return true;

	case eMethVersion:
	{
		ToV8String(L"1.1.4", pvarRetValue, m_iMemory);
	}

	return true;

	case eMethDecodeBarcode:
	{
		if (!paParams)
		{
			DiagToV8String(pvarRetValue, m_iMemory, false, L"Отсутствуют параметры функции (Путь входного файла или двоичные данные картинки.");

		}
		else
		{
			if ((TV_VT(paParams) == VTYPE_PWSTR)|| TV_VT(paParams) == VTYPE_BLOB)
			{
				m_SendData.Initialize(m_iConnect, m_iMemory);
				m_SendData.DecodeBarcode(paParams, pvarRetValue);
			}
			else
			{
				DiagToV8String(pvarRetValue, m_iMemory, false, L"Параметр должен быть строка - URI файла картинки штрихкода или ДД картинки.");

			}
		}
	}
	return true;

	case eMethScanFolder:
	{
		mFileWorks->ScanFolder(pvarRetValue, paParams);
	}
		return true;

	case eMethIsDirectory:
	{
		mFileWorks->IsDirectory(pvarRetValue, paParams);
	}
	return true;

	case eMethCreateDirectory:
	{
		if (lSizeArray < 2)
		{
			DiagToV8String(pvarRetValue, m_iMemory, false, L"Не полный набор параметров: должно быть 2.");
		}
		else
		{
			mFileWorks->CreateDirectory(pvarRetValue, paParams);
		}

	}
	return true;

	case eMethFSOPresent:
	{
		if (lSizeArray < 1)
		{
			DiagToV8String(pvarRetValue, m_iMemory, false, L"Не полный набор параметров: должно быть 2.");
		}
		else
		{
			mFileWorks->IsFSOPresent(pvarRetValue, paParams);
		}
	}
	return true;

	case eMethDeleteFileOrDirectory:
	{
		mFileWorks->DeleteFileOrDirectory(pvarRetValue, paParams);
	}
	return true;
		
	case eMethCreateFile:
	{
			mFileWorks->CreateFile(pvarRetValue, paParams);
	}
	return true;

	case eMethRenameFileOrDirectory:
	{
		mFileWorks->RenameFileOrDirectory(pvarRetValue,paParams);
	}
	return true;

	case eMethWriteDataToFile:
	{
		if (lSizeArray < 2)
		{
			ToV8String(L"Недостаточно параметров (должно быть 2)!", pvarRetValue, m_iMemory);
		}
		else
		{
			mFileWorks->WriteDataToFile(pvarRetValue, paParams);
		}
		
	}
	return true;

	case eMethReadDataFromFile:
		mFileWorks->ReadDataFromFile(pvarRetValue, paParams);
		return true;

	case eMethGEOStartListening:
		if (TV_VT(&paParams[0]) == VTYPE_BOOL && TV_VT(&paParams[1]) == VTYPE_BOOL)
		{
			m_SendData.Initialize(m_iConnect, m_iMemory);
			m_SendData.StartGeolocation(paParams, pvarRetValue);
		}
		else
		{
			DiagToV8String(pvarRetValue, m_iMemory, false, L"Не верный тип аргументов (должны быть булевы)");
		}

		return true;

	case eMethGEOStopListening:

		m_SendData.Initialize(m_iConnect, m_iMemory);
		m_SendData.StopGeolocation(pvarRetValue);

		return true;

	case eMethGEOGetNow:

		if (TV_VT(&paParams[0]) == VTYPE_BOOL && TV_VT(&paParams[1]) == VTYPE_BOOL)
		{
			m_SendData.Initialize(m_iConnect, m_iMemory);
			m_SendData.GetLocationNow(paParams,pvarRetValue);
		}
		else
		{
			DiagToV8String(pvarRetValue, m_iMemory, false, L"Не верный тип аргументов (должны быть булевы)");
		}

		return true;

	case eMethSqlLiteOpenConnection:
	{

		if (paParams->vt != VTYPE_PWSTR)
		{
			DiagStructure(false, L"Параметр должен быть строкой!", L"", pvarRetValue, m_iMemory);
			return true;
		}

		wchar_t* dbName = nullptr;
		convFromShortWchar(&dbName, paParams->pwstrVal);
		wstring wsDbName = wstring(dbName);

		Json::Value root = mSQLt.Open(&wsDbName);
		string desc = root["Description"].asString();
		DiagStructure(root["Status"].asBool(), &desc, &root["Data"], pvarRetValue, m_iMemory);

		delete dbName;

	}
	return true;

	case eMethSqlLiteCloseConnection:
	{
		if (!isNumericParameter(paParams))
		{
			DiagStructure(false, L"Параметр должен быть числом!", L"", pvarRetValue, m_iMemory);
			return true;
		}

		Json::Value root = mSQLt.Close(numericValue(paParams));

		string desc = root["Description"].asString();
		DiagStructure(root["Status"].asBool(), &desc, &root["Data"], pvarRetValue, m_iMemory);

	}
	return true;

	case eMethSqlLiteGetOpenedConnectionsList:
	{
		Json::Value root = mSQLt.GetConnections();

		string desc = root["Description"].asString();
		DiagStructure(root["Status"].asBool(), &desc, &root["Data"], pvarRetValue, m_iMemory);
	}
	return true;

	case eMethSqlLiteGetDbDetails:
	{
		if (!isNumericParameter(paParams))
		{
			DiagStructure(false, L"Параметр должен быть числом!", L"", pvarRetValue, m_iMemory);
			return true;
		}

		Json::Value root = mSQLt.GetDbDetails(numericValue(paParams));

		string desc = root["Description"].asString();
		DiagStructure(root["Status"].asBool(), &desc, &root["Data"], pvarRetValue, m_iMemory);
	}
	return true;

	case eMethSqlLiteExecuteStatement:
	{
		if (lSizeArray < 2)
		{
			DiagStructure(false, L"Параметра должно быть 2.", L"", pvarRetValue, m_iMemory);
			return true;
		}

		if (!isNumericParameter(&paParams[0]))
		{
			DiagStructure(false, L"Первый параметр (connection ID) должен быть числом.", L"", pvarRetValue, m_iMemory);
			return true;
		}
		if ((&paParams[1])->vt != VTYPE_PWSTR)
		{
			DiagStructure(false, L"Второй параметр (скрипт запроса к БД) должен быть строкой.", L"", pvarRetValue, m_iMemory);
			return true;
		}

		wchar_t* statement = nullptr;
		convFromShortWchar(&statement, (&paParams[1])->pwstrVal);

		wstring ws_statement = wstring(statement);
		Json::Value root = mSQLt.Exec(numericValue(&paParams[0]), &ws_statement);

		delete statement;

		string desc = root["Description"].asCString();
		DiagStructure(root["Status"].asBool(), &desc, &root["Data"], pvarRetValue, m_iMemory);

	}
	return true;

	case eMethSqlLiteExecuteStatement_v2:
	{
		if (lSizeArray < 2)
		{
			DiagStructure(false, L"Параметра должно быть 2.", L"", pvarRetValue, m_iMemory);
			return true;
		}

		if ((&paParams[0])->vt != VTYPE_PWSTR)
		{
			DiagStructure(false, L"Первый параметр (путь к файлу БД) должен быть строкой.", L"", pvarRetValue, m_iMemory);
			return true;
		}
		if ((&paParams[1])->vt != VTYPE_PWSTR)
		{
			DiagStructure(false, L"Второй параметр (скрипт запроса к БД) должен быть строкой.", L"", pvarRetValue, m_iMemory);
			return true;
		}

		wchar_t* dbname = nullptr;
		wchar_t* statement = nullptr;

		convFromShortWchar(&dbname, (&paParams[0])->pwstrVal);
		convFromShortWchar(&statement, (&paParams[1])->pwstrVal);

		wstring ws_database = wstring(dbname);
		wstring ws_statement = wstring(statement);
		Json::Value root = mSQLt.Exec_v2(&ws_database, &ws_statement);

		delete dbname;
		delete statement;

		string desc = root["Description"].asCString();
		DiagStructure(root["Status"].asBool(), &desc, &root["Data"], pvarRetValue, m_iMemory);

	}
	return true;

	case eMethSqlLiteInsertBlobData:
	{
		if (lSizeArray < 3)
		{
			DiagStructure(false, L"Параметра должно быть 3 (ИД соединения, скрипт запроса, двоичные данные).", L"", pvarRetValue, m_iMemory);
			return true;
		}

		if (!isNumericParameter(&paParams[0]))
		{
			DiagStructure(false, L"Первый параметр (connection ID) должен быть числом.", L"", pvarRetValue, m_iMemory);
			return true;
		}

		if ((&paParams[1])->vt != VTYPE_PWSTR)
		{
			DiagStructure(false, L"Второй параметр (скрипт запроса к БД) должен быть строкой.", L"", pvarRetValue, m_iMemory);
			return true;
		}
		if ((&paParams[2])->vt != VTYPE_BLOB)
		{
			DiagStructure(false, L"Третий параметр должен быть двоичными данными.", L"", pvarRetValue, m_iMemory);
			return true;
		}


		wchar_t* statement = nullptr;
		convFromShortWchar(&statement, (&paParams[1])->pwstrVal);
		wstring ws_statement = wstring(statement);

		char* blob_data_from_platform = new char[(&paParams[2])->strLen];
		memset(blob_data_from_platform, 0, (&paParams[2])->strLen);
		memcpy(blob_data_from_platform, (&paParams[2])->pstrVal, (&paParams[2])->strLen);

		Json::Value root = mSQLt.InsertBlobData(numericValue(&paParams[0]), &ws_statement, blob_data_from_platform, (&paParams[2])->strLen);
		delete[] blob_data_from_platform;

		string desc = root["Description"].asCString();
		DiagStructure(root["Status"].asBool(), &desc, &root["Data"], pvarRetValue, m_iMemory);

	}
	return true;

	case eMethSqlLiteSelectBlobData:
	{
		if (lSizeArray < 2)
		{
			DiagStructure(false, L"Параметра должно быть 3 (ИД соединения, скрипт запроса, двоичные данные).", L"", pvarRetValue, m_iMemory);
			return true;
		}

		if (!isNumericParameter(&paParams[0]))
		{
			DiagStructure(false, L"Первый параметр (connection ID) должен быть числом.", L"", pvarRetValue, m_iMemory);
			return true;
		}

		if ((&paParams[1])->vt != VTYPE_PWSTR)
		{
			DiagStructure(false, L"Второй параметр (скрипт запроса к БД) должен быть строкой.", L"", pvarRetValue, m_iMemory);
			return true;
		}

		wchar_t* statement = nullptr;
		convFromShortWchar(&statement, (&paParams[1])->pwstrVal);
		wstring ws_statement = wstring(statement);

		Json::Value root = mSQLt.SelectBlobData(numericValue(&paParams[0]), &ws_statement);
		uint64_t pointer = root["Data"]["Pointer"].asUInt64();

		if (root["Status"] == false)
		{
			string desc = root["Description"].asCString();
			DiagStructure(root["Status"].asBool(), &desc, &root["Data"], pvarRetValue, m_iMemory);
		}
		else
		{

			if (pointer)
			{
				if (m_iMemory->AllocMemory((void**)&pvarRetValue->pstrVal, root["Data"]["Size"].asInt()))
				{
					pvarRetValue->strLen = root["Data"]["Size"].asInt();
					TV_VT(pvarRetValue) = VTYPE_BLOB;
					memcpy(pvarRetValue->pstrVal, reinterpret_cast<char*>(pointer), pvarRetValue->strLen);
				}
				else
				{
					stringstream ss;
					ss << "Не удалось выделить память под возващаемые данные (" << to_string(root["Data"]["Size"].asInt()) << ")";

					string s_loc = ss.str();

					DiagStructure(false, &s_loc, nullptr, pvarRetValue, m_iMemory);
				}
			}
			else
			{
				stringstream ss;
				ss << "Не удалось выделить память под возващаемые данные (" << to_string(root["Data"]["Size"].asInt()) << ")";

				string s_loc = ss.str();

				DiagStructure(false, &s_loc, nullptr, pvarRetValue, m_iMemory);
			}
		}

		if (pointer)
			delete[] reinterpret_cast<char*>(pointer);

	}
	return true;

	case eMethSmbListCatalog:
	{
		Samba smb(m_iMemory);
		smb.ListCatalog(paParams, pvarRetValue);

	}
	return true;

	case eMethSmbGetFile:
	{
		Samba smb(m_iMemory);
		smb.GetFileData(paParams, pvarRetValue);
	}
	return true;


	case eMethSmbPutFile:
	{
		Samba smb(m_iMemory);
		smb.PutFileData(paParams, pvarRetValue);
	}
	return true;

	case eMethRexMatch:
	{
		RegExFor1c rex(m_iMemory);
		rex.regexMatch(paParams, pvarRetValue);
	}
	return true;

	case eMethRexReplace:
	{
		RegExFor1c rex(m_iMemory);
		rex.regexReplace(paParams, pvarRetValue);
	}
	return true;

	case eMethGetQR:
	{
		if (!isNumericParameter(&paParams[1]))
			DiagToV8String(pvarRetValue, m_iMemory, false, L"Второй параметр должен быть целым числом, большим 50");

		if (numericValue(&paParams[1]) < 50)
			DiagToV8String(pvarRetValue, m_iMemory, false, L"Второй параметр должен быть целым числом, не меньшим 50");

		if ((&paParams[0])->wstrLen == 0)
			DiagToV8String(pvarRetValue, m_iMemory, false, L"Кодируемая строка не должна быть пустой");


		m_SendData.Initialize(m_iConnect, m_iMemory);
		m_SendData.GetQR(paParams, pvarRetValue);

	}
	return true;

	case eMethJvdValidate:
	{
		ValiJson jvd(m_iMemory);
		jvd.validateJsonByScheme(paParams, pvarRetValue);
	}
	return true;


	default:
		return false;
	}

}


/////////////////////////////////////////////////////////////////////////////
// ILocaleBase
//---------------------------------------------------------------------------//
void Hermes::SetLocale(const WCHAR_T* loc)
{
}

/////////////////////////////////////////////////////////////////////////////
// Other

//---------------------------------------------------------------------------//
void Hermes::addError(uint32_t wcode, const wchar_t* source, const wchar_t* descriptor, long code)
{
	if (m_iConnect)
	{
		WCHAR_T* err = 0;
		WCHAR_T* descr = 0;

		convToShortWchar(&err, source);
		convToShortWchar(&descr, descriptor);

		m_iConnect->AddError(wcode, err, descr, code);

		delete[] descr;
		delete[] err;
	}
}

//---------------------------------------------------------------------------//
long Hermes::findName(const wchar_t* names[], const wchar_t* name, const uint32_t size) const
{
	long ret = -1;
	for (uint32_t i = 0; i < size; i++)
	{
		if (!wcscmp(names[i], name))
		{
			ret = i;
			break;
		}
	}
	return ret;
}

bool Hermes::isNumericParameter(tVariant* par)
{
	return par->vt == VTYPE_I4 || par->vt == VTYPE_UI4 || par->vt == VTYPE_R8;
}

long Hermes::numericValue(tVariant* par)
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

void ToV8String(const wchar_t* wstr, tVariant* par, IMemoryManager* m_iMemory)
{
	if (wstr)
	{
		ULONG len = wcslen(wstr);
		m_iMemory->AllocMemory((void**)&par->pwstrVal, (len + 1) * sizeof(WCHAR_T));
		convToShortWchar(&par->pwstrVal, wstr);
		par->vt = VTYPE_PWSTR;
		par->wstrLen = len;
	}
	else
		par->vt = VTYPE_EMPTY;
}

void ToV8StringFromChar(const char* str, tVariant* par, IMemoryManager* m_iMemory)
{
	if (str)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		std::wstring wstr = converter.from_bytes(std::string(str));

		ULONG len = wcslen(wstr.c_str());
		m_iMemory->AllocMemory((void**)&par->pwstrVal, (len + 1) * sizeof(WCHAR_T));
		convToShortWchar(&par->pwstrVal, wstr.c_str());
		par->vt = VTYPE_PWSTR;
		par->wstrLen = len;
	}
	else
		par->vt = VTYPE_EMPTY;
}


wstring ToWStringJni(jstring jstr)
{
	wstring ret;
	if (jstr)
	{
		JNIEnv* env = getJniEnv();
		const jchar* jChars = env->GetStringChars(jstr, NULL);
		jsize jLen = env->GetStringLength(jstr);
		ret.assign(jChars, jChars + jLen);
		env->ReleaseStringChars(jstr, jChars);
	}
	return ret;
}

jstring ToJniString(wstring* in_std_wstring)
{
	JNIEnv* env = getJniEnv();
	WCHAR_T* WCHART = nullptr;
	convToShortWchar(&WCHART, in_std_wstring->c_str());
	int len = getLenShortWcharStr(WCHART);

	jstring retJString = env->NewString(WCHART, len);
	delete[] WCHART;

	return retJString;
}

WCHAR_T* ToV8StringJni(jstring jstr, ULONG* lSize, IMemoryManager* m_iMemory)		//Не работает - надо разбираться почему
{
	WCHAR_T* ret = NULL;
	if (jstr)
	{
		JNIEnv* jenv = getJniEnv();
		*lSize = jenv->GetStringLength(jstr);
		const WCHAR_T* pjstr = jenv->GetStringChars(jstr, NULL);
		m_iMemory->AllocMemory((void**)&ret, (*lSize + 1) * sizeof(WCHAR_T));
		for (auto i = 0; i < *lSize; ++i)
			ret[i] = pjstr[i];
		ret[*lSize] = 0;
		jenv->ReleaseStringChars(jstr, pjstr);
	}
	return ret;
}

int V8ToChar(tVariant *in_value, char** ch_out)
{

	wchar_t* _wchar_t = nullptr;
	convFromShortWchar(&_wchar_t, in_value->pwstrVal);
	int ret = 0;

	size_t size = (wcslen(_wchar_t)+1)*sizeof(wchar_t);
	try
	{
		*ch_out = new char[size];
		memset(*ch_out, 0, size);
	}
	catch (const std::exception&)
	{
		ret = 1;			//mem alloc fail
		*ch_out = nullptr;
	}
	
	if (*ch_out)
	{
		int res = wcstombs(*ch_out, _wchar_t, getLenShortWcharStr(TV_WSTR(in_value)));
		if (res == -1)
		{
			delete[] *ch_out;
			*ch_out = nullptr;
			ret = 2;			//wcstombs fail
		}
	}

	delete[] _wchar_t;

	return ret;
}

//Диагностика
//wstring DiagStructure(bool status, wstring ws_description, wstring ws_data)
//{
//
//	wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
//	string s_description = converter.to_bytes(ws_description);
//	string s_data = converter.to_bytes(ws_data);
//
//	Json::Value root;
//	root["status"] = status;
//	root["description"] = s_description;
//	root["data"] = s_data;
//
//	Json::StreamWriterBuilder builder;
//	string s = Json::writeString(builder, root);
//
//	return converter.from_bytes(s);
//
//}

bool DiagStructure(bool status, const char* ch_description, const char* ch_data, char** out_str)
{
	
	Json::Value root;
	root["Status"] = status;
	root["Description"] = ch_description;
	root["Data"] = ch_data;

	Json::StreamWriterBuilder builder;
	string s_res = Json::writeString(builder, root);

	if (!*out_str)
	{
		*out_str = new char[(s_res.length() + 1)];
	}
	else
	{
		return false;
	}

	strcpy(*out_str, s_res.c_str());
	return true;

}

bool DiagStructure(bool status, const wchar_t *wch_description, const wchar_t *wch_data, wchar_t **out_str)
{
	wstring ws_dwscription = wstring(wch_description);
	wstring ws_data = wstring(wch_data);

	wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
	string s_description = converter.to_bytes(ws_dwscription);
	string s_data = converter.to_bytes(ws_data);

	Json::Value root;
	root["Status"] = status;
	root["Description"] = s_description.c_str();
	root["Data"] = s_data.c_str();

	Json::StreamWriterBuilder builder;
	string s_res = Json::writeString(builder, root);
	wstring wstr = converter.from_bytes(s_res);

	if (!*out_str)
	{
		*out_str = new wchar_t[(wstr.length() + 1) * sizeof(wchar_t)];
	}
	else
	{
		return false;
	}

	wcscpy(*out_str, wstr.c_str());
	return true;
}

void DiagStructure(bool status, const wchar_t* wch_description, const wchar_t* wch_data, tVariant* pvarRetValue, IMemoryManager* m_iMemory)
{
	wstring ws_dwscription = wstring(wch_description);
	wstring ws_data = wstring(wch_data);

	wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
	string s_description = converter.to_bytes(ws_dwscription);
	string s_data = converter.to_bytes(ws_data);

	Json::Value root;
	root["Status"] = status;
	root["Description"] = s_description.c_str();
	root["Data"] = s_data.c_str();

	Json::StreamWriterBuilder builder;
	string s_res = Json::writeString(builder, root);
	wstring wstr = converter.from_bytes(s_res);

	ToV8String(wstr.c_str(), pvarRetValue, m_iMemory);

	return;
}

void DiagStructure(bool status, string* s_description, Json::Value* j_data, tVariant* pvarRetValue, IMemoryManager* m_iMemory)
{

	Json::Value root;
	root["Status"] = status;
	root["Description"] = *s_description;
	root["Data"] = *j_data;

	wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;

	Json::StreamWriterBuilder builder;
	string s_res = Json::writeString(builder, root);
	wstring wstr = converter.from_bytes(s_res);

	ToV8String(wstr.c_str(), pvarRetValue, m_iMemory);

	return;
}

void DiagToV8String(tVariant* pvarRetValue, IMemoryManager* m_iMemory, bool status, const wchar_t* wch_description)
{
	wchar_t* wch_err = nullptr;
	if (DiagStructure(status, wch_description, L"", &wch_err))
	{
		ToV8String(wch_err, pvarRetValue, m_iMemory);
		delete[] wch_err;
	}
	
}

void DiagToV8String(tVariant* pvarRetValue, IMemoryManager* m_iMemory, bool status, const char* ch_description)
{
	char* ch_err = nullptr;
	if (DiagStructure(status, ch_description, "", &ch_err))
	{
		ToV8StringFromChar(ch_err, pvarRetValue, m_iMemory);
		delete[] ch_err;
	}

}