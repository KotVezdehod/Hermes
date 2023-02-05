#pragma once

#include <set>

#include "../valijson/adapters/jsoncpp_adapter.hpp"
#include "../valijson/schema_parser.hpp"
#include "../valijson/validator.hpp"
#include "../valijson/validation_results.hpp"
#include "AddInNative.h"
#include "Json.h"

class ValiJson
{

public:
	
    void validateJsonByScheme(tVariant* paParams, tVariant* pvarRetValue)
    {
        if (TV_VT(&paParams[0]) != VTYPE_PWSTR)
        {
            DiagToV8String(pvarRetValue, iMemoryManager, false, L"Первый параметр - схема (строка)");
            return;
        }

        if (TV_VT(&paParams[1]) != VTYPE_PWSTR)
        {
            DiagToV8String(pvarRetValue, iMemoryManager, false, L"Второй параметр - сам json (строка)");
            return;
        }

        wchar_t* wchChema = nullptr;
        convFromShortWchar(&wchChema, (&paParams[0])->pwstrVal);

        if (wchChema)
        {
            wchar_t* wchData = nullptr;
            convFromShortWchar(&wchData, (&paParams[1])->pwstrVal);

            if (wchData)
            {
                size_t chemaSz = (wcslen(wchChema) + 1) * sizeof(wchar_t);
                char* chChema = nullptr;
                chChema = new char[chemaSz];

                if (chChema)
                {
                    memset(chChema, 0, chemaSz);
                    wcstombs(chChema, wchChema, chemaSz);
                    
                    size_t dataSz = (wcslen(wchData)+1)*sizeof(wchar_t);

                    char* chData = nullptr;
                    chData = new char[dataSz];

                    if (chData)
                    {
                        memset(chData, 0, dataSz);
                        wcstombs(chData, wchData, dataSz);

                        Json::Value root;

                        Json::Value details = Json::arrayValue;
                        details.clear();

                        root["Status"]      = validate(chChema, chemaSz, chData, dataSz, &details);
                        root["Description"] = details;
                        root["Data"]        = "";

                        Json::FastWriter fw;
                        string s_res = fw.write(root);
                        ToV8StringFromChar(s_res.c_str(), pvarRetValue, iMemoryManager);

                        delete[] chData;
                    }


                    delete[] chChema;
                }
                else
                {
                    DiagToV8String(pvarRetValue, iMemoryManager, false, L"не удалось выделить память под строку схемы.");
                }

                delete[] wchData;
            }
            else
            {
                DiagToV8String(pvarRetValue, iMemoryManager, false, L"не удалось выделить память под строку json.");
            }
            delete[] wchChema;
        }
        else
        {
            DiagToV8String(pvarRetValue, iMemoryManager, false, L"не удалось выделить память под строку схемы.");
        }

        return;
    };

    ValiJson(IMemoryManager* iMemoryManagerIn):
        iMemoryManager(iMemoryManagerIn){};

    ~ValiJson() {};


private:

    IMemoryManager* iMemoryManager = nullptr;

    bool validate(char* chemaIn, size_t chemaSz, char* dataIn, size_t dataSz, Json::Value* outDiag)
    {
        Json::Value rootSchema;
        Json::CharReaderBuilder builder;
        const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

        std::string err;

        
        if (reader->parse(chemaIn, chemaIn + chemaSz, &rootSchema, &err))
        {

            valijson::Schema schema;
            valijson::SchemaParser parser;
            valijson::adapters::JsonCppAdapter mySchemaAdapter(rootSchema);
            parser.populateSchema(mySchemaAdapter, schema);

            Json::Value rootData;

            if (reader->parse(dataIn, dataIn + dataSz, &rootData, &err))
            {

                valijson::Validator validator;
                valijson::adapters::JsonCppAdapter dataAdapter(rootData);
                valijson::ValidationResults res;

                if (!validator.validate(schema, dataAdapter, &res))
                {
                    buildFullErrorMessage(res, outDiag);
                    return false;
                }

                outDiag->append(std::string("OK"));
                return true;
            }
            else
            {
                outDiag->append(std::string("Ошибка при разборе JSON: "));
                outDiag->append(err);
                return false;
            }
        }
        else
        {
            outDiag->append(std::string("Ошибка при разборе схемы: "));
            outDiag->append(err);
            return false;
        }
    };

	void buildFullErrorMessage(valijson::ValidationResults diagObject, Json::Value* outDiag)
	{
		
	    set<std::string> setDiag;
		size_t lastLen = 0;
		for (const valijson::ValidationResults::Error& it : diagObject)
		{
			if (it.context.size() >= lastLen)
			{
				std::string currDiag = it.description + " Поле: ";
				for (const std::string& strErr : it.context)
				{
					currDiag += strErr;
				};
				setDiag.insert(currDiag);
			};
			lastLen = it.context.size();
		}
		for (const std::string& strLoc : setDiag)
		{
            outDiag->append(strLoc);
		}
		return;
	};

};

