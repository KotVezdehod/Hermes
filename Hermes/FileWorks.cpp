#include "pch.h"
#include "FileWorks.h"
#include "Json.h"
#include <codecvt>
#include <dirent.h>
#include <string>
//#include <iostream>
#include "AddInNative.h"
#include <sys/stat.h>
#include <codecvt>


using namespace std;

void FileWorks::ScanFolder(tVariant* pvarRetValue, tVariant* paParams)
{
	if (paParams->vt == VTYPE_PWSTR)
	{
		char* ch_dir;
		V8ToChar(paParams, &ch_dir);

		DIR* dir;
		struct dirent* ent;
		if ((dir = opendir(ch_dir)) != NULL)
		{
			Json::Value root;

			int counter = 0;
			while ((ent = readdir(dir)) != NULL)
			{
				string tmp = std::to_string(counter);
				tmp = "entry_" + tmp;
				root[tmp.c_str()] = ent->d_name;
				counter++;
			}
			closedir(dir);

			Json::StreamWriterBuilder builder;
			string s_res = Json::writeString(builder, root);

			std::wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
			std::wstring ws = converter.from_bytes(s_res);

			wchar_t* wch_json = nullptr;
			DiagStructure(true, L"", ws.c_str(), &wch_json);
			ToV8String(wch_json, pvarRetValue, mIMemoryManager);
			delete wch_json;
		}
		else
		{
			DiagToV8String(pvarRetValue, mIMemoryManager, false, L"Ошибка при открытии каталога");
		}
	}
	else
	{
		DiagToV8String(pvarRetValue, mIMemoryManager, false, L"Параметр должен быть строкой");

	}
}

void FileWorks::IsDirectory(tVariant* pvarRetValue, tVariant* paParams)
{

	if (paParams->vt == VTYPE_PWSTR)
	{

		char* path = nullptr;

		if (!V8ToChar(paParams, &path))
		{
			if (path)
			{
				struct stat* buf = (struct stat*)malloc(sizeof(struct stat));
				memset(buf, 0, sizeof(struct stat));
				if (!stat(path, buf))
				{

					Json::Value root;
					
					if (buf->st_mode & S_IFDIR)
					{
						root["IsDirectory"] = true;
					}
					/*else if (buf->st_mode & S_IFREG)
					{
						root["IsDirectory"] = 0;
					}*/
					else
					{
						root["IsDirectory"] = false;
					}

					Json::StreamWriterBuilder builder;
					string s_res = Json::writeString(builder, root);

					std::wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
					std::wstring ws = converter.from_bytes(s_res);

					wchar_t* wch_json = nullptr;
					if (DiagStructure(true, L"", ws.c_str(), &wch_json))
					{
						if (wch_json)
						{
							ToV8String(wch_json, pvarRetValue, mIMemoryManager);
							delete[] wch_json;
						}
						else
						{
							DiagToV8String(pvarRetValue, mIMemoryManager, false, L"Не удалось построить диагностическую структуру.");
						}
					}
				}
				else
				{
					DiagToV8String(pvarRetValue, mIMemoryManager, false, L"Не удалось получить атрибуты объекта файловой системы.");
				}
				free(buf);
				delete path;
			}
		}
	}
	else
	{
		DiagToV8String(pvarRetValue, mIMemoryManager, false, L"Параметр должен быть строкой");
	}

	return;
}

