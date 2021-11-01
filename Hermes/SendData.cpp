
#include <wchar.h>
#include "SendData.h"
#include "ConversionWchar.h"
#include <string>
#include <iostream>
#include "AddInNative.h"


using std::string;
using std::wstring;

wstring glob_last_broadcast_extra = L"";

#pragma region common

SendData::SendData() : cc(nullptr), obj(nullptr), loc_iMemoryManager(nullptr)
{

}

SendData::~SendData()
{
	loc_iMemoryManager = nullptr;
	helper = nullptr;
	if (obj)
	{
		Stop(); // call to unregister broadcast receiver
		StopHTTP();
		JNIEnv* env = getJniEnv();
		env->DeleteGlobalRef(obj);
		env->DeleteGlobalRef(cc);

	}
}

void SendData::Initialize(IAddInDefBaseEx* cnn, IMemoryManager *in_iMemoryManager)
{
	loc_iMemoryManager = in_iMemoryManager;

	if (!obj)
	{
		helper = (IAndroidComponentHelper*)cnn->GetInterface(eIAndroidComponentHelper);
		if (helper)
		{
			WCHAR_T* className = nullptr;

			convToShortWchar(&className, CATCHER_CLASS_NAME);

			jclass ccloc = helper->FindClass(className);

			delete[] className;

			className = nullptr;

			if (ccloc)
			{

				JNIEnv* env = getJniEnv();

				cc = static_cast<jclass>(env->NewGlobalRef(ccloc));

				env->DeleteLocalRef(ccloc);

				jobject activity = helper->GetActivity();

				// call of constructor for java class
				jmethodID methID = env->GetMethodID(cc, "<init>", "(Landroid/app/Activity;J)V");
				//jmethodID methID = env->GetMethodID(cc, "<init>", "(Landroidx.core.app.ActivityCompat;J)V");

				jobject objloc = env->NewObject(cc, methID, activity, (jlong)cnn);

				obj = static_cast<jobject>(env->NewGlobalRef(objloc));

				env->DeleteLocalRef(objloc);

				methID = env->GetMethodID(cc, "show", "()V");

				env->CallVoidMethod(obj, methID);

				env->DeleteLocalRef(activity);
			}
		}
	}
}

#pragma endregion

#pragma region broadcast

void SendData::Start(wstring* std_wstr_filter, IAddInDefBaseEx* cnn) const
{
	if (obj)
	{
		JNIEnv* env = getJniEnv();
		jmethodID methID = env->GetMethodID(cc, "start", "(Ljava/lang/String;)V");
		//jmethodID methID = env->GetMethodID(cc, "start", "()V");
		jstring j_string = ToJniString(std_wstr_filter);

		env->CallVoidMethod(obj, methID, j_string);
		env->DeleteLocalRef(j_string);

	}
}

void SendData::Stop() const
{
	if (obj)
	{
		JNIEnv* env = getJniEnv();
		jmethodID methID = env->GetMethodID(cc, "stop", "()V");
		env->CallVoidMethod(obj, methID);
	}
}

void SendData::GetCurrentFilter(wstring* Dest)
{
	Dest->clear();
	*Dest += current_b_cast_filter;
}

void SendData::SetCurrentFilter(wstring* str_filter)
{
	current_b_cast_filter.clear();
	current_b_cast_filter += *str_filter;
	return;
}

void SendData::GetLastBroadcastExtra(wstring* std_wstr_extra)
{
	std_wstr_extra->clear();
	std_wstr_extra->assign(glob_last_broadcast_extra);

	return;
}

#pragma endregion

#pragma region getid
void SendData::GetID(tVariant* pvarRetValue)
{
	if (obj)
	{
		JNIEnv* env = getJniEnv();
		jmethodID methID = env->GetMethodID(cc, "GetID", "()Ljava/lang/String;");
		jstring stringObject = static_cast<jstring>(env->CallObjectMethod(obj, methID));
		wstring wstr = ToWStringJni(stringObject);
		env->DeleteLocalRef(stringObject);

		//Исторически так сложилось, что для ид устройства используется ГУИД. Традиции - наше все. Но дроид нам возвращает короткую строку формата "cc33bf353f67e3eb".
		//Приведем ее к формату ГУИДа.

		wstring wstr1 = L"";

		if (wstr.length() < 17)
		{
			wstr1.assign(17 - wstr.length(), '0');
			wstr1 = wstr + wstr1;
		}
		else
		{
			wstr1 = wstr.substr(0, 17);
		}

		wstr.clear();
		wstr = wstr1.substr(0, 8) + L"-" + wstr1.substr(8, 4) + L"-" + wstr1.substr(12, 4) + L"-" + wstr1.substr(0, 4) + L"-" + wstr1.substr(0, 12);
		ToV8String(wstr.c_str(), pvarRetValue, loc_iMemoryManager);
	}
	else
	{
		
		ToV8String(L"JNI Error", pvarRetValue, loc_iMemoryManager);
	}

	return;
}

