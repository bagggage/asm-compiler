#include "lexer.h"

using namespace ASM;

#define SS_TO_KIND(x, y) { x,  TokKind::y }
#define KW_TO_KIND(x, y) { x,  TokKind::kw_ ## y }
#define ID_TO_REG(x)     { #x, Arch::RegisterIdentifier::x }

const std::unordered_map<char, TokKind> Lexer::SpecialSymbolsToKind = 
{
    SS_TO_KIND(',', comma),
    SS_TO_KIND(':', colon),
    SS_TO_KIND('[', l_square),
    SS_TO_KIND(']', r_square),
    SS_TO_KIND('(', l_paren),
    SS_TO_KIND(')', r_paren),
    SS_TO_KIND('-', minus),
    SS_TO_KIND('~', tilda),
    SS_TO_KIND('+', plus),
    SS_TO_KIND('*', star),
    SS_TO_KIND('/', slash),
    SS_TO_KIND('?', question),
    SS_TO_KIND('|', pipe),
    SS_TO_KIND('&', amp),
    SS_TO_KIND('^', caret),
    SS_TO_KIND('@', at),
    SS_TO_KIND('$', dolar),
    SS_TO_KIND('<', lessless),
    SS_TO_KIND('>', greatgreat),
};

const std::unordered_map<std::string, TokKind> Lexer::KeywordsToKind =
{
    KW_TO_KIND("SECTION", section),
    KW_TO_KIND("SEGMENT", segment),
    KW_TO_KIND("OFFSET",  offset),
    KW_TO_KIND("ORG",     org),
    KW_TO_KIND("GLOBAL",  global),
    KW_TO_KIND("EXTERN",  extern),
    KW_TO_KIND("ALIGN",   align),
    KW_TO_KIND("DUP",     dup),
    KW_TO_KIND("EQU",     equ),
};

const std::unordered_map<std::string, Arch::RegisterIdentifier> Lexer::IdentifierToRegId = 
{
    ID_TO_REG(AL),
    ID_TO_REG(AH),
    ID_TO_REG(AX),
    ID_TO_REG(CL),
    ID_TO_REG(CH),
    ID_TO_REG(CX),
    ID_TO_REG(DL),
    ID_TO_REG(DH),
    ID_TO_REG(DX),
    ID_TO_REG(BL),
    ID_TO_REG(BH),
    ID_TO_REG(BX),
    ID_TO_REG(SI),
    ID_TO_REG(DI),
    ID_TO_REG(BP),
    ID_TO_REG(SP),

    ID_TO_REG(EAX),
    ID_TO_REG(ECX),
    ID_TO_REG(EDX),
    ID_TO_REG(EBX),
    ID_TO_REG(ESI),
    ID_TO_REG(EDI),
    ID_TO_REG(EBP),
    ID_TO_REG(ESP),

    ID_TO_REG(RAX),
    ID_TO_REG(RCX),
    ID_TO_REG(RDX),
    ID_TO_REG(RBX),
    ID_TO_REG(RSI),
    ID_TO_REG(RDI),
    ID_TO_REG(RBP),
    ID_TO_REG(RSP),

    ID_TO_REG(MM0),
    ID_TO_REG(MM1),
    ID_TO_REG(MM2),
    ID_TO_REG(MM3),
    ID_TO_REG(MM4),
    ID_TO_REG(MM5),
    ID_TO_REG(MM6),
    ID_TO_REG(MM7),

    ID_TO_REG(XMM0),
    ID_TO_REG(XMM1),
    ID_TO_REG(XMM2),
    ID_TO_REG(XMM3),
    ID_TO_REG(XMM4),
    ID_TO_REG(XMM5),
    ID_TO_REG(XMM6),
    ID_TO_REG(XMM7),

    ID_TO_REG(ES),
    ID_TO_REG(CS),
    ID_TO_REG(SS),
    ID_TO_REG(DS),
    ID_TO_REG(FS),
    ID_TO_REG(GS),

    ID_TO_REG(CR0),
    ID_TO_REG(CR1),
    ID_TO_REG(CR2),
    ID_TO_REG(CR3),
    ID_TO_REG(CR4),
    ID_TO_REG(CR5),
    ID_TO_REG(CR6),
    ID_TO_REG(CR7)
};

#undef SS_TO_KIND
#undef KW_TO_KIND
#undef ID_TO_REG

Lexer::Lexer(AssemblyContext& context) : context(&context)
{
    cursor = context.GetSource().data();
    cursor.line = 0;
}

