#ifndef __ASM_ASSEMBLED_OBJECT_H
#define __ASM_ASSEMBLED_OBJECT_H

#include <istream>

namespace ASM
{
    class AssembledObject
    {
    public:
        virtual bool Deserialize(std::istream& stream) = 0;
        virtual bool Serialize(std::ostream& stream) const = 0;

        virtual ~AssembledObject() = default;
    };
}

#endif