void SendData::GetAndroidID(tVariant* pvarRetValue)
{
	
	if (obj)
	{
		JNIEnv* env = getJniEnv();
		jmethodID methID = env->GetMethodID(cc, "GetID", "()Ljava/lang/String;");
		jstring stringObject = static_cast<jstring>(env->CallObjectMethod(obj, methID));
		wstring wstr = ToWStringJni(stringObject);
		env->DeleteLocalRef(stringObject);
		ToV8String(wstr.c_str(), pvarRetValue, loc_iMemoryManager);
	}
	else
	{

		ToV8String(L"JNI Error", pvarRetValue, loc_iMemoryManager);
	}
	return;
}

#pragma endregion

#pragma region getosabis
void SendData::GetOSAbis(tVariant* pvarRetValue)
{
	if (obj)
	{
		JNIEnv* env = getJniEnv();
		jmethodID methID = env->GetMethodID(cc, "GetOSAbis", "()Ljava/lang/String;");
		jstring stringObject = static_cast<jstring>(env->CallObjectMethod(obj, methID));
		wstring wstr = ToWStringJni(stringObject);
		env->DeleteLocalRef(stringObject);
		ToV8String(wstr.c_str(), pvarRetValue, loc_iMemoryManager);
	}
	else
	{
		ToV8String(L"JNI Error", pvarRetValue, loc_iMemoryManager);
	}
	
	return;
}
#pragma endregion

#pragma region http
void SendData::StartHTTP(tVariant* pvarRetValue, int PortNumber)
{
	if (obj)
	{
		JNIEnv* env = getJniEnv();
		jmethodID methID = env->GetMethodID(cc, "StartHTTP", "(I)Ljava/lang/String;");
		jstring stringObject = static_cast<jstring>(env->CallObjectMethod(obj, methID, PortNumber));
		wstring std_wstr = ToWStringJni(stringObject);
		env->DeleteLocalRef(stringObject);
		ToV8String(std_wstr.c_str(), pvarRetValue, loc_iMemoryManager);
	}
	else
	{
		ToV8String(L"JNI Error", pvarRetValue, loc_iMemoryManager);
	}
	return;
}

void SendData::StopHTTP(tVariant* pvarRetValue)
{
	if (obj)
	{
		JNIEnv* env = getJniEnv();
		jmethodID methID = env->GetMethodID(cc, "StopHTTP", "()Ljava/lang/String;");
		jstring stringObject = static_cast<jstring>(env->CallObjectMethod(obj, methID));
		wstring std_wstr = ToWStringJni(stringObject);
		env->DeleteLocalRef(stringObject);
		ToV8String(std_wstr.c_str(), pvarRetValue, loc_iMemoryManager);
	}
	
	return;
}

void SendData::StopHTTP()
{
	if (obj)
	{
		JNIEnv* env = getJniEnv();
		jmethodID methID = env->GetMethodID(cc, "StopHTTP", "()Ljava/lang/String;");
		jstring stringObject = static_cast<jstring>(env->CallObjectMethod(obj, methID));
		env->DeleteLocalRef(stringObject);
	}
	
	return;
}

void SendData::ReplyHTTP(tVariant* paParams)
{
	if (obj)
	{
		wstring std_wstr(paParams->pwstrVal, paParams->pwstrVal + paParams->strLen);
		jstring j_string = ToJniString(&std_wstr);

		JNIEnv* env = getJniEnv();
		jmethodID methID = env->GetMethodID(cc, "handleAnswerFrom1c", "(Ljava/lang/String;)V");
		env->CallVoidMethod(obj, methID, j_string);
		env->DeleteLocalRef(j_string);

	}
	
	return;
}

#pragma endregion

#pragma region Photo

