#ifndef __ASM_REGS_H
#define __ASM_REGS_H

#include <cstdint>

namespace ASM::Arch
{
    enum class RegisterIdentifier : uint8_t
    {
        //8-bit registers
        AL, 
        CL,
        DL,
        BL,
        AH,
        CH,
        DH,
        BH,
   
        //16-bit registers
        AX, 
        CX,
        DX,
        BX,
        SP,
        BP,
        SI,
        DI,
    
        //32-bit registers
        EAX,
        ECX,
        EDX,
        EBX,
        ESP,
        EBP,
        ESI,
        EDI,
    
        //64-bit registers
        RAX,
        RCX,
        RDX,
        RBX,
        RSP,
        RBP,
        RSI,
        RDI,
    
        //SIMD extention 80-bit registers
        MM0, 
        MM1,
        MM2,
        MM3,
        MM4,
        MM5,
        MM6,
        MM7,
    
        //SIMD extention 128-bit registers
        XMM0,
        XMM1,
        XMM2,
        XMM3,
        XMM4,
        XMM5,
        XMM6,
        XMM7,
    
        //Segment registers
        ES,
        CS,
        SS,
        DS,
        FS,
        GS,

        //Constrol registers
        CR0,
        CR1,
        CR2,
        CR3,
        CR4,
        CR5,
        CR6,
        CR7
    };

    enum class RegisterGroup
    {
        GeneralPerpose,
        Segment,
        Control,
        MMX,
        Extented64,
        Unknown
    };
}

#endif