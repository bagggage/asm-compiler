#ifndef __TOKEN_H
#define __TOKEN_H

#include <string>
#include <cstdint>
#include <memory>
#include <cassert>

#include "context/source-location.h"

#include "arch/8086/regs.h"

namespace ASM
{
    class Lexer;

    struct TokenData 
    {
        virtual ~TokenData() = default;
    };

    struct TokenString final : public TokenData
    {
    private:
        std::string value;
    public:
        TokenString(const char* const string, size_t length) : value(string, length) {}
        TokenString(const std::string& string) : value(string) {}

        inline const std::string& GetValue() const { return value; }
    };

    struct TokenNumeric final : public TokenData
    {
    private:
        double value = 0;
    public:
        TokenNumeric(double val) : value(val) {}
        TokenNumeric(Arch::RegisterIdentifier reg) : value(static_cast<uint8_t>(reg)) {}

        inline double GetValue() const { return value; }
        inline int64_t GetNum() const { return  static_cast<int64_t>(value); }
        inline char GetAscii() const { return  static_cast<char>(value); }
        inline Arch::RegisterIdentifier GetRegId() const { return static_cast<Arch::RegisterIdentifier>(value); }
    };

    struct Token
    {
        enum class Kind : uint8_t
        {
            eof,
            identifier,
            string_literal,
            char_constant,
            num_constant,
            reg,

            kw_global,
            kw_extern,
            kw_org,
            kw_section,
            kw_segment,
            kw_offset,
            kw_align,
            kw_dup,
            kw_equ,

            l_square,
            r_square,
            l_paren,
            r_paren,
            comma,
            colon,

            minus,
            tilda,
            plus,
            slash,
            star,
            caret,
            pipe,
            amp,
            lessless,
            greatgreat,

            question,
            at,
            dolar,
            dolardolar,

            unknown
        };

    private:
        SourceLocation location;
        unsigned int length = 0;

        std::unique_ptr<TokenData> data = nullptr;

        Kind kind = Kind::unknown;

        friend class Lexer;
    public:
        inline Kind GetKind() const { return kind; }
        inline unsigned int GetLength() const { return length; }
        inline const SourceLocation& GetLocation() const { return location; }

        inline bool Is(Kind intendentKind) const { return (kind == intendentKind); }
        inline bool IsKeyword() const { return (kind >= Kind::kw_global && kind < Kind::l_square); }
        inline bool IsBinaryOperator() const { return (kind >= Kind::plus && kind <= Kind::greatgreat); }
        inline bool IsUnaryOperator() const { return (kind >= Kind::minus && kind <= Kind::tilda); }
        inline bool IsSameLine(const Token& other) const { return (location.line == other.location.line); }

        inline TokenString* GetAsString() const
        { 
            assert(dynamic_cast<TokenString*>(data.get()));
            return reinterpret_cast<TokenString*>(data.get());
        }

        inline TokenNumeric* GetAsNum() const
        {
            assert(dynamic_cast<TokenNumeric*>(data.get()));
            return reinterpret_cast<TokenNumeric*>(data.get());
        }
    };

    using TokKind = Token::Kind;
}

#endif