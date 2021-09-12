
#include "pch.h"
#include "AddInNative.h"
#include "ConversionWchar.h"
#include "wchar.h"
#include <chrono>
#include <thread>
#include <string>
#include <iostream>
#include "../jni/jnienv.h"
#include "../include/IAndroidComponentHelper.h"

using std::string;
using std::wstring;



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
	/*case eMethTakeAPhoto:
		return 3;*/
	case eMethPHOTORefactorImage:
		return 5;
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
		return true;

	case eMethStartBroadcastsInterception:
		return true;
	case eMethStopBroadcastsInterception:
		return true;
	case eMethDeviceID:
		return true;
	case eMethAndroidID:
		return true;
	case eMethOSAbis:
		return true;
	case eMethStartHTTP:
		return true;
	case eMethStopHTTP:
		return true;
	case eMethZLIBCompress:
		return true;
	case eMethZLIBDecompress:
		return true;
	case eMethZLIBCompressFile:
		return true;
	case eMethZLIBDecompressFile:
		return true;
	/*case eMethTakeAPhoto:
		return true;
	case eMethRequestPhotoPermissions:
		return true;*/
	case eMethPHOTORefactorImage:
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
												ToV8String(L"Не верный тип параматра 2 (должнf быть строка)!", pvarRetValue, m_iMemory);
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