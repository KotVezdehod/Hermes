
#ifndef __ADDINNATIVE_H__
#define __ADDINNATIVE_H__

#include <string>
#include "../include/ComponentBase.h"
#include "../include/AddInDefBase.h"
#include "../include/IMemoryManager.h"
#include "SendData.h"
#include "Compressor.h"
#include "FileWorks.h"

void ToV8String(const wchar_t* wstr, tVariant*, IMemoryManager* m_iMemory);
void ToV8StringFromChar(const char* wstr, tVariant*, IMemoryManager* m_iMemory);
std::wstring ToWStringJni(jstring jstr);
jstring ToJniString(std::wstring* in_std_wstring);
WCHAR_T* ToV8StringJni(jstring jstr, ULONG* lSize, IMemoryManager* m_iMemory);		//Не работает - надо разбираться почему
int V8ToChar(tVariant* in_value, char** ch_out);
//std::wstring DiagStructure(bool status, std::wstring ws_description, std::wstring ws_data);
bool DiagStructure(bool status, const wchar_t* wch_description, const wchar_t* wch_data, wchar_t** out_str);
void DiagToV8String(tVariant* pvarRetValue, IMemoryManager* m_iMemory, bool status, const wchar_t* wch_description);


static const wchar_t* g_PropNames[] =
{
	L"DeviceInfo",
	L"CurrentBroadcastsFilters_r",
	L"LastBroadcastExtra_r"
};

static const wchar_t* g_PropNamesRu[] =
{
	L"ОписаниеУстройства",
	L"ТекущиеБроадкастФильтры_ч",
	L"ПследниеДанныеИзПерехватчикаБроадкаста_ч"
};

static const wchar_t* g_MethodNames[] =
{
	L"Delay",
	L"Test",
	L"StartBroadcastsInterception",
	L"StopBroadcastsInterception",
	L"DeviceID",
	L"AndroidID",
	L"OSAbis",
	L"HTTP_Start",
	L"HTTP_Stop",
	L"HTTP_Reply",
	L"ZLIB_Compress",
	L"ZLIB_Decompress",
	L"ZLIB_CompressFile",
	L"ZLIB_DecompressFile",
	L"PHOTO_RefactorImage",
	L"Version",
	L"ZXING_DecodeBarcode",
	L"FS_ScanFolder",
	L"FS_IsDirectory",
	L"FS_DeleteFileOrDirectory",
	L"FS_CreateFile",
	L"FS_CreateDirectory",
	L"FS_RenameFileOrDirectory",
	L"FS_WriteDataToFile",
	L"FS_FSOPresent",
	L"FS_ReadDataFromFile",
	L"GEO_StartListening",
	L"GEO_StopListening"
	L"GEO_GetNow"


};

static const wchar_t* g_MethodNamesRu[] =
{
	L"Пауза",
	L"Тест",
	L"ЗапуститьПерехватБроадкастов",
	L"ОстановитьПерехватБроадкастов",
	L"ИДУстройства",
	L"АндроидИД",
	L"Архитектура",
	L"HTTPСервер_Запустить",
	L"HTTPСервер_Остановить",
	L"HTTPСервер_Ответить",
	L"ZLIB_Архивировать",
	L"ZLIB_Разархивировать",
	L"ZLIB_АрхивироватьФайл",
	L"ZLIB_РазархивироватьФайл",
	L"PHOTO_ИзменитьКартинку",
	L"Версия",
	L"ZXING_РаспознатьШтрихкод",
	L"ФС_ПрочитатьКаталог",
	L"ФС_ЭтоДиректория",
	L"ФС_УдалитьФайлИлиКаталог",
	L"ФС_СоздатьФайл",
	L"ФС_СоздатьКаталог",
	L"ФС_ПереименоватьФайлИлиКаталог",
	L"ФС_ЗаписатьДанныеВФайл",
	L"ФС_ОбъектФССуществует",
	L"ФС_ПрочитатьДанныеИзФайла",
	L"ГЕО_НачатьПолучениеКоординат",
	L"ГЕО_ОстановитьПолучениеКоординат",
	L"ГЕО_ПолучитьСейчас"

};

