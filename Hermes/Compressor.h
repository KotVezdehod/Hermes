#pragma once
#include <stdlib.h>
#include <algorithm>
#include <string>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include "AddInNative.h"
#include "../include/AddInDefBase.h"
#include "../include/IMemoryManager.h"
#include "ConversionWchar.h"
#include <zlib.h>

class Compressor
{

private:
	IMemoryManager* m_iMemory;
	const int CHUNK_SIZE = 4096;
	const int COMPRESSION_LEVEL = Z_BEST_COMPRESSION;

	bool compress_file(FILE* src, FILE* dst, tVariant* pvarRetValue);
	bool decompress_file(FILE* src, FILE* dst, tVariant* pvarRetValue);

public:
	Compressor(IMemoryManager* in_iMemoryManager);
	~Compressor();
	void CompressBuffer(tVariant* pvarRetValue, tVariant* paParams);
	void DecompressBuffer(tVariant* pvarRetValue, tVariant* paParams_buf, tVariant* paParams_buf_len);
	void CompressFile(tVariant* pvarRetValue, tVariant* paParams_file_src, tVariant* paParams_file_dst);
	void DecompressFile(tVariant* pvarRetValue, tVariant* paParams_file_src, tVariant* paParams_file_dst);

};