void Lexer::TokenReinit(Token& token)
{
    token.data.reset();
    token.kind = TokKind::unknown;
    token.length = 0;
    token.location = cursor;
}

bool Lexer::IsValid() const
{
    return (cursor != nullptr);
}

void Lexer::SkipLine()
{
    while (*cursor != '\n' && *cursor != '\0')
        ++cursor;

    if (*cursor == '\n')
    {
        ++cursor.line;
        ++cursor;
    }
}

TokKind Lexer::LexSpecificSymbol(Token& result)
{
    if (SpecialSymbolsToKind.count(*cursor) == 0)
        return TokKind::unknown;

    TokKind& kind = result.kind;

    kind = SpecialSymbolsToKind.at(*cursor);

    switch (kind)
    {
    case TokKind::lessless:
        result.length = 2;

        if (*(++cursor) != '<')
            kind = TokKind::unknown;
        break;
    case TokKind::greatgreat:
        result.length = 2;

        if (*(++cursor) != '>')
            kind = TokKind::unknown;
        break;
    case TokKind::dolar:
        if (*(cursor + 1) == '$')
        {
            cursor++;

            result.length = 2;
            kind = TokKind::dolardolar;
        }

        break;
    default:
        result.length = 1;
        break;
    }

    if (kind == TokKind::unknown)
        result.length = 1;
    else
        ++cursor;

    return kind;
}

TokKind Lexer::LexKeyword(Token& result)
{
    const std::string& value = reinterpret_cast<TokenString*>(result.data.get())->GetValue();

    if (KeywordsToKind.count(value) == 0)
        return TokKind::unknown;

    result.kind = KeywordsToKind.at(value);
    result.data.reset();

    return result.kind;
}

TokKind Lexer::LexIdentifierOrLiteral(Token& result)
{
    //Literal
    if (*cursor == '\'')
    {
        char asciiChar = *(++cursor);

        if (asciiChar == '\0' || *(++cursor) != '\'')
        {
            result.length = cursor - result.location;
            return TokKind::unknown;
        }
        
        ++cursor;
        
        result.kind = TokKind::char_constant;
        result.length = 3;
        result.data = std::make_unique<TokenNumeric>(asciiChar);

        return TokKind::char_constant;
    }
    else if (*cursor == '\"')
    {
        do
        {
            ++cursor;
        } while(*cursor != '\"' && *cursor != '\0' && *cursor != '\n');

        if (*cursor != '\"')
            return TokKind::unknown;

        ++cursor;

        result.kind = TokKind::string_literal;
        result.length = cursor - result.location;
        result.data = std::make_unique<TokenString>(result.location + 1, result.length - 2);

        return TokKind::string_literal;
    }

    //Identifier
    while (std::isalnum(*cursor) || *cursor == '.' || *cursor == '_')
        ++cursor;

    result.length = cursor - result.location;

    if (result.length == 0)
        return TokKind::unknown;

    std::string value(result.location, result.length);

    for (auto& c : value)
        c = std::toupper(c);

    try
    {
        double numericValue = std::stold(value);

        result.kind = TokKind::num_constant;
        result.data = std::make_unique<TokenNumeric>(numericValue);
    }
    catch (std::invalid_argument& e)
    {
        if (IdentifierToRegId.count(value) == 0)
        {
            result.kind = TokKind::identifier;
            result.data = std::make_unique<TokenString>(std::move(value));

            return TokKind::identifier;
        }

        result.kind = TokKind::reg;
        result.data = std::make_unique<TokenNumeric>(IdentifierToRegId.at(value));
    }
    catch (std::out_of_range& e)
    {
        return TokKind::unknown;
    }

    return result.kind;
}

bool Lexer::GetNextToken(Token& result)
{
    assert(IsValid());

    while (std::isspace(*cursor))
        Next();

    while (*cursor == commentSym)
    {
        SkipLine();
        
        return GetNextToken(result);
    }

    TokenReinit(result);

    if (*cursor == '\0')
    {
        result.kind = TokKind::eof;
        return false;
    }

    if (LexSpecificSymbol(result) != TokKind::unknown || result.length > 0)
        return true;

    if (LexIdentifierOrLiteral(result) != TokKind::unknown)
    {
        if (result.Is(TokKind::identifier))
            LexKeyword(result);
    }
    else
    {
        while (std::isspace(*cursor) == false && *cursor != '\0')
            Next();

        return false;
    }

    return true;
}