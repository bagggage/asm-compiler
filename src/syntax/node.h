#ifndef __AST_NODE_H
#define __AST_NODE_H

#include <type_traits>

#include "context/source-location.h"

namespace ASM { class Parser; }

namespace ASM::AST
{
    struct Node
    {
    protected:
        SourceLocation location;
        unsigned int length = 0;

        friend class ASM::Parser;
    public:
        template<typename T>
        bool Is() const noexcept
        {
            static_assert(std::is_base_of<Node, T>::value);
            return (dynamic_cast<const T*>(this) != nullptr);
        }

        template<typename T>
        inline T* GetAs()
        { static_assert(std::is_base_of<Node, T>::value); return reinterpret_cast<T*>(this); }

        template<typename T>
        inline const T* GetAs() const
        { static_assert(std::is_base_of<Node, T>::value); return reinterpret_cast<const T*>(this); }

        inline const SourceLocation& GetLocation() const { return location; }
        inline unsigned int GetLength() const { return length; }

        virtual ~Node() = default;
    };
}

#endif