class Hermes : public IComponentBase
{
	

public:
	enum Props
	{
		ePropDevice = 0,
		ePropCurrentBroadcastsFilters,
		ePropLastBroadcastExtra,
		ePropLast         // Always last
	};

	enum Methods
	{
		eMethDelay = 0,
		eMethTest,
		eMethStartBroadcastsInterception,
		eMethStopBroadcastsInterception,
		eMethDeviceID,
		eMethAndroidID,
		eMethOSAbis,
		eMethStartHTTP,
		eMethStopHTTP,
		eMethReplyHTTP,
		eMethZLIBCompress,
		eMethZLIBDecompress,
		eMethZLIBCompressFile,
		eMethZLIBDecompressFile,
		eMethPHOTORefactorImage,
		eMethVersion,
		eMethDecodeBarcode,
		eMethScanFolder,
		eMethIsDirectory,
		eMethDeleteFileOrDirectory,
		eMethCreateFile,
		eMethCreateDirectory,
		eMethRenameFileOrDirectory,
		eMethWriteDataToFile,
		eMethFSOPresent,
		eMethReadDataFromFile,
		eMethGEOStartListening,
		eMethGEOStopListening,
		eMethGEOGetNow,
		eMethLast       // Always last
	};

	Hermes(void);
	virtual ~Hermes();
	// IInitDoneBase
	virtual bool ADDIN_API Init(void*);
	virtual bool ADDIN_API setMemManager(void* mem);
	virtual long ADDIN_API GetInfo();
	virtual void ADDIN_API Done();
	// ILanguageExtenderBase
	virtual bool ADDIN_API RegisterExtensionAs(WCHAR_T**);
	virtual long ADDIN_API GetNProps();
	virtual long ADDIN_API FindProp(const WCHAR_T* wsPropName);
	virtual const WCHAR_T* ADDIN_API GetPropName(long lPropNum, long lPropAlias);
	virtual bool ADDIN_API GetPropVal(const long lPropNum, tVariant* pvarPropVal);
	virtual bool ADDIN_API SetPropVal(const long lPropNum, tVariant* varPropVal);
	virtual bool ADDIN_API IsPropReadable(const long lPropNum);
	virtual bool ADDIN_API IsPropWritable(const long lPropNum);
	virtual long ADDIN_API GetNMethods();
	virtual long ADDIN_API FindMethod(const WCHAR_T* wsMethodName);
	virtual const WCHAR_T* ADDIN_API GetMethodName(const long lMethodNum,
		const long lMethodAlias);
	virtual long ADDIN_API GetNParams(const long lMethodNum);
	virtual bool ADDIN_API GetParamDefValue(const long lMethodNum, const long lParamNum,
		tVariant* pvarParamDefValue);
	virtual bool ADDIN_API HasRetVal(const long lMethodNum);
	virtual bool ADDIN_API CallAsProc(const long lMethodNum,
		tVariant* paParams, const long lSizeArray);
	virtual bool ADDIN_API CallAsFunc(const long lMethodNum,
		tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray);
	// ILocaleBase
	virtual void ADDIN_API SetLocale(const WCHAR_T* loc);
	


private:
	long findName(const wchar_t* names[], const wchar_t* name, const uint32_t size) const;
	void addError(uint32_t wcode, const wchar_t* source,
		const wchar_t* descriptor, long code);

	bool isNumericParameter(tVariant*);
	long numericValue(tVariant*);

	IAddInDefBaseEx* m_iConnect;
	IMemoryManager* m_iMemory;

	bool isScreenOn; // current blocking state
	SendData m_SendData{};
	FileWorks *mFileWorks;
};

#endif
