#ifndef __ASM_INSTRUCTION_H
#define __ASM_INSTRUCTION_H

#include <initializer_list>
#include <vector>
#include <string>
#include <unordered_map>
#include <cassert>

#include "regs.h"
#include "encoding.h"

#include "utils/small-vector.h"

namespace ASM::Arch
{
    struct Operand
    {
    public:
        enum class Type : uint8_t
        {
            none,

            r,
            m,
            rm,
            imm,
            creg,
            sreg,
            moffs,
            rel,
            ptr,

            AL,
            AX,
            EAX,
            DX,
            CL,
            
            CS,
            DS,
            ES,
            SS,
            FS,
            GS,

            ONE
        };

        Type type = Type::none;
        uint8_t size = 0;
    };

    typedef OperandEncoding OpEn;
    typedef Operand::Type OpType;

    struct OperandEvaluation
    {
    public:
        enum class Kind : uint8_t
        {
            Register,
            Immediate,
            Memory
        };

        enum class Sign : uint8_t
        {
            none,

            Unsigned,
            Signed
        };

        OperandEvaluation() = default;
        OperandEvaluation(Kind kind) : kind(kind) {}

        Kind kind;

        SmallVector<OpType, 4> expectedTypes;
        int64_t approximateValue = 0;
        uint8_t minRequiredSize = 0;

        Sign sign;

        inline bool Is(Kind expectedKind) const { return kind == expectedKind; }
    };

    enum class SpecialFeature : uint8_t
    {
        none,

        SignExtended
    };

    struct Instruction
    {
    public:
        Instruction() = default;
        constexpr Instruction
        (
            const std::initializer_list<uint8_t> opcodes,
            const OperandEncoding operandEncoding,
            const std::initializer_list<Operand> operands,
            uint8_t opcodeExtention = opcodeExtentionNone,
            SpecialFeature feature = SpecialFeature::none
        )
            : opencode(operandEncoding),
            opcode(opcodes),
            operands(operands),
            opcodeExtention(opcodeExtention),
            feature(feature)
        {}

        static constexpr const uint8_t opcodeExtentionNone = 0xff;

        SmallVector<uint8_t, 3> opcode;
        uint8_t opcodeExtention = opcodeExtentionNone;

        SpecialFeature feature = SpecialFeature::none;

        OperandEncoding opencode = OperandEncoding::ZO;
        SmallVector<Operand, 4> operands;

        size_t GetMaxByteSize() const;
    };
}

namespace std
{
    template<>
    struct hash<std::vector<ASM::Arch::RegisterIdentifier>>
    {
        size_t operator()(const std::vector<ASM::Arch::RegisterIdentifier>& vec) const 
        {
            size_t seed = vec.size();

            for(auto& i : vec) 
            {
              seed ^= (uint8_t)i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }

            return seed;
        }
    };
}

#endif