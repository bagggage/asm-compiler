#ifndef __ASM_SYMBOL_H
#define __ASM_SYMBOL_H

#include <cstdint>

#include "syntax/declarations.h"

namespace ASM
{
    class SymbolValue
    {
    public:
        enum class Kind : uint8_t
        {
            Address,
            Literal
        };
    protected:
        Kind kind = Kind::Literal;

        double absoluteValue = 0;
    public:
        SymbolValue() = default;
        SymbolValue(Kind kind, double value) : kind(kind), absoluteValue(value) {}

        uint8_t GetRequiredSize() const;
        inline Kind GetKind() const { return kind; }

        inline int64_t GetAsInt() const { return absoluteValue; }
        inline double GetAsFloat() const { return absoluteValue; }

        inline bool Is(Kind intendentKind) const { return intendentKind == kind; }
    };

    class Symbol
    {
    private:
        const AST::SymbolDecl* declaration = nullptr;
        SymbolValue value;

        bool isEvaluated = false;
    public:
        Symbol() = default;
        Symbol(const AST::SymbolDecl* declaration) : declaration(declaration){}
        Symbol(const AST::SymbolDecl* declaration, SymbolValue value)
            : declaration(declaration), value(value), isEvaluated(true) {}

        inline const AST::SymbolDecl& GetDeclaration() const { return *declaration; }
        inline const SymbolValue& GetValue() const { return value; }

        inline bool IsEvaluated() const { return isEvaluated; }

        inline void Evaluate(SymbolValue& value) { this->value = value; isEvaluated = true; }
        inline void Evaluate(SymbolValue&& value) { this->value = value; isEvaluated = true; }
    };
}

#endif