#pragma once
#include <regex>
#include "AddInNative.h"

class RegExFor1c
{
	
public:

	RegExFor1c(IMemoryManager* iMemoryManagerIn) :
		iMemoryManager(iMemoryManagerIn) {};

	~RegExFor1c() {};

	void regexMatch(tVariant* par, tVariant* pvarRetValue);
	void regexSearch(tVariant* par, tVariant* pvarRetValue);
	void regexReplace(tVariant* par, tVariant* pvarRetValue);

private:

	IMemoryManager* iMemoryManager;

};

