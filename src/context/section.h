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
        std::string name;

        Codegen::MachineCode code;
        std::vector<LinkingTarget> linkingTargets;
    public:
        Section() = default;
        Section(const std::string& name) : name(name) {}

        inline const std::string& GetName() const { return name; }

        inline Codegen::MachineCode& GetCode() { return code; }
        inline std::vector<LinkingTarget>& GetLinkingTargets() { return linkingTargets; }
    };
}

#endif