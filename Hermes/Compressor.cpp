//1.Переписать диагностику в строки возврата

#include "pch.h"
#include "Compressor.h"

using namespace std;

Compressor::Compressor(IMemoryManager* in_iMemoryManager) { m_iMemory = in_iMemoryManager; };
Compressor::~Compressor() { m_iMemory = nullptr; };

bool Compressor::compress_file(FILE* src, FILE* dst, tVariant* pvarRetValue)
{
	
	uint8_t inbuff[CHUNK_SIZE];
	uint8_t outbuff[CHUNK_SIZE];
	z_stream stream = { 0 };

	if (deflateInit(&stream, COMPRESSION_LEVEL) != Z_OK)
	{
		ToV8String(L"deflateInit(...) failed!", pvarRetValue, m_iMemory);
		return false;
	}

	int flush;
	do {
		stream.avail_in = fread(inbuff, 1, CHUNK_SIZE, src);
		if (ferror(src))
		{
			//fprintf(stderr, "fread(...) failed!\n");
			ToV8String(L"fread(...) failed!", pvarRetValue, m_iMemory);
			deflateEnd(&stream);
			return false;
		}

		flush = feof(src) ? Z_FINISH : Z_NO_FLUSH;
		stream.next_in = inbuff;

		do {
			stream.avail_out = CHUNK_SIZE;
			stream.next_out = outbuff;
			deflate(&stream, flush);
			uint32_t nbytes = CHUNK_SIZE - stream.avail_out;

			if (fwrite(outbuff, 1, nbytes, dst) != nbytes ||
				ferror(dst))
			{
				ToV8String(L"fwrite(...) failed!", pvarRetValue, m_iMemory);
				deflateEnd(&stream);
				return false;
			}
		} while (stream.avail_out == 0);
	} while (flush != Z_FINISH);

	deflateEnd(&stream);
	return true;
}

bool Compressor::decompress_file(FILE* src, FILE* dst, tVariant* pvarRetValue)
{
	uint8_t inbuff[CHUNK_SIZE];
	uint8_t outbuff[CHUNK_SIZE];
	z_stream stream = { 0 };

	int result = inflateInit(&stream);
	if (result != Z_OK)
	{
		ToV8String(L"inflateInit(...) failed!", pvarRetValue, m_iMemory);
		return false;
	}

	do {
		stream.avail_in = fread(inbuff, 1, CHUNK_SIZE, src);
		if (ferror(src))
		{
			ToV8String(L"fread(...) failed!", pvarRetValue, m_iMemory);
			inflateEnd(&stream);
			return false;
		}

		if (stream.avail_in == 0)
			break;

		stream.next_in = inbuff;

		do {
			stream.avail_out = CHUNK_SIZE;
			stream.next_out = outbuff;
			result = inflate(&stream, Z_NO_FLUSH);
			if (result == Z_NEED_DICT || result == Z_DATA_ERROR ||
				result == Z_MEM_ERROR)
			{
				ToV8String(L"inflate(...) failed!", pvarRetValue, m_iMemory);
				inflateEnd(&stream);
				return false;
			}

			uint32_t nbytes = CHUNK_SIZE - stream.avail_out;

			if (fwrite(outbuff, 1, nbytes, dst) != nbytes ||
				ferror(dst))
			{
				ToV8String(L"fwrite(...) failed!", pvarRetValue, m_iMemory);
				inflateEnd(&stream);
				return false;
			}
		} while (stream.avail_out == 0);
	} while (result != Z_STREAM_END);

	inflateEnd(&stream);
	return result == Z_STREAM_END;
}