void FileWorks::CreateDirectory(tVariant* pvarRetValue, tVariant* paParams)
{
		if ((&paParams[0])->vt == VTYPE_PWSTR)
		{
			if ((&paParams[1])->vt == VTYPE_I4 || (&paParams[1])->vt == VTYPE_UI4 || (&paParams[1])->vt == VTYPE_R8)
			{
				char* path = nullptr;

				if (!V8ToChar(&paParams[0], &path))
				{
					if (path)
						
					{
						//if (mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
						//if (mkdir(path, S_IRWXU | S_IRGRP | S_IXGRP) != 0)
						if (!mkdir(path, numericValue(&paParams[1])))
						{
							DiagToV8String(pvarRetValue, mIMemoryManager, true, L"Ок");
						}
						else
						{
							DiagToV8String(pvarRetValue, mIMemoryManager, false, L"Fail");
						}

						delete path;
					}
				}
			}
			else
			{
				DiagToV8String(pvarRetValue, mIMemoryManager, false, L"Второй параметр (атрибуты каталога) должен быть числом.");
			}
		}
		else
		{
			DiagToV8String(pvarRetValue, mIMemoryManager, false, L"Первый параметр (путь к родительскому каталогу) должен быть строкой.");
		}
	return;
}

void FileWorks::DeleteFileOrDirectory(tVariant* pvarRetValue, tVariant* paParams)
{

	if (paParams->vt == VTYPE_PWSTR)
	{

		char* path = nullptr;

		if (!V8ToChar(paParams, &path))
		{
			if (path)
			{

				if (!remove(path))
				{
					DiagToV8String(pvarRetValue, mIMemoryManager, true, L"Ок");
				}
				else
				{
					DiagToV8String(pvarRetValue, mIMemoryManager, false, L"Fail");
				}

				delete path;
			}
		}

	}
	else
	{
		DiagToV8String(pvarRetValue, mIMemoryManager, false, L"Параметр должен быть строкой");
	}
	return;
}

void FileWorks::CreateFile(tVariant* pvarRetValue, tVariant* paParams)
{

	if (paParams->vt == VTYPE_PWSTR)
	{
			char* path = nullptr;

			if (!V8ToChar(&paParams[0], &path))
			{
				if (path)
				{
					ofstream ostr;
					ostr.open(path, std::_LIBCPP_NAMESPACE::ios_base::out);
					if (ostr.is_open())
					{
						ostr.flush();
						DiagToV8String(pvarRetValue, mIMemoryManager, true, L"Ок");
					}
					else
					{
						DiagToV8String(pvarRetValue, mIMemoryManager, false, L"Fail");
					}

					delete path;
				}
			}
	}
	else
	{
		DiagToV8String(pvarRetValue, mIMemoryManager, false, L"Первый параметр (путь к файлу) должен быть строкой.");
	}
}

void FileWorks::IsFSOPresent(tVariant* pvarRetValue, tVariant* paParams)
{

	if (paParams->vt == VTYPE_PWSTR)
	{

		char* path = nullptr;

		if (!V8ToChar(paParams, &path))
		{
			if (path)
			{
				struct stat* buf = (struct stat*)malloc(sizeof(struct stat));
				memset(buf, 0, sizeof(struct stat));
				if (!stat(path, buf))
				{
					DiagToV8String(pvarRetValue, mIMemoryManager, true, L"");
				}
				else
				{
					DiagToV8String(pvarRetValue, mIMemoryManager, false, L"");
				}
				free(buf);
				delete path;
			}
		}
	}
	else
	{
		DiagToV8String(pvarRetValue, mIMemoryManager, false, L"Параметр должен быть строкой");
	}

	return;
}

