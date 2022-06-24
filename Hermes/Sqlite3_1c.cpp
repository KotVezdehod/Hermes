#include "Sqlite3_1c.h"
//try to use functors
//vector for(...)
//https://www.sqlite.org/capi3ref.html#sqlite3_db_filename
//https://habr.com/ru/company/infopulse/blog/226557/

Json::Value Sqlite3_1c::Open(wstring* DbFileName)
{
    if (DbFileName->empty())
        return GetResult(false, string("Не указано имя базы данных."), nullptr);

    int new_conn_id = GetNewConnectionID();
    if (new_conn_id == -1)
        return GetResult(false, string("Превышен лимит открытых соединений (1000)."), nullptr);


    ConnectionList.push_back(Sqlite3_ConnID(DbFileName, new_conn_id));
    string sDbFileNane = WstringToString(DbFileName);


    if (sqlite3_open(sDbFileNane.c_str(), &((ConnectionList[ConnectionList.size() - 1]).connection)))
    {
        string s_ret = string(sqlite3_errmsg(((ConnectionList[ConnectionList.size() - 1]).connection)));
        ConnectionList.pop_back();
        Json::Value ret = GetResult(false, s_ret, nullptr);
        return ret;
    };

    Json::Value jv_conn_i;
    jv_conn_i["conn_id"] = new_conn_id;

    return GetResult(true, string(""), &jv_conn_i);

}

Json::Value Sqlite3_1c::Close(int in_connection_id)
{
    wstring strtmp;
    int x = 0;
    for (Sqlite3_ConnID& p : ConnectionList)
    {
        if (in_connection_id == p.connection_id)
        {
            if (sqlite3_close(p.connection))
            {
                Json::Value ret = GetResult(false, string(sqlite3_errmsg(p.connection)), nullptr);
                return ret;
            }
            ConnectionList.erase(ConnectionList.begin() + x);
            return GetResult(true, string(""), nullptr);
        }
        x++;
    };

    return GetResult(false, string("Нет открытых соединений с указанным ID."), nullptr);
}

Json::Value Sqlite3_1c::Exec(int in_connection_id, wstring* Statement)
{
    int x = 0;
    for (Sqlite3_ConnID& p : ConnectionList)
    {
        if (in_connection_id == p.connection_id)
        {
            if (p.SQLite_is_buisy)
                return GetResult(false, string("SQLite движок занят. Попробуйте позднее."), nullptr);

            Json::Value root;
            root = p.Exec(WstringToString(Statement).c_str());
            return root;
        }
        x++;
    };
    return GetResult(false, string("Нет открытых соединений с указанным ID."), nullptr);
}

Json::Value Sqlite3_1c::GetConnections()
{
    Json::Value root;
    Json::Value arr = Json::arrayValue;
    arr.clear();


    for (Sqlite3_ConnID& conn : ConnectionList)
    {
        Json::Value v;
        v["db_name"] = WstringToString(&conn.DbName);
        v["conn_id"] = conn.connection_id;
        arr.append(v);
    }

    root["rows"] = arr;

    return GetResult(true, string(""), &root);
}

Json::Value Sqlite3_1c::GetDbDetails(int in_connection_id)
{
    for (Sqlite3_ConnID& conn : ConnectionList)
    {
        if (in_connection_id == conn.connection_id)
        {
            Json::Value root;
            ifstream in(WstringToString(&conn.DbName), ifstream::ate | std::ifstream::binary);
            in.seekg(0, in.end);
            int pos = in.tellg();
            root["Size"] = pos;

            return GetResult(true, string(""), &root);
        };
    };

    return GetResult(false, string("Нет открытых соединений с указанным ID."), nullptr);
}

