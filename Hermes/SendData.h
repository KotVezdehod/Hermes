
#pragma once

#include "../include/AddInDefBase.h"
#include "../include/IAndroidComponentHelper.h"
#include "../jni/jnienv.h"
#include "../include/IMemoryManager.h"
#include <string>
#include <iostream>
#include <stdio.h>


class SendData
{
private:

	jclass cc;
	jobject obj;
	std::wstring current_b_cast_filter = L"";
	IMemoryManager* loc_iMemoryManager;
	IAndroidComponentHelper* helper;
	

public:
	
	const wchar_t* CATCHER_CLASS_NAME = L"ru.somecompany.dreamcatcher.catcher";

	SendData();
	~SendData();

	void Initialize(IAddInDefBaseEx* cnn, IMemoryManager* in_iMemoryManager);
	long numericValue(tVariant* par);

	void Start(std::wstring* std_wstr_filter, IAddInDefBaseEx* cnn) const; // Start monitoring lock state
	void Stop() const; // End of monitoring
	void GetCurrentFilter(std::wstring *Dest);
	void SetCurrentFilter(std::wstring *str_filter);
	void GetLastBroadcastExtra(std::wstring* std_wstr_extra);
	void GetID(tVariant* pvarRetValue);
	void GetAndroidID(tVariant* pvarRetValue);
	void GetOSAbis(tVariant *pvarRetValue);
	void StartHTTP(tVariant* pvarRetValue, int PortNumber);
	void StopHTTP(tVariant* pvarRetValue);
	void StopHTTP();
	void ReplyHTTP(tVariant* paParams);
	/*void MakeAPhoto(int xSize, int ySize, int Quality, tVariant* pvarRetValue);
	void RequestPhotoPermissions(tVariant* pvarRetValue);*/
	void RefactorImage(wchar_t *fn_in, wchar_t* fn_out, int xSize, int ySize, int Quality, tVariant* pvarRetValue);
	void DecodeBarcode(tVariant* paParams, tVariant* pvarRetValue);
	void StartGeolocation(tVariant* paParams, tVariant* pvarRetValue);
	void StopGeolocation(tVariant* pvarRetValue);
	void GetLocationNow(tVariant* paParams, tVariant* pvarRetValue);
	void GetQR(tVariant* paParams, tVariant* pvarRetValue);
};