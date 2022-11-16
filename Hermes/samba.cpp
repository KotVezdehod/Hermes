#include "samba.h"
#include <string>
#include <fcntl.h>
#include <unistd.h>

#include "../smb2/include/smb2/smb2.h"
#include "../smb2/include/smb2/libsmb2.h"
#include "../smb2/include/smb2/libsmb2-raw.h"

#include "Json.h"

void Samba::ListCatalog(tVariant* paParams, tVariant* pvarRetValue)
{
	
	if (TV_VT(&paParams[0]) != VTYPE_PWSTR)
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"Первый gараметр - URL: строка. От 1 до 1024 символов.");
		return;
	}
	else if((&paParams[0])->wstrLen > 1024 || (&paParams[0])->wstrLen == 0)
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"Первый gараметр - URL: строка. От 1 до 1024 символов.");
		return;
	}

	if (TV_VT(&paParams[1]) != VTYPE_PWSTR)
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"Второй параметр - пароль: строка. От 1 до 32 символов.");
		return;
	}
	else if ((&paParams[1])->wstrLen > 32 || (&paParams[1])->wstrLen == 0)
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"Второй параметр - пароль: строка. От 1 до 32 символов.");
		return;
	}

	wchar_t* wchFullUrlIn = nullptr;
	convFromShortWchar(&wchFullUrlIn, (&paParams[0])->pwstrVal);

	if (!wchFullUrlIn)
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"SmbLib: invalid URL\n");
		return;
	}
	
	wchar_t* wchPsw = nullptr;
	char* chPsw = nullptr;
	convFromShortWchar(&wchPsw, (&paParams[1])->pwstrVal);
	size_t pswSz = 0;

	if (wchPsw)
	{
		pswSz = (wcslen(wchPsw) + 1) * sizeof(wchar_t);
		chPsw = new char[pswSz];
		wcstombs(chPsw, wchPsw, pswSz);
		delete[] wchPsw;
	}
	
	struct smb2_context* smb2;
	smb2 = smb2_init_context();
	if (smb2)
	{
		char* sFullUrl = nullptr;
		size_t inURLsz = (wcslen(wchFullUrlIn) + 1) * sizeof(wchar_t);
		sFullUrl = new char[inURLsz];
		memset(sFullUrl, 0, inURLsz);
		wcstombs(sFullUrl, wchFullUrlIn, inURLsz);

		struct smb2_url* url = smb2_parse_url(smb2, sFullUrl);
		if (url)
		{
			if (url->user && strlen(url->user) > 0)
			{
				smb2_set_user(smb2, url->user);
			}

			if (chPsw && strlen(chPsw) > 0)
			{
				smb2_set_password(smb2, chPsw);
			}
			smb2_set_security_mode(smb2, SMB2_NEGOTIATE_SIGNING_ENABLED);

			if (smb2_connect_share(smb2, url->server, url->share, url->user) >= 0) 
			{
			
				struct smb2dir* dir = smb2_opendir(smb2, url->path);
				if (dir) 
				{

					struct smb2dirent* ent = nullptr;
					char* link = nullptr;

					Json::Value root;
					Json::Value arr = Json::arrayValue;
					arr.clear();

					while ((ent = smb2_readdir(smb2, dir))) 
					{
						Json::Value v;

						const char* type;
						time_t t;

						t = (time_t)ent->st.smb2_mtime;
						switch (ent->st.smb2_type)
						{
						case SMB2_TYPE_LINK:
							type = "LINK";
							break;
						case SMB2_TYPE_FILE:
							type = "FILE";
							break;
						case SMB2_TYPE_DIRECTORY:
							type = "DIRECTORY";
							break;
						default:
							type = "unknown";
							break;
						}

						v["name"] = ent->name;
						v["type"] = type;
						v["size"] = static_cast<unsigned int>(ent->st.smb2_size);
						v["time"] = asctime(localtime(&t));

						if (ent->st.smb2_type == SMB2_TYPE_LINK)
						{
							char buf[256];

							if (url->path && url->path[0]) 
							{
								v["link_path"] = url->path;
								v["link_name"] = ent->name;
							}
							else 
							{
								v["link_path"] = "";
								v["link_name"] = ent->name;
						
							}
							smb2_readlink(smb2, link, buf, 256);
							v["link"] = buf;
							free(link);
						}
						arr.append(v);
					}
					
					root["Status"] = true;
					root["Description"] = "";
					root["Data"] = arr;
					
					Json::FastWriter fw;
					string s_res = fw.write(root);
					ToV8StringFromChar(s_res.c_str(), pvarRetValue, iMemoryManager);
					smb2_closedir(smb2, dir);
				}
				else
				{
					stringstream err;
					err << "SmbLib: smb2_opendir failed (" << smb2_get_error(smb2) << ")";
					string strErr(err.str());
					DiagToV8String(pvarRetValue, iMemoryManager, false, strErr.c_str());
				}

				smb2_disconnect_share(smb2);

			}
			else
			{
				stringstream err;
				err << "SmbLib: smb2_connect_share failed (" << smb2_get_error(smb2) << ")";
				string strErr(err.str());
				DiagToV8String(pvarRetValue, iMemoryManager, false, strErr.c_str());
			}

			smb2_destroy_url(url);
		}
		else
		{
			stringstream err;
			err << "SmbLib: Failed to parse url (" << smb2_get_error(smb2) << ")";
			string strErr(err.str());
			DiagToV8String(pvarRetValue, iMemoryManager, false, strErr.c_str());
			
		}
		if (sFullUrl)
			delete[] sFullUrl;
	}
	else
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"SmbLib: Failed to init context");
	}

	if (wchFullUrlIn)
		delete[] wchFullUrlIn;

	if (chPsw)
		delete[] chPsw;

	smb2_destroy_context(smb2);
	return;

}

