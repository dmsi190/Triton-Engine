// filesystem_manager.hpp

#pragma once

#include "object.hpp"
#include "subsystem.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;

    class CDataFile : public iObject
    {
        cContext* _context = nullptr;
        types::boolean _is_existing = types::K_FALSE;
        SMemoryAllocation _data;
        types::usize _byte_size;

    public:
        explicit CDataFile(
            const SMemoryAllocation& mem_alloc,
            cContext* context,
            const std::string& path,
            types::boolean is_text_file
        );

        ~CDataFile() override;

        inline const types::u8* GetData() const { return _data.mem; }

        inline types::usize GetByteSize() const { return _byte_size; }

        inline types::boolean Exists() const { return _is_existing; }
    };

    class CFileSystem : public CSubsystem
    {
        TRITON_CLASS_NAME(CFileSystem)

    public:
        explicit CFileSystem(cContext* context);

        ~CFileSystem() override = default;

        std::string TextFileToString(const std::string& path);

        types::usize BinaryFileToArray(
            const std::string& path,
            SMemoryAllocation& array,
            types::usize offset
        );

        void ReleaseBinaryFileArray(const SMemoryAllocation& array);
        
        types::usize TellFileByteSize(const std::string& path);
    };
}