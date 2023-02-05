//https://www.tutorialspoint.com/sqlite/sqlite_c_cpp.htm

#pragma once

#include "sqlite3.h"
#include <vector>
#include <string>
#include <codecvt>
#include <sstream>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include "Json.h"

using namespace std;

//typedef int (*sqlite3_callback)
	//	(
	//		void*,    /* Data provided in the 4th argument of sqlite3_exec() */
	//		int,      /* The number of columns in row */
	//		char**,   /* An array of strings representing fields in the row */
	//		char**    /* An array of strings representing column names */
	//		);

//static int callback(void* s_out, int argc, char** argv, char** azColName);
//static bool SQLite_is_buisy = false;

class Sqlite3_ConnID
{

private:
	Json::Value GetResult(bool Status, string Description, void* in_data)
	{
		Json::Value root;

		root["Status"] = Status;
		root["Description"] = Description;
		if (in_data == nullptr)
		{
			Json::Value empty;
			root["Data"] = empty;
		}
		else
		{
			root["Data"] = *((Json::Value*)in_data);
		}
		return root;
	}


public:
	int connection_id = -1;
	wstring DbName;
	sqlite3* connection = NULL;
	Sqlite3_ConnID(wstring* in_DbName, int in_connection_id) { DbName = wstring(in_DbName->c_str()); connection_id = in_connection_id; }
	bool SQLite_is_buisy = false;


	Json::Value Exec(const char* statement)
	{

		SQLite_is_buisy = true;
		char* err = nullptr;
		Json::Value root;
		Json::Value Jval;
		Jval = Json::arrayValue;


		if (sqlite3_exec(connection, statement, [](void* s_out, int argc, char** argv, char** azColName)
			{
				Json::Value* JArrVal = (Json::Value*)s_out;
				Json::Value v;
				Json::Value empty;
				for (size_t i = 0; i < argc; i++)
				{
					v[azColName[i]] = argv[i] ? argv[i] : empty;
				};
				JArrVal->append(v);
				return 0;
			},
			&Jval, &err))
		{
			string s_err = string(err);
			sqlite3_free(err);
			SQLite_is_buisy = false;
			return GetResult(false, s_err, nullptr);
		};
			SQLite_is_buisy = false;
			root["rows"] = Jval;
			return GetResult(true, string(""), &root);
	};

};

class Sqlite3_1c
{

public:

	vector<Sqlite3_ConnID> ConnectionList;

	Json::Value Open(wstring* DbFileName);
	Json::Value Close(int in_connection_id);
	Json::Value Exec(int in_connection_id, wstring* Statement);
	Json::Value GetConnections();
	Json::Value GetDbDetails(int in_connection_id);
	Json::Value Exec_v2(wstring* DbFileName, wstring* Statement);
	Json::Value InsertBlobData(int in_connection_id, wstring* Statement, void* data, int sz);
	Json::Value SelectBlobData(int in_connection_id, wstring* Statement);

	string WstringToString(wstring* wch_in);

private:
	int GetNewConnectionID();
	Json::Value GetResult(bool Status, string Description, void* in_data);

};