void Samba::GetFileData(tVariant* paParams, tVariant* pvarRetValue)
{

	size_t MAXBUF = 16 * 1024 * 1024;
	if (TV_VT(&paParams[0]) != VTYPE_PWSTR)
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"Первый параметр - URL: строка. От 1 до 1024 символов.");
		return;
	}
	else if ((&paParams[0])->wstrLen > 1024 || (&paParams[0])->wstrLen == 0)
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"Первый параметр - URL: строка. От 1 до 1024 символов.");
		return;
	}

	if (TV_VT(&paParams[1]) != VTYPE_PWSTR)
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"Второй параметр - пароль: строка. От 1 до 32 символов.");
		return;
	}
	else if ((&paParams[1])->wstrLen > 32 || (&paParams[1])->wstrLen == 0)
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"Второй параметр - пароль: строка. От 1 до 32 символов.");
		return;
	}

	wchar_t* wchFullUrlIn = nullptr;
	convFromShortWchar(&wchFullUrlIn, (&paParams[0])->pwstrVal);

	if (!wchFullUrlIn)
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"SmbLib: invalid URL\n");
		return;
	}

	wchar_t* wchPsw = nullptr;
	char* chPsw = nullptr;
	convFromShortWchar(&wchPsw, (&paParams[1])->pwstrVal);
	size_t pswSz = 0;

	if (wchPsw)
	{
		pswSz = (wcslen(wchPsw) + 1) * sizeof(wchar_t);
		chPsw = new char[pswSz];
		wcstombs(chPsw, wchPsw, pswSz);
		delete[] wchPsw;
	}

	if (!isNumericParameter(&paParams[2]))
	{
		wstringstream wssLoc;
		wssLoc << L"Третий параметр - размер буффера: число, от 1 до " << std::to_wstring(MAXBUF) << " байт.";
		DiagToV8String(pvarRetValue, iMemoryManager, false, wssLoc.str().c_str());
		return;
	}

	size_t bufSz = static_cast<size_t>(numericValue(&paParams[2]));

	if (bufSz > MAXBUF || bufSz == 0)
	{
		wstringstream wssLoc;
		wssLoc << L"Третий параметр - размер буффера: число, от 1 до " << std::to_wstring(MAXBUF) << " байт.";
		DiagToV8String(pvarRetValue, iMemoryManager, false, wssLoc.str().c_str());
		return;
	}

	if (!isNumericParameter(&paParams[3]))
	{
		wstringstream wssLoc;
		wssLoc << L"Четвертый параметр - позиция начала чтения: неотрицательное число.";
		DiagToV8String(pvarRetValue, iMemoryManager, false, wssLoc.str().c_str());
		return;
	}
	size_t startPos = static_cast<size_t>(numericValue(&paParams[3]));


	if ((&paParams[4])->vt != VTYPE_BLOB)
	{
		wstringstream wssLoc;
		wssLoc << L"Пятый параметр должен быть указателем на двоичные данные.";
		DiagToV8String(pvarRetValue, iMemoryManager, false, wssLoc.str().c_str());
		return;
	}
	
	struct smb2_context* smb2;
	smb2 = smb2_init_context();
	if (smb2)
	{
		char* sFullUrl = nullptr;
		size_t inURLsz = (wcslen(wchFullUrlIn) + 1) * sizeof(wchar_t);
		sFullUrl = new char[inURLsz];
		memset(sFullUrl, 0, inURLsz);
		wcstombs(sFullUrl, wchFullUrlIn, inURLsz);

		struct smb2_url* url = smb2_parse_url(smb2, sFullUrl);
		if (url)
		{
			if (url->user && strlen(url->user) > 0)
			{
				smb2_set_user(smb2, url->user);
			}

			if (chPsw && strlen(chPsw) > 0)
			{
				smb2_set_password(smb2, chPsw);
			}
			smb2_set_security_mode(smb2, SMB2_NEGOTIATE_SIGNING_ENABLED);

			if (smb2_connect_share(smb2, url->server, url->share, url->user) >= 0)
			{
				smb2fh* fh = nullptr;
				int count = 0;

				fh = smb2_open(smb2, url->path, O_RDONLY);
				if (fh)
				{
					unsigned char* buf = new unsigned char[bufSz];
					bool success = true;
					while ((count = smb2_pread(smb2, fh, buf, bufSz, startPos)) != 0)
					{
						if (count == -EAGAIN) 
						{
							continue;
						}
						if (count < 0) 
						{
							success = false;
							stringstream err;
							err << "SmbLib: failed to read file (" << smb2_get_error(smb2) << ")";
							string strErr(err.str());
							DiagToV8String(pvarRetValue, iMemoryManager, false, strErr.c_str());
						}

						break;
					};

					if (success)
					{

						iMemoryManager->FreeMemory(reinterpret_cast<void**>(&(&paParams[4])->pstrVal));
						iMemoryManager->AllocMemory(reinterpret_cast<void**>(&(&paParams[4])->pstrVal), count);
						(&paParams[4])->strLen = count;
						memcpy((&paParams[4])->pstrVal, buf, count);

						Json::Value root;
						Json::Value v;
						v["size"] = count;

						root["Status"] = true;
						root["Description"] = "";
						root["Data"] = v;

						Json::FastWriter fw;
						string s_res = fw.write(root);
						ToV8StringFromChar(s_res.c_str(), pvarRetValue, iMemoryManager);
					}
					
					if (buf)
						delete[] buf;

					smb2_close(smb2, fh);
				}
				else
				{
					stringstream err;
					err << "SmbLib: smb2_open failed (" << smb2_get_error(smb2) << ")";
					string strErr(err.str());
					DiagToV8String(pvarRetValue, iMemoryManager, false, strErr.c_str());
				}

				smb2_disconnect_share(smb2);

			}
			else
			{
				stringstream err;
				err << "SmbLib: smb2_connect_share failed (" << smb2_get_error(smb2) << ")";
				string strErr(err.str());
				DiagToV8String(pvarRetValue, iMemoryManager, false, strErr.c_str());
			}

			smb2_destroy_url(url);
		}
		else
		{
			stringstream err;
			err << "SmbLib: Failed to parse url (" << smb2_get_error(smb2) << ")";
			string strErr(err.str());
			DiagToV8String(pvarRetValue, iMemoryManager, false, strErr.c_str());

		}
		if (sFullUrl)
			delete[] sFullUrl;
	}
	else
	{
		DiagToV8String(pvarRetValue, iMemoryManager, false, L"SmbLib: Failed to init context");
	}

	if (wchFullUrlIn)
		delete[] wchFullUrlIn;

	if (chPsw)
		delete[] chPsw;

	smb2_destroy_context(smb2);
	return;
}

bool Samba::isNumericParameter(tVariant* par)
{
	return par->vt == VTYPE_I4 || par->vt == VTYPE_UI4 || par->vt == VTYPE_R8;
}

long Samba::numericValue(tVariant* par)
{
	long ret = 0;
	switch (par->vt)
	{
	case VTYPE_I4:
		ret = par->lVal;
		break;
	case VTYPE_UI4:
		ret = par->ulVal;
		break;
	case VTYPE_R8:
		ret = par->dblVal;
		break;
	case VTYPE_R4:
		ret = par->dblVal;
		break;
	}
	return ret;
}
