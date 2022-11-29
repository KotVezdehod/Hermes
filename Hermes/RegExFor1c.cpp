#include "RegExFor1c.h"
#include "Json.h"
//https://en.cppreference.com/w/cpp/regex


void RegExFor1c::regexMatch(tVariant* paParams, tVariant* pvarRetValue)
{
	if (TV_VT(&paParams[0]) != VTYPE_PWSTR)
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"Первый параметр - строка, в которой будет производиться поиск совпадений.");
		return;
	}

	if (TV_VT(&paParams[1]) != VTYPE_PWSTR)
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"Второй параметр - регулярное выражение.");
		return;
	}

	wchar_t* wchTxt = nullptr;
	convFromShortWchar(&wchTxt, (&paParams[0])->pwstrVal);

	if (!wchTxt)
	{
		wstringstream wssLoc;
		wssLoc << L"Ошибка работы convFromShortWchar: вероятно проблемы с выделением памяти под первый параметр.";
		DiagToV8String(pvarRetValue, iMemoryManager, false, wssLoc.str().c_str());
		return;
	}

	size_t wchTxtSz = (wcslen(wchTxt) + 1) * sizeof(wchar_t);
	char* chTxt = new char[wchTxtSz];
	memset(chTxt,0, wchTxtSz);
	wcstombs(chTxt, wchTxt, wchTxtSz);
	delete[] wchTxt;
	std::string strTxt(chTxt);
	delete[] chTxt;


	wchar_t* wchRegExpr = nullptr;
	convFromShortWchar(&wchRegExpr, (&paParams[1])->pwstrVal);

	if (!wchRegExpr)
	{
		wstringstream wssLoc;
		wssLoc << L"Ошибка работы convFromShortWchar: веро¤тно проблемы с выделением памяти под второй параметр.";
		DiagToV8String(pvarRetValue, iMemoryManager, false, wssLoc.str().c_str());
		return;
	}

	size_t wchRegExpSz = (wcslen(wchRegExpr) + 1)* sizeof(wchar_t);
	char* chRegExp = new char[wchRegExpSz];
	memset(chRegExp,0, wchRegExpSz);
	wcstombs(chRegExp, wchRegExpr, wchRegExpSz);
	delete[] wchRegExpr;

	std::regex regExpression(std::string(chRegExp), std::regex_constants::ECMAScript);
	delete[] chRegExp;

	std::smatch m;

	if (!std::regex_match(std::string(chTxt), m, regExpression))
	{
		wstringstream wssLoc;
		wssLoc << L"Вхождений не обнаружено.";
		DiagToV8String(pvarRetValue, iMemoryManager, false, wssLoc.str().c_str());
		return;
	};

	Json::Value root;
	Json::Value arr = Json::arrayValue;
	
	for (std::smatch::iterator it = m.begin(); it != m.end(); std::advance(it,1))
	{
		arr.append((*it).str());
	}

	root["Status"]		= true;
	root["Description"] = "";
	root["Data"]		= arr;

	Json::FastWriter fw;
	string s_res = fw.write(root);
	ToV8StringFromChar(s_res.c_str(), pvarRetValue, iMemoryManager);

	return;
}

void RegExFor1c::regexReplace(tVariant* paParams, tVariant* pvarRetValue)
{
	if (TV_VT(&paParams[0]) != VTYPE_PWSTR)
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"Первый параметр - строка, в которой будет производиться замена.");
		return;
	}

	if (TV_VT(&paParams[1]) != VTYPE_PWSTR)
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"Второй параметр - регулярное выражение.");
		return;
	}

	if (TV_VT(&paParams[2]) != VTYPE_PWSTR)
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"Третий параметр - паттерн для замены.");
		return;
	}


	wchar_t* wchTxt = nullptr;
	convFromShortWchar(&wchTxt, (&paParams[0])->pwstrVal);

	if (!wchTxt)
	{
		wstringstream wssLoc;
		wssLoc << L"Ошибка работы convFromShortWchar: вероятно проблемы с выделением памяти под первый параметр.";
		DiagToV8String(pvarRetValue, iMemoryManager, false, wssLoc.str().c_str());
		return;
	}

	size_t wchTxtSz = (wcslen(wchTxt) + 1) * sizeof(wchar_t);
	char* chTxt = new char[wchTxtSz];
	memset(chTxt, 0, wchTxtSz);
	wcstombs(chTxt, wchTxt, wchTxtSz);
	delete[] wchTxt;
	std::string strTxt(chTxt);
	delete[] chTxt;


	wchar_t* wchRegExpr = nullptr;
	convFromShortWchar(&wchRegExpr, (&paParams[1])->pwstrVal);

	if (!wchRegExpr)
	{
		wstringstream wssLoc;
		wssLoc << L"Ошибка работы convFromShortWchar: вероятно проблемы с выделением памяти под второй параметр.";
		DiagToV8String(pvarRetValue, iMemoryManager, false, wssLoc.str().c_str());
		return;
	}

	size_t wchRegExpSz = (wcslen(wchRegExpr) + 1) * sizeof(wchar_t);
	char* chRegExp = new char[wchRegExpSz];
	memset(chRegExp, 0, wchRegExpSz);
	wcstombs(chRegExp, wchRegExpr, wchRegExpSz);
	delete[] wchRegExpr;
	std::regex regExpression(std::string(chRegExp), std::regex_constants::ECMAScript);
	delete[] chRegExp;


	wchar_t* wchPattern = nullptr;
	convFromShortWchar(&wchPattern, (&paParams[2])->pwstrVal);
	if (!wchPattern)
	{
		wstringstream wssLoc;
		wssLoc << L"Ошибка работы convFromShortWchar: вероятно проблемы с выделением памяти под третий параметр.";
		DiagToV8String(pvarRetValue, iMemoryManager, false, wssLoc.str().c_str());
		return;
	}

	size_t wchPatternSz = (wcslen(wchPattern) + 1) * sizeof(wchar_t);
	char* chPattern = new char[wchPatternSz];
	memset(chPattern, 0, wchPatternSz);
	wcstombs(chPattern, wchPattern, wchPatternSz);
	delete[] wchPattern;
	std::string strPattern(chPattern);
	delete[] chPattern;

	Json::Value root;

	std::string sOut = std::regex_replace(strTxt, regExpression, strPattern);

	root["Status"]		= true;
	root["Description"] = "";
	root["Data"]		= sOut.c_str();

	Json::FastWriter fw;
	string s_res = fw.write(root);
	ToV8StringFromChar(s_res.c_str(), pvarRetValue, iMemoryManager);
	
	return;
}

void RegExFor1c::regexSearch(tVariant* par, tVariant* pvarRetValue)
{

	return;
}