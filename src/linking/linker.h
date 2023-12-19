#ifndef __ASM_LINKER_H
#define __ASM_LINKER_H

#include "assembled-object.h"
#include "context/context.h"
#include "raw-binary.h"

namespace ASM
{
    enum class LinkingFormat
    {
        RawBinary,
        DosExecutable,
        WinExecutable,
        Elf
    };

    class Linker
    {
    private:
        AssemblyContext* context = nullptr;
        std::unordered_map<std::string, int64_t> symbolMap;
        std::vector<Section*> sectionOrder;

        void EvaluateSymbol(const Symbol& symbol, unsigned int depth = 0);
        void LinkRawBinary(RawBinary& result);

        static constexpr const size_t maxEvalDepth = 1000;
        static const std::unordered_map<std::string, unsigned int> segmentsPriorityMap;
    public:
        Linker(AssemblyContext& context) : context(&context) {}

        std::unique_ptr<AssembledObject> Link(LinkingFormat format);
    };
}

#endif