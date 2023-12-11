#ifndef __ASM_RAW_BINARY_OBJECT_H
#define __ASM_RAW_BINARY_OBJECT_H

#include "assembled-object.h"
#include "codegen/code-generator.h"

namespace ASM
{
    class RawBinary : public AssembledObject
    {
    private:
        Codegen::MachineCode code;
    public:
        bool Deserialize(std::istream& stream) override;
        bool Serialize(std::ostream& stream) const override;

        inline bool IsEmpty() const { return code->empty(); }
        
        inline Codegen::MachineCode& GetCode() { return code; }
        inline const Codegen::MachineCode& GetCode() const { return code; }
    };
}

#endif