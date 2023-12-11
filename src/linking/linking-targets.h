#ifndef __ASM_LINKING_TARGET_H
#define __ASM_LINKING_TARGET_H

#include <unordered_map>

#include "codegen/machine-code.h"
#include "syntax/expressions.h"

namespace ASM
{
    class LinkingTarget
    {
    public:
        enum class Kind : uint8_t
        {
            Value,
            AbsoluteAddress,
            RelativeAddress
        };

        enum class Type : uint8_t
        {
            Integer,
            Float
        };
    private:
        Kind dependency = Kind::Value;
        Type type = Type::Integer;

        uint8_t size = 0;

        uint64_t replacmentRelativeOrigin = 0;
        uint64_t replacmentSectionOffset = 0;

        AST::Expression* expression = nullptr;
    public:
        LinkingTarget(AST::Expression* expression, uint64_t offset, uint8_t size, uint64_t relativeOrigin)
            : replacmentSectionOffset(offset),
            expression(expression),
            dependency(Kind::RelativeAddress), size(size),
            replacmentRelativeOrigin(relativeOrigin),
            type(Type::Integer) {}

        LinkingTarget(AST::Expression* expression, Kind dependency, uint64_t offset, uint8_t size, Type type = Type::Integer) 
            : replacmentSectionOffset(offset), expression(expression), dependency(dependency), size(size), type(type)
        {
            assert(dependency == Kind::Value || type == Type::Integer);
        }

        inline Kind GetKind() const { return dependency; }
        inline Type GetType() const { return type; }
        inline uint8_t GetSize() const { return size; }
        inline uint64_t GetSectionOffset() const { return replacmentSectionOffset; }
        inline uint64_t GetRelativeOrigin() const { return replacmentRelativeOrigin; }
        inline AST::Expression* GetExpression() { return expression; }
        inline const AST::Expression* GetExpression() const { return expression; }
    };
}

#endif