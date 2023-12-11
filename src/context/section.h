#ifndef __ASM_SECTION_H
#define __ASM_SECTION_H

#include "codegen/machine-code.h"
#include "linking/linking-targets.h"

#include "syntax/declarations.h"

namespace ASM
{
    class Section
    {
    private:
        Codegen::MachineCode code;
        std::vector<LinkingTarget> linkingTargets;
    public:
        inline Codegen::MachineCode& GetCode() { return code; }
        inline std::vector<LinkingTarget>& GetLinkingTargets() { return linkingTargets; }
    };
}

#endif