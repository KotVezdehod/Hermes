#pragma once
#include "ConversionWchar.h"
#include "../include/IMemoryManager.h"

class FileWorks
{

public:
	FileWorks(IMemoryManager* mIMemoryManager_in)
	{
		mIMemoryManager = mIMemoryManager_in;
	};
	~FileWorks() {};

	void ScanFolder(tVariant *pvarRetValue, tVariant* paParams);
	void IsDirectory(tVariant* pvarRetValue, tVariant* paParams);
	void DeleteFileOrDirectory(tVariant* pvarRetValue, tVariant* paParams);
	void CreateFile(tVariant* pvarRetValue, tVariant* paParams);
	void CreateDirectory(tVariant* pvarRetValue, tVariant* paParams);
	void RenameFileOrDirectory(tVariant* pvarRetValue, tVariant* paParams);
	void WriteDataToFile(tVariant* pvarRetValue, tVariant* paParams);
	void ReadDataFromFile(tVariant* pvarRetValue, tVariant* paParams);
	void IsFSOPresent(tVariant* pvarRetValue, tVariant* paParams);
	

private:
	IMemoryManager *mIMemoryManager;
	long numericValue(tVariant* par);
};

