#ifndef __LEXER_H
#define __LEXER_H

#include <sstream>
#include <unordered_map>

#include "context/context.h"
#include "token.h"

namespace ASM
{
    class Lexer
    {
    private:
        AssemblyContext* context = nullptr;
        SourceLocation cursor;

        static constexpr char commentSym = ';';

        bool IsValid() const;

        void TokenReinit(Token& token);

        inline void Next() { if (*(cursor++) == '\n') ++cursor.line; }

        void SkipLine();

        TokKind LexSpecificSymbol(Token& result);
        TokKind LexKeyword(Token& result);
        TokKind LexIdentifierOrLiteral(Token& result);
    public:
        Lexer(AssemblyContext& context);

        static const std::unordered_map<char, TokKind> SpecialSymbolsToKind;
        static const std::unordered_map<std::string, TokKind> KeywordsToKind;
        static const std::unordered_map<std::string, Arch::RegisterIdentifier> IdentifierToRegId;

        bool GetNextToken(Token& result);
    };
}

#endif