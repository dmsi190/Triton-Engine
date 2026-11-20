// filesystem_manager.hpp

#pragma once

#include "object.hpp"
#include "subsystem.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;

    class XDataFile : public iObject
    {
        TRITON_OBJECT(XDataFile)

        cContext* _context = nullptr;
        types::boolean _is_existing = types::K_FALSE;
        SMemAlloc _data;
        types::usize _byte_size;

    public:
        explicit XDataFile(
            const SMemAlloc& mem_alloc,
            cContext* context,
            const std::string& path,
            types::boolean is_text_file
        );

        ~XDataFile() override;

        inline const types::u8* GetData() const { return _data.mem; }

        inline types::usize GetByteSize() const { return _byte_size; }

        inline types::boolean Exists() const { return _is_existing; }
    };

    class XFileSystem : public XSubsystem
    {
        TRITON_OBJECT(XFileSystem)

    public:
        explicit XFileSystem(const SMemAlloc& mem_alloc, cContext* context);

        ~XFileSystem() override = default;

        std::string TextFileToString(const std::string& path);

        types::usize BinaryFileToArray(
            const std::string& path,
            SMemAlloc& array,
            types::usize offset
        );

        void ReleaseBinaryFileArray(SMemAlloc& array);
        
        types::usize TellFileByteSize(const std::string& path);
    };
}