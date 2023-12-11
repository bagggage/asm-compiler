#ifndef __ASM_ENCODING_H
#define __ASM_ENCODING_H

#include <cstdint>

namespace ASM::Arch
{
    enum class Register : uint8_t
    {
        EAX = 0x0,
        AX = EAX, 
        AL = EAX, 
        MM0 = EAX, 
        XMM0 = EAX,

        ECX = 0x1,
        CX = ECX,
        CL = ECX,
        MM1 = ECX,
        XMM1 = ECX,

        EDX = 0x2,
        DX = EDX,
        DL = EDX,
        MM2 = EDX,
        XMM2 = EDX,

        EBX = 0x3,
        BX = EBX,
        BL = EBX,
        MM3 = EBX,
        XMM3 = EBX,

        ESP = 0x4,
        SP = ESP,
        AH = ESP,
        MM4 = ESP,
        XMM4 = ESP,

        EBP = 0x5,
        BP = EBP,
        CH = EBP,
        MM5 = EBP,
        XMM5 =EBP,

        ESI = 0x6,
        SI = ESI,
        DH = ESI,
        MM6 = ESI,
        XMM6 = ESI,

        EDI = 0x7,
        DI = EDI,
        BH = EDI,
        MM7 = EDI,
        XMM7 = EDI,

        ES = 0x0,
        CS = 0x1,
        SS = 0x2,
        DS = 0x6,

        CR0 = 0x0,
        CR1 = 0x1,
        CR2 = 0x2,
        CR3 = 0x3,
        CR4 = 0x4,
        CR5 = 0x5,
        CR6 = 0x6,
        CR7 = 0x7,

        Invalid = 0xff
    };

    enum class Mod : uint8_t
    {
        MEM     = 0x0,
        MEM8    = 0x1,
        MEM_EX  = 0x2,
        REG     = 0x3
    };

    enum class RM : uint8_t
    {
        BX_SI = 0x0,
        BX_DI = 0x1,
        BP_SI = 0x2,
        BP_DI = 0x3,
        SI    = 0x4,
        DI    = 0x5,
        BP    = 0x6,
        BX    = 0x7,

        DISP_16 = 0x6
    };

    enum class OperandEncoding : uint8_t
    {
        ZO,
        RM,
        RMI,
        MR,
        MI,
        FD,
        TD,
        OI,
        D,
        I,
        M,
        O,
        S,
        M1,
        MC
    };

    enum class InstructionPrefix : uint8_t
    {
        LOCK = 0xf0,

        REPNE = 0xf2,
        REPNZ = 0xf2,

        REP = 0xf3,
        REPE = 0xf3,
        REPZ = 0xf3,

        BND = 0xf2,

        CS = 0x2e,
        SS = 0x36,
        DS = 0x3e,
        ES = 0x26,
        FS = 0x64,
        GS = 0x65,

        JCC_BT = 0x2e,
        JCC_BNT =  0x3e,

        OPSIZE = 0x66,
        ADDRSIZE = 0x67
    };

    struct __attribute__((packed)) ModRM
    {
        RM       rm  : 3 = RM::BX_SI;
        Register reg : 3 = Register::AL;
        Mod      mod : 2 = Mod::MEM;

        inline operator uint8_t() const
        {
            return *(reinterpret_cast<const uint8_t*>(this));
        }
    };
}

#endif