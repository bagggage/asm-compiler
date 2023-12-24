#ifndef __ASM_EXE_OBJECT_H
#define __ASM_EXE_OBJECT_H

#include <limits>
#include <vector>

#include "assembled-object.h"
#include "codegen/machine-code.h"

namespace ASM
{
    class ExeObject : public AssembledObject
    {
    public:
        struct MzHeader
        {
            uint8_t mzSignature[2] = { 'M','Z' };
            uint16_t bytesInTheLastPage = 0;
            uint16_t fileSizeInPages = 0;
            uint16_t numberOfRelocationTargets = 0;
            uint16_t headerSizeInParagraphs = 0;

            uint16_t minAllocatedParagprahs = 0;
            uint16_t maxAllocatedParagraphs = std::numeric_limits<uint16_t>::max();

            uint16_t initialRelativeSS = 0;
            uint16_t initialSp = 0;

            uint16_t checksum = 0;

            uint16_t initialIp = 0;
            uint16_t initialRelativeCS = 0;

            uint16_t relocationTableOffset = sizeof(MzHeader);
            uint16_t overlayNumber = 0;
        };

        struct RelocationTarget
        {
            uint16_t offset = 0;
            uint16_t segment = 0;
        };
    private:
        static constexpr size_t paragraphByteSize = 16;
        static constexpr size_t pageByteSize = 256;

        mutable MzHeader mzHeader;
        std::vector<RelocationTarget> relocationTable;

        Codegen::MachineCode code;

        friend class Linker;
    public:
        bool Deserialize(std::istream& stream) override;
        bool Serialize(std::ostream& stream) const override;
    };
}

#endif