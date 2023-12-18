#ifndef __ASM_ARCH_8086_H
#define __ASM_ARCH_8086_H

#include <unordered_map>

#include "instruction.h"

namespace ASM::Arch
{
    class Arch8086
    {
    public:
        static const std::unordered_map<std::string, std::vector<Instruction>> InstructionSet;
        static const std::unordered_map<std::vector<RegisterIdentifier>, RM> RmRegsCombinations;
        static const std::unordered_map<std::string, uint8_t> DefineDataMnemonics;
        static const std::unordered_map<std::string, uint8_t> ReserveDataMnemonics;
        static const std::unordered_map<RegisterIdentifier, InstructionPrefix> SregToSegOverride;

        static inline bool HasMnemonic(const std::string& intendentMnemonic) { return (InstructionSet.count(intendentMnemonic) > 0); }
    };
}

#endif