void FileWorks::RenameFileOrDirectory(tVariant* pvarRetValue, tVariant* paParams)
{
	
	if ((&paParams[0])->vt == VTYPE_PWSTR)
	{
		if ((&paParams[1])->vt == VTYPE_PWSTR)
		{
			char* path0 = nullptr;

			if (!V8ToChar((&paParams[0]), &path0))
			{
				if (path0)
				{
					
					char* path1 = nullptr;
					if (!V8ToChar((&paParams[1]), &path1))
					{
						if (path1)
						{
							if (!rename(path0, path1))
							{
								DiagToV8String(pvarRetValue, mIMemoryManager, true, L"Ok.");
							}
							else
							{
								DiagToV8String(pvarRetValue, mIMemoryManager, false, L"Не удалось переименовать объект.");
							}

							delete path1;
						}
						else
						{
							DiagToV8String(pvarRetValue, mIMemoryManager, false, L"Не удалось выделить память для второго параметра.");
						}
					}
					else
					{
						DiagToV8String(pvarRetValue, mIMemoryManager, false, L"Не удалось выделить память для второго параметра.");
					}
					delete path0;
				}
				else
				{
					DiagToV8String(pvarRetValue, mIMemoryManager, false, L"Не удалось выделить память для первого параметра.");
				}
			}
			else
			{
				DiagToV8String(pvarRetValue, mIMemoryManager, false, L"Не удалось выделить память для первого параметра.");
			}
		}
		else
		{
			DiagToV8String(pvarRetValue, mIMemoryManager, false, L"Второй параметр должен быть строкой (новое имя FSO)");
		}
	}
	else
	{
		DiagToV8String(pvarRetValue, mIMemoryManager, false, L"Первый параметр должен быть строкой (исходное имя FSO)");
	}
	return;
}

void FileWorks::WriteDataToFile(tVariant* pvarRetValue, tVariant* paParams)
{
	if ((&paParams[0])->vt == VTYPE_PWSTR)
	{
		if ((&paParams[1])->vt == VTYPE_BLOB)
		{
			char* path = nullptr;

			if (!V8ToChar((&paParams[0]), &path))
			{
				if (path)
				{
					ofstream ostr;
					ostr.open(path, ios_base::trunc);

					if (ostr.is_open())
					{
						ostr.flush();
						ostr.close();
						ostr.open(path, ios_base::binary);
						if (ostr.is_open())
						{
							ostr.write((&paParams[1])->pstrVal, (&paParams[1])->strLen);
							ostr.close();
							DiagToV8String(pvarRetValue, mIMemoryManager, true, L"");
						}
						else
						{
							DiagToV8String(pvarRetValue, mIMemoryManager, false, L"Не удалось открыть поток на запись в режиме двоичных данных в указанный файл.");
						}
					}
					else
					{
						DiagToV8String(pvarRetValue, mIMemoryManager, false, L"Не удалось открыть поток на запись в указанный файл.");
					}
					delete path;
				}
			}
		}
		else
		{
			DiagToV8String(pvarRetValue, mIMemoryManager, false, L"Второй параметр должен быть двоичными данными");
		}
	}
	else
	{
		DiagToV8String(pvarRetValue, mIMemoryManager, false, L"Первый параметр должен быть строкой");
	}

	return;

}

void FileWorks::ReadDataFromFile(tVariant* pvarRetValue, tVariant* paParams)
{
	if ((&paParams[0])->vt == VTYPE_PWSTR)
	{
		char* path = nullptr;

		if (!V8ToChar(paParams, &path))
		{
			if (path)
			{
				ifstream istr;
				istr.open(path, ios_base::binary);

				if (istr.is_open())
				{
					istr.seekg(0, istr.end);
					int n = istr.tellg()*sizeof(char);
					istr.seekg(0, istr.beg);

					if (mIMemoryManager->AllocMemory((void**)&pvarRetValue->pstrVal, n))
					{
						istr.read(pvarRetValue->pstrVal, n);
						(&pvarRetValue[0])->strLen = n;
						TV_VT(pvarRetValue) = VTYPE_BLOB;
					}
					else
					{
						DiagToV8String(pvarRetValue, mIMemoryManager, false, L"Не удалось выделить память под двоичные данные.");
					}
					
				}
				else
				{
					DiagToV8String(pvarRetValue, mIMemoryManager, false, L"Не удалось открыть поток на чтение из указанного файла.");
				}

				delete path;
			}
		}
	}
	else
	{
		DiagToV8String(pvarRetValue, mIMemoryManager, false, L"Первый параметр должен быть строкой");
	}

	return;

}

long FileWorks::numericValue(tVariant* par)
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

