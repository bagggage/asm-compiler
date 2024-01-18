#ifndef __ASM_OBJECT_FILE_H
#define __ASM_OBJECT_FILE_H

#include "assembled-object.h"
#include "context/context.h"

namespace ASM
{
    class ObjectFile : public AssembledObject
    {
    private:
        AssemblyContext* context = nullptr;
    public:
        ObjectFile(AssemblyContext& context) : context(&context) {}

        bool Deserialize(std::istream& stream) override;
        bool Serialize(std::ostream& stream) const override;
    };
}

#endif