void Compressor::CompressBuffer(tVariant* pvarRetValue, tVariant* paParams)
{
	uLongf compress_buff_size = compressBound(paParams->strLen);
	Bytef* dst_temp_buf = nullptr;
	uLongf compressed_size = compress_buff_size;

	dst_temp_buf = (Bytef*)malloc(compress_buff_size);
	if (dst_temp_buf == nullptr)
	{
		wstring err = wstring(L"Ошибка выделения памяти: ") +
			wstring(L"; compress_buff_size = ") + to_wstring(compress_buff_size);
		ToV8String(err.c_str(), pvarRetValue, m_iMemory);
		return;
	}
	memset(dst_temp_buf, 0, compress_buff_size);

	int res = compress(dst_temp_buf, &compressed_size, (const Bytef*)(paParams->pstrVal), paParams->strLen);


	if (res == Z_OK)
	{
		if (compressed_size && m_iMemory->AllocMemory((void**)&pvarRetValue->pstrVal, compressed_size))
		{
			memcpy(pvarRetValue->pstrVal, dst_temp_buf, compressed_size);
			pvarRetValue->strLen = compressed_size;
			TV_VT(pvarRetValue) = VTYPE_BLOB;

		}
		else
		{
			wstring err = wstring(L"Ошибка выделения памяти менеджером памяти 1с: ") +
				wstring(L"paParams->strLen = ") + to_wstring(paParams->strLen) +
				wstring(L"; compress_buff_size = ") + to_wstring(compress_buff_size) +
				wstring(L"; compressed_size = ") + to_wstring(compressed_size);


			ToV8String(err.c_str(), pvarRetValue, m_iMemory);
		}
	}
	else
	{
		wstring err_ws = wstring(L"Ошибка сжатия: ") + to_wstring(res) +
			wstring(L"; paParams->strLen = ") + to_wstring(paParams->strLen) +
			wstring(L"; compress_buff_size = ") + to_wstring(compress_buff_size) +
			wstring(L"; compressed_size = ") + to_wstring(compressed_size);
		ToV8String(err_ws.c_str(), pvarRetValue, m_iMemory);
	}

	free(dst_temp_buf);

	return;
}

void Compressor::DecompressBuffer(tVariant* pvarRetValue, tVariant* paParams_buf, tVariant* paParams_buf_len)
{
	uLongf awaiting_decompressed_size = (uLongf)paParams_buf_len->intVal;
	uLongf decompressed_size = awaiting_decompressed_size;

	if (awaiting_decompressed_size && m_iMemory->AllocMemory((void**)&pvarRetValue->pstrVal, awaiting_decompressed_size))
	{
		int res = uncompress((unsigned char*)pvarRetValue->pstrVal, &decompressed_size, (unsigned char*)paParams_buf->pstrVal, paParams_buf->strLen);

		if (res == Z_OK)
		{
			pvarRetValue->strLen = decompressed_size;
			TV_VT(pvarRetValue) = VTYPE_BLOB;
		}
		else
		{
			wstring err_ws = wstring(L"Ошибка распаковки: ") + to_wstring(res) +
				wstring(L"; paParams_buf->strLen = ") + to_wstring(paParams_buf->strLen) +
				wstring(L"; awaiting_decompressed_size = ") + to_wstring(awaiting_decompressed_size) +
				wstring(L"; decompressed_size = ") + to_wstring(decompressed_size);
			ToV8String(err_ws.c_str(), pvarRetValue, m_iMemory);
		}
	}
	else
	{
		wstring err = wstring(L"Ошибка выделения памяти менеджером памяти 1с: ") +
			wstring(L"; awaiting_decompressed_size = ") + to_wstring(awaiting_decompressed_size);
		ToV8String(err.c_str(), pvarRetValue, m_iMemory);
	}

	return;
}

void Compressor::CompressFile(tVariant* pvarRetValue, tVariant* paParams_file_src, tVariant* paParams_file_dst)
{
	
	ToV8String(L"ok_en", pvarRetValue, m_iMemory);
	int res;
	char* ch_fn_src;
	res = V8ToChar(paParams_file_src, &ch_fn_src);


	if (res==0 && ch_fn_src!=nullptr)
	{
		FILE* f_src;
		try
		{
			f_src = fopen(ch_fn_src, "rb");
		}
		catch (const std::exception&)
		{
			f_src = nullptr;
		};
		
		if (f_src)
		{
			fseek(f_src, 0, SEEK_SET);

			char* ch_fn_dst;
			res = V8ToChar(paParams_file_dst, &ch_fn_dst);

			if (res == 0 && ch_fn_dst != nullptr)
			{
				FILE* f_dst;
				try
				{
					f_dst = fopen(ch_fn_dst, "wb");
				}
				catch (const std::exception&)
				{
					f_dst = nullptr;
				};
					
				if (f_dst)
				{
					fseek(f_dst, 0, SEEK_SET);
					compress_file(f_src, f_dst, pvarRetValue);
					fclose(f_dst);
				}
				else
				{
					ToV8String(L"Не удалось открыть файл назначения на запись!", pvarRetValue, m_iMemory);
				};
			}
			else
			{

				if (res == 1)
				{
					ToV8String(L"Ошибка выделения памяти для работы с именем файла назначения!", pvarRetValue, m_iMemory);
				}
				else
				{
					ToV8String(L"Имя файла назначения не должно включать символов, не кодируемых в multibyte char (кириллица не катит: это Linux, детка)!", pvarRetValue, m_iMemory);
				}

			}
			fclose(f_src);
		}
		else
		{
			ToV8String(L"Не удалось открыть источник на чтение!", pvarRetValue, m_iMemory);
		};

		delete[] ch_fn_src;
	}
	else
	{
		if (res == 1)
		{
			ToV8String(L"Ошибка выделения памяти для работы с именем источника!", pvarRetValue, m_iMemory);
		}
		else
		{
			ToV8String(L"Имя источника не должно включать символов, не кодируемых в multibyte char (кириллица не катит: это Linux, детка)!", pvarRetValue, m_iMemory);
		}
		
	}

	return;
}

void Compressor::DecompressFile(tVariant* pvarRetValue, tVariant* paParams_file_src, tVariant* paParams_file_dst)
{

	ToV8String(L"ok_en", pvarRetValue, m_iMemory);
	int res;

	char* ch_fn_src;
	res = V8ToChar(paParams_file_src, &ch_fn_src);
	
	if (res == 0 && ch_fn_src != nullptr)
	{
		FILE* f_src;
		try
		{
			f_src = fopen(ch_fn_src, "rb");
		}
		catch (const std::exception&)
		{
		}

		if (f_src)
		{
			fseek(f_src, 0, SEEK_SET);

			char* ch_fn_dst;
			res = V8ToChar(paParams_file_dst, &ch_fn_dst);

			if (res == 0 && ch_fn_dst != nullptr)
			{
				FILE* f_dst;
				try
				{
					f_dst = fopen(ch_fn_dst, "wb");
				}
				catch (const std::exception&)
				{
				}
				if (f_dst)
				{
					fseek(f_dst, 0, SEEK_SET);
					decompress_file(f_src, f_dst, pvarRetValue);
					fclose(f_dst);
				}
				else
				{
					ToV8String(L"Не удалось открыть файл назначения на запись!", pvarRetValue, m_iMemory);
				};
			}
			else
			{
				if (res == 1)
				{
					ToV8String(L"Ошибка выделения памяти для работы с именем файла назначения!", pvarRetValue, m_iMemory);
				}
				else
				{
					ToV8String(L"Имя файла назначения не должно включать символов, не кодируемых в multibyte char (кириллица не катит: это Linux, детка)!", pvarRetValue, m_iMemory);
				}
			}
			fclose(f_src);
		}
		else
		{
			ToV8String(L"Не удалось открыть источник на чтение!", pvarRetValue, m_iMemory);
		};

		delete[] ch_fn_src;
	}
	else
	{
		if (res == 1)
		{
			ToV8String(L"Ошибка выделения памяти для работы с именем источника!", pvarRetValue, m_iMemory);
		}
		else
		{
			ToV8String(L"Имя источника не должно включать символов, не кодируемых в multibyte char (кириллица не катит: это Linux, детка)!", pvarRetValue, m_iMemory);
		}
	}

	return;
}

