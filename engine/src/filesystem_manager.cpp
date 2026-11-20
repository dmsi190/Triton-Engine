// filesystem_manager.cpp

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include "application.hpp"
#include "context.hpp"
#include "filesystem_manager.hpp"
#include "engine.hpp"
#include "creator.hpp"

using namespace types;

triton::XDataFile::XDataFile(
    const SMemAlloc& mem_alloc,
    cContext* context,
    const std::string& path,
    types::boolean is_text_file
) : iObject(mem_alloc, context)
{
    std::ifstream input_file(path, std::ios::binary);

    if (!input_file.is_open())
    {
        _is_existing = K_FALSE;

        Print("Error: file '" + path + "' does not exist");

        return;
    }
    else
    {
        _is_existing = K_TRUE;
    }

    input_file.seekg(0, std::ios::end);

    const usize file_byte_size = input_file.tellg();

    input_file.seekg(0, std::ios::beg);

    _byte_size = file_byte_size + (is_text_file == True ? 1 : 0);

    _data = CMemoryAllocator::Allocate(_byte_size);

    input_file.read((char*)_data.mem, _byte_size);

    input_file.close();
}

triton::XDataFile::~XDataFile()
{
    CMemoryAllocator::Deallocate(_data);
}

triton::XFileSystem::XFileSystem(const SMemAlloc& mem_alloc, cContext* context) : XSubsystem(mem_alloc, context) {}

std::string triton::XFileSystem::TextFileToString(const std::string& path)
{
    XDataFile* file = CCreator::Create<XDataFile>(_context, path, K_TRUE);

    std::string str = "";

    for (usize i = 0; i < file->GetByteSize(); i++)
        str.push_back(((u8*)file->GetData())[i]);

    CCreator::Destroy<XDataFile>(file);
    
    return str;
}

usize triton::XFileSystem::BinaryFileToArray(
    const std::string& path,
    SMemAlloc& array,
    usize offset
)
{
    XDataFile* file = CCreator::Create<XDataFile>(_context, path, K_FALSE);

    const usize file_byte_size = file->GetByteSize();

    if (offset > file_byte_size)
        return 0;

    const usize array_byte_size = file_byte_size - offset;

    array = CMemoryAllocator::Allocate(array_byte_size);

    for (usize i = offset; i < file_byte_size; i++)
        array.mem[i - offset] = file->GetData()[i];

    CCreator::Destroy<XDataFile>(file);

    return array_byte_size;
}

void triton::XFileSystem::ReleaseBinaryFileArray(SMemAlloc& array)
{
    CMemoryAllocator::Deallocate(array);
}

usize triton::XFileSystem::TellFileByteSize(const std::string& path)
{
    std::ifstream input_file(path, std::ios::binary);

    input_file.seekg(0, std::ios::end);

    const usize byte_size = input_file.tellg();

    input_file.seekg(0, std::ios::beg);
    input_file.close();

    return byte_size;
}