#pragma once

#include "AddInNative.h"

class Samba
{

public:

	Samba(IMemoryManager* iMemoryManagerIn):
		iMemoryManager(iMemoryManagerIn){};

	~Samba() {};
	
	void ListCatalog(tVariant* par, tVariant* pvarRetValue);
	void GetFileData(tVariant* par, tVariant* pvarRetValue);

private:
	IMemoryManager* iMemoryManager;
	bool isNumericParameter(tVariant* par);
	long numericValue(tVariant* par);
};

