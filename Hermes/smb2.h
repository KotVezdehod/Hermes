#pragma once

#include "AddInNative.h"

class smb2
{

public:

	smb2(IMemoryManager* iMemoryManagerIn): 
		iMemoryManager(iMemoryManagerIn){};

	~smb2() {};
	
	void ListCatalog(tVariant* par, tVariant* pvarRetValue, const long lSizeArray);
	void ListCatalogIndirect(tVariant* par, tVariant* pvarRetValue, const long lSizeArray);
	
	void GetFileData(tVariant* par, tVariant* pvarRetValue, const long lSizeArray);
	void GetFileDataIndirect(tVariant* par, tVariant* pvarRetValue, const long lSizeArray);


private:
	IMemoryManager* iMemoryManager;
	bool smb2::isNumericParameter(tVariant* par);
	long smb2::numericValue(tVariant* par);
};