void SendData::RefactorImage(wchar_t* fn_in, wchar_t* fn_out, int xSize, int ySize, int Quality, tVariant* pvarRetValue)
{

	if (obj)
	{
		wstring wstr_fn_in = std::wstring(fn_in);
		wstring wstr_fn_out = std::wstring(fn_out);

		jstring jstr_fn_in = ToJniString(&wstr_fn_in);
		jstring jstr_fn_out = ToJniString(&wstr_fn_out);

		JNIEnv* env = getJniEnv();
		jmethodID methID = env->GetMethodID(cc, "RefactorImage", "(Ljava/lang/String;Ljava/lang/String;III)Ljava/lang/String;");
		jstring stringObject = static_cast<jstring>(env->CallObjectMethod(obj, methID, jstr_fn_in, jstr_fn_out, xSize, ySize, Quality));
		wstring std_wstr = ToWStringJni(stringObject);
		env->DeleteLocalRef(stringObject);
		env->DeleteLocalRef(jstr_fn_in);
		env->DeleteLocalRef(jstr_fn_out);

		ToV8String(std_wstr.c_str(), pvarRetValue, loc_iMemoryManager);
	}
	else
	{
		ToV8String(L"JNI Error", pvarRetValue, loc_iMemoryManager);
	}
	return;
}

#pragma endregion

#pragma region Barcode
void SendData::DecodeBarcode(tVariant* paParams, tVariant* pvarRetValue)
{
	if (obj)
	{
		JNIEnv* env = getJniEnv();

		if (TV_VT(paParams) == VTYPE_PWSTR)
		{
			wchar_t* wch_fn_in = nullptr;
			convFromShortWchar(&wch_fn_in, paParams->pwstrVal);
			wstring wstr_fn_in = std::wstring(wch_fn_in);
			delete[] wch_fn_in;

			jstring js_fn_in = ToJniString(&wstr_fn_in);

			jmethodID methID = env->GetMethodID(cc, "DecodeBarcode", "(Ljava/lang/String;[B)Ljava/lang/String;");

			jstring stringObject = static_cast<jstring>(env->CallObjectMethod(obj, methID, js_fn_in, nullptr));
			wstring std_wstr = ToWStringJni(stringObject);

			ToV8String(std_wstr.c_str(), pvarRetValue, loc_iMemoryManager);

			env->DeleteLocalRef(stringObject);

			env->DeleteLocalRef(js_fn_in);
		}
		else   //BLOB
		{
			jbyteArray jb = env->NewByteArray(paParams->strLen);
			env->SetByteArrayRegion(jb, 0, paParams->strLen, (jbyte*)(paParams->pstrVal));
			
			jmethodID methID = env->GetMethodID(cc, "DecodeBarcode", "(Ljava/lang/String;[B)Ljava/lang/String;");

			jstring stringObject = static_cast<jstring>(env->CallObjectMethod(obj, methID, NULL, jb));
			wstring std_wstr = ToWStringJni(stringObject);

			ToV8String(std_wstr.c_str(), pvarRetValue, loc_iMemoryManager);

			env->DeleteLocalRef(jb);
			env->DeleteLocalRef(stringObject);
		}

	}
	else
	{
		DiagToV8String(pvarRetValue, loc_iMemoryManager, false, L"JNI Error");
	}
	
	return;
}
#pragma endregion


static const wchar_t g_EventSource[] = L"Hermes";
static const wchar_t g_EventName[] = L"BroadcastCatched";
static WcharWrapper s_EventSource(g_EventSource);
static WcharWrapper s_EventName(g_EventName);

//https://www3.ntu.edu.sg/home/ehchua/programming/java/javanativeinterface.html
// name of function built according to Java native call
//
extern "C" JNIEXPORT void JNICALL Java_ru_coolclever_dreamcatcher_catcher_OnBroadcastCatched(JNIEnv * env, jclass jClass, jlong pObject, jstring inJNIStr)
{
	wstring std_wstring = ToWStringJni(inJNIStr);
	glob_last_broadcast_extra.clear();
	glob_last_broadcast_extra.assign(std_wstring);
	WCHAR_T* WCHART = nullptr;
	convToShortWchar(&WCHART, std_wstring.c_str());

	IAddInDefBaseEx* pAddIn = (IAddInDefBaseEx*)pObject;
	pAddIn->ExternalEvent(s_EventSource, s_EventName, WCHART);

	delete[] WCHART;

}

static const wchar_t g_EventName_http[] = L"http_request";
static WcharWrapper s_EventName_http(g_EventName_http);

extern "C" JNIEXPORT void JNICALL Java_ru_coolclever_dreamcatcher_catcher_OnHttpServerServ(JNIEnv * env, jclass jClass, jlong pObject, jstring inReq)
{
	wstring std_wstring = ToWStringJni(inReq);
	WCHAR_T* WCHART = nullptr;
	convToShortWchar(&WCHART, std_wstring.c_str());

	IAddInDefBaseEx* pAddIn = (IAddInDefBaseEx*)pObject;
	pAddIn->ExternalEvent(s_EventSource, s_EventName_http, WCHART);

	delete[] WCHART;

}