Json::Value Sqlite3_1c::Exec_v2(wstring* DbFileName, wstring* Statement)
{
    if (DbFileName->empty())
        return GetResult(false, string("Не указано имя базы данных."), nullptr);

    string sDbFileNane = WstringToString(DbFileName);
    Json::Value ret;
    sqlite3* db = nullptr;

    if (sqlite3_open(sDbFileNane.c_str(), &db))
    {
        string s_ret = string(sqlite3_errmsg(((ConnectionList[ConnectionList.size() - 1]).connection)));
        ret = GetResult(false, s_ret, nullptr);
        return ret;
    };

    Sqlite3_ConnID Sqlite3_ConnID(DbFileName, -1);
    Sqlite3_ConnID.connection = db;
    string ch_statement = WstringToString(Statement);
    ret = Sqlite3_ConnID.Exec(ch_statement.c_str());

    if (ret["status"] == false)
    {
        string s_ret = string(sqlite3_errmsg(db));
        ret = GetResult(false, s_ret, nullptr);
    }
    sqlite3_close(db);
    return ret;
}

int Sqlite3_1c::GetNewConnectionID()
{

    for (size_t i = 0; i < 1000; i++)
    {
        bool locked = false;
        for (Sqlite3_ConnID& p : ConnectionList)
        {
            if (p.connection_id == i)
            {
                locked = true;
                break;
            }
        }
        if (!locked)
        {
            return i;
        }
    }

    return -1;
}

Json::Value Sqlite3_1c::InsertBlobData(int in_connection_id, wstring* Statement, void* data, int sz)
{
    //Statement = "insert into table_name (column_name) VALUES(?);"

    string s_statement = WstringToString(Statement);
    sqlite3_stmt* stmt;
    sqlite3* db = nullptr;

    for (Sqlite3_ConnID& p : ConnectionList)
    {
        if (p.connection_id == in_connection_id)
        {
            db = p.connection;
            break;
        }
    }

    if (db == nullptr)
    {
        return GetResult(false, string("Нет открытых соединений с указанным ID."), nullptr);
    }

    if (sqlite3_prepare_v2(db, s_statement.c_str(), s_statement.size() * sizeof(char), &stmt, nullptr))
    {
        return GetResult(false, string(sqlite3_errmsg(db)), nullptr);
    }

    if (sqlite3_bind_blob(stmt, 1, data, sz, nullptr))
    {
        sqlite3_finalize(stmt);
        return GetResult(false, string(sqlite3_errmsg(db)), nullptr);
    }

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        return GetResult(false, string(sqlite3_errmsg(db)), nullptr);
    }

    sqlite3_finalize(stmt);
    return GetResult(true, "", nullptr);
}

Json::Value Sqlite3_1c::SelectBlobData(int in_connection_id, wstring* Statement)
{
    sqlite3* db = nullptr;
    for (Sqlite3_ConnID& p : ConnectionList)
    {
        if (p.connection_id == in_connection_id)
        {
            db = p.connection;
            break;
        }
    }

    if (!db)
    {
        return GetResult(false, string("Нет открытых соединений с указанным ID."), nullptr);
    }

    sqlite3_stmt* stmt = nullptr;

    string s_statement = WstringToString(Statement);


    if (sqlite3_prepare_v2(db, s_statement.c_str(), s_statement.size() * sizeof(char), &stmt, nullptr))
    {
        return GetResult(false, string(sqlite3_errmsg(db)), nullptr);
    };
    int result = sqlite3_step(stmt);

    if (result == SQLITE_ROW)
    {
        const void* pReadBlobData = sqlite3_column_blob(stmt, 0);
        int len = sqlite3_column_bytes(stmt, 0);
        unsigned char* read_data = new unsigned char[len + 1];
        memset(read_data, 0, len + 1);
        memcpy(read_data, pReadBlobData, len);

        Json::LargestUInt lUint;
        lUint = reinterpret_cast<uint64_t>(read_data);

        Json::Value v;
        v["Pointer"] = lUint;
        v["Size"] = len;

        sqlite3_finalize(stmt);

        return GetResult(true, "", &v);
    }

    sqlite3_finalize(stmt);

    return GetResult(false, string(sqlite3_errmsg(db)), nullptr);
}


string Sqlite3_1c::WstringToString(wstring* wch_in)
{
    wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(wch_in->c_str());
}

Json::Value Sqlite3_1c::GetResult(bool Status, string Description, void* in_data)
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

