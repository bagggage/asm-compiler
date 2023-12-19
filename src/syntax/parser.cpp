#include "parser.h"

#include <iostream>

#include "arch/arch.h"

using namespace ASM;
using namespace ASM::AST;

const std::unordered_map<char, uint8_t> Parser::operatorPriorities =
{
    {'|', 0},
    {'^', 1},
    {'&', 2},
    {'>', 3},
    {'<', 3},
    {'+', 4},
    {'-', 4},
    {'*', 5},
    {'/', 5},
    {'~', 6}
};

const std::unordered_map<TokKind, char> Parser::kindOperatorToChar = 
{
    {TokKind::minus,      '-'},
    {TokKind::plus,       '+'},
    {TokKind::slash,      '/'},
    {TokKind::star,       '*'},
    {TokKind::tilda,      '~'},
    {TokKind::amp,        '&'},
    {TokKind::pipe,       '|'},
    {TokKind::caret,      '^'},
    {TokKind::greatgreat, '>'},
    {TokKind::lessless,   '<'}
};

Token& Parser::NextToken()
{
    if (tokenStream.empty() == false && tokenStream.front().Is(TokKind::eof))
        return tokenStream.front();

    if (context->IsCurrentMode(AssemblyMode::Direct))
    {
        tokenStream.pop_front();

        return tokenStream.front();
    }

    uint64_t i = 0;

    while (tokenStream.size() < 2);

    tokenStreamMutex.lock();
    tokenStream.pop_front();
    tokenStreamMutex.unlock();

    //Debug
    //std::cout << "Tok: " << std::string(tokenStream.front().GetLocation().sourcePointer, tokenStream.front().GetLength()) << std::endl;

    return tokenStream.front();
}

Token& Parser::LookAhead()
{
    if (tokenStream.empty() == false && tokenStream.front().Is(TokKind::eof))
        return tokenStream.front();

    if (context->IsCurrentMode(AssemblyMode::Direct))
        return *(++tokenStream.begin());
    
    //wait for lexer (async)
    while (tokenStream.size() < 2);

    return *(++tokenStream.begin());
}

bool Parser::HasNextToken() const
{
    if (tokenStream.empty() == false && tokenStream.front().Is(TokKind::eof))
        return false;

    return (context->IsCurrentMode(AssemblyMode::Direct) && tokenStream.empty() == false);
}

void Parser::PushToken(Token&& token)
{
    if (context->IsCurrentMode(AssemblyMode::Direct))
    {
        tokenStream.push_back(std::move(token));    
        return;
    }

    tokenStreamMutex.lock();
    tokenStream.push_back(std::move(token));
    tokenStreamMutex.unlock();
}

AbstractSyntaxTree Parser::Parse()
{
    AbstractSyntaxTree result;

    result.push_back(std::make_unique<SectionDecl>(context->UnnamedSection.data()));
    currentSection = result.back()->GetAs<SectionDecl>();

    if (tokenStream.empty())
    {
        if (context->IsCurrentMode(AssemblyMode::Parallel)) {
            while(tokenStream.empty());
        }
        else {
            return std::move(result);
        }
    }
  
    Token* token = &tokenStream.front();

    while (token->Is(TokKind::eof) == false)
    {
        bool success = false;
        bool isChanged = true;

        try 
        {
            switch (token->GetKind())
            {
            case TokKind::identifier:
            {
                Token& nextTok = LookAhead();

                bool nextTokSameLine = token->GetLocation().line == nextTok.GetLocation().line;
                bool hasMnemonicWithSuchName = Arch::Arch8086::HasMnemonic(token->GetAsString()->GetValue());

                if (nextTokSameLine && nextTok.Is(TokKind::colon))
                {
                    if (hasMnemonicWithSuchName)
                        context->Warn("Lable has the same name as mnemonic", token->GetLocation(), token->GetLength());

                    result.push_back(std::make_unique<LableDecl>("", currentSection));
                    success = ParseLableDecl(*reinterpret_cast<LableDecl*>(result.back().get()));

                    break;  
                }
                else if (nextTokSameLine && nextTok.Is(TokKind::kw_equ))   
                {   
                    if (hasMnemonicWithSuchName)
                        context->Warn("Constant has the same name as mnemonic", token->GetLocation(), token->GetLength( ));

                    result.push_back(std::make_unique<ConstantDecl>("", nullptr));
                    success = ParseConstantDecl(*reinterpret_cast<ConstantDecl*>(result.back().get()));

                    break;
                }
                else if (hasMnemonicWithSuchName)
                {
                    result.push_back(std::make_unique<InstructionStmt>());
                    success = ParseInstructionStmt(*reinterpret_cast<InstructionStmt*>(result.back().get()));

                    break;
                }
                else if (Arch::Arch8086::DefineDataMnemonics.count(token->GetAsString()->GetValue()) > 0)
                {
                    result.push_back(std::make_unique<DefineDataStmt>());
                    success = ParseDefineDataStmt(*reinterpret_cast<DefineDataStmt*>(result.back().get()));

                    break;
                }

                context->Error("Unknown identifier", token->GetLocation(), token->GetLength());
                isChanged = false;

                break;
            }
            case TokKind::kw_section: case TokKind::kw_segment:
            {
                result.push_back(std::make_unique<SectionDecl>(""));
                success = ParseSectionDecl(*reinterpret_cast<SectionDecl*>(result.back().get()));

                currentSection = result.back()->GetAs<SectionDecl>();

                break;
            }
            case TokKind::kw_extern: case TokKind::kw_global:
            {
                result.push_back(std::make_unique<SymbolDecl>(""));
                success = ParseSymbolDecl(*reinterpret_cast<SymbolDecl*>(result.back().get()));

                break;
            }
            case TokKind::kw_align:
            {
                result.push_back(std::make_unique<AlignStmt>());
                success = ParseParametricStmt(*result.back()->GetAs<ParametricStmt>());

                break;
            }
            case TokKind::kw_offset:
            {
                result.push_back(std::make_unique<OffsetStmt>());
                success = ParseParametricStmt(*result.back()->GetAs<ParametricStmt>());

                break;
            }
            case TokKind::kw_org: 
            {
                result.push_back(std::make_unique<OrgStmt>());
                success = ParseParametricStmt(*result.back()->GetAs<ParametricStmt>());

                break;
            }
            default:
            {
                context->Error("Unknown syntax", token->GetLocation());

                unsigned int currentLine = token->GetLocation().line;
                Token* next = &LookAhead();

                while (currentLine == next->GetLocation().line && next->Is(TokKind::eof) == false)
                {
                    NextToken();
                    next = &LookAhead();
                }

                isChanged = false;

                break;
            }
            }
        }
        catch (std::exception& e) 
        {
            context->Error((std::string("Exception occured while parsing AST: ") + e.what()).c_str());

            success = false;
        }

        if (!success && isChanged)
            result.pop_back();
        else if (result.back()->Is<Statement>())
            currentStmtOffset += result.back()->GetAs<Statement>()->GetMaxStmtByteSize();

        token = &NextToken();
    }

    return std::move(result);
}

bool Parser::ParsePrimary(Expression*& result)
{
    if (ParseExpression(result) == false)
        return false;

    Token* token = &LookAhead();
    uint8_t operatorPriority = std::numeric_limits<uint8_t>::max();

    while (token->GetLocation().line == tokenStream.front().GetLocation().line &&
        (token->IsBinaryOperator() || token->IsUnaryOperator()))
    {
        if (token->IsUnaryOperator() && token->Is(TokKind::minus) == false)
        {
            context->Error("Invalid expression, expected binary operator between", tokenStream.front().GetLocation());
            return false;
        }

        token = &NextToken();

        uint8_t currentOperatorPriority = operatorPriorities.at(kindOperatorToChar.at(token->GetKind()));

        Token* next = &LookAhead();

        if (token->GetLocation().line != next->GetLocation().line ||
            next->IsKeyword() || next->IsBinaryOperator() ||
            next->IsUnaryOperator())
        {
            delete result;
            context->Error("Invalid binary expression", token->GetLocation());

            return false;
        }

        if (operatorPriority < currentOperatorPriority)
        {
            BinaryExpr* currentBinaryExpr = reinterpret_cast<BinaryExpr*>(result);
            Expression* currentRhs = currentBinaryExpr->rhs.release();
            BinaryExpr* binaryExpr = new BinaryExpr();

            if (ParseBinaryExpr(*binaryExpr, currentRhs) == false)
            {
                delete result;
                delete currentRhs;
                delete binaryExpr;

                return false;
            }

            currentBinaryExpr->rhs.reset(binaryExpr);
        }
        else
        {
            BinaryExpr* binaryExpr = new BinaryExpr();

            if (ParseBinaryExpr(*binaryExpr, result) == false)
            {
                delete binaryExpr;
                delete result;

                return false;
            }

            result = binaryExpr;
        }

        operatorPriority = currentOperatorPriority;
        token = &LookAhead();
    }

    return true;
}

bool Parser::ParseExpression(Expression*& result)
{
    Token* firstToken = &tokenStream.front();

    //used only when parsing memory expression
    uint8_t memExprSizeOverride = 0;
    RegisterExpr* memExprSegOverride = nullptr;

    switch (firstToken->GetKind())
    {
    case Token::Kind::reg:
    {
    parse_reg_expr:
        result = new RegisterExpr(firstToken->GetAsNum()->GetRegId());
        result->location = firstToken->GetLocation();
        result->length = firstToken->GetLength();

        Token& next = LookAhead();

        if (next.IsSameLine(*firstToken) && next.Is(TokKind::colon) &&
            result->GetAs<RegisterExpr>()->GetGroup() == Arch::RegisterGroup::Segment)
        {
            memExprSegOverride = result->GetAs<RegisterExpr>();

            //Skip colon
            NextToken();
            firstToken = &NextToken();

            if (firstToken->Is(TokKind::l_square) == false) [[unlikely]]
            {
                context->Error("Memory expression expected after segment register override", result->location);
                delete result;

                return false;
            }

            goto parse_mem_expr;
        }

        break;
    }
    case Token::Kind::num_constant: case Token::Kind::char_constant:
    {
        result = new NumberExpr(firstToken->GetAsNum()->GetValue());
        result->location = firstToken->GetLocation();
        result->length = firstToken->GetLength();

        break;
    }
    case Token::Kind::kw_byte:
    case Token::Kind::kw_word:
    case Token::Kind::kw_dword:
    case Token::Kind::kw_qword:
    {
        memExprSizeOverride = 1 << (static_cast<uint8_t>(firstToken->GetKind()) - static_cast<uint8_t>(TokKind::kw_byte));

        Token& next = LookAhead();

        if (next.Is(TokKind::kw_ptr))
            //Skip ptr keyword
            NextToken();

        firstToken = &NextToken();

        if (firstToken->Is(TokKind::reg) && LookAhead().Is(TokKind::colon))
        {
            goto parse_reg_expr;
        }
        else if (firstToken->Is(TokKind::l_square) == false)
        {
            context->Error("Expecting memory expression after pointer size is determined", firstToken->GetLocation());
            return false;
        }
    }
    case Token::Kind::l_square: case Token::Kind::l_paren:
    {
    parse_mem_expr:
        if (firstToken->Is(Token::Kind::l_square))
        {
            MemoryExpr* memoryExpr = new MemoryExpr(nullptr);
            result = memoryExpr;

            if (memExprSizeOverride != 0)
                memoryExpr->sizeOverride = memExprSizeOverride;
            if (memExprSegOverride != nullptr)
                memoryExpr->segOverride.reset(memExprSegOverride);
        }
        else
        {
            result = new ParenExpr(nullptr);
        }

        if (ParseParenExpr(*reinterpret_cast<ParenExpr*>(result)) == false)
        {
            delete result;
            return false;
        }

        break;
    }
    case Token::Kind::identifier:
    {
        result = new SymbolExpr(firstToken->GetAsString()->GetValue());
        result->location = firstToken->GetLocation();
        result->length = firstToken->GetLength();

        if (result->GetAs<SymbolExpr>()->name[0] == '.')
        {
            if (currentParentLable == nullptr)
            {
                context->Error("Using local lable symbol ouside of any parent lable", result->location, result->length);
                delete result;

                return false;
            }

            SymbolExpr* symbolExpr = result->GetAs<SymbolExpr>();
            symbolExpr->name.insert(symbolExpr->name.begin(), currentParentLable->name.begin(), currentParentLable->name.end());
        }

        break;
    }
    case Token::Kind::string_literal:
    {
        result = new LiteralExpr(firstToken->GetAsString()->GetValue());
        result->location = firstToken->GetLocation();
        result->length = firstToken->GetLength();
        break;
    }
    case Token::Kind::at:
    {
        NextToken();

        if (tokenStream.front().Is(Token::Kind::identifier) == false)
        {
            context->Error("Unexpected token, segment name expected after \'@\'", tokenStream.front().GetLocation(), tokenStream.front().GetLength());
            return false;
        }

        result = new SymbolExpr('@' + tokenStream.front().GetAsString()->GetValue());

        result->location = tokenStream.front().GetLocation();
        result->length = tokenStream.front().GetLength();

        break;
    }
    case Token::Kind::dolar: case Token::Kind::dolardolar:
    {
        result = new SymbolExpr((firstToken->Is(Token::Kind::dolar) ? "$" : "$$"));
        result->location = firstToken->GetLocation();
        result->length = firstToken->GetLength();

        break;
    }
    case Token::Kind::question:
    {
        result = new SymbolExpr("?");
        result->location = firstToken->GetLocation();
        result->length = firstToken->GetLength();

        break;
    }
    default:
    {
        if (firstToken->IsUnaryOperator())
        {
            result = new UnaryExpr();

            if (ParseUnaryExpr(*reinterpret_cast<UnaryExpr*>(result)) == false)
            {
                delete result;
                return false;
            }

            break;
        }

        context->Error("Invalid syntax", firstToken->GetLocation(), firstToken->GetLength());

        return false;
        break;
    }
    }

    return true;
}

bool Parser::ParseUnaryExpr(UnaryExpr& result)
{
    Token& token = tokenStream.front();

    assert(token.IsUnaryOperator());

    result.location = tokenStream.front().GetLocation();
    result.operation = kindOperatorToChar.at(token.GetKind());

    NextToken();

    Expression* expression;

    if (ParseExpression(expression) == false)
        return false;

    result.expression.reset(expression);

    result.length =
        (tokenStream.front().GetLocation().sourcePointer + tokenStream.front().GetLength() - result.location.sourcePointer);

    return true;
}

bool Parser::ParseBinaryExpr(BinaryExpr& result, Expression* lhs)
{
    result.location = tokenStream.front().GetLocation();
    result.operation = kindOperatorToChar.at(tokenStream.front().GetKind());
    result.lhs.reset(lhs);

    Expression* rhs;

    if (tokenStream.front().Is(TokKind::minus))
    {
        result.operation = kindOperatorToChar.at(TokKind::plus);

        rhs = new UnaryExpr();

        if (ParseUnaryExpr(*reinterpret_cast<UnaryExpr*>(rhs)) == false)
        {
            result.lhs.release();
            delete rhs;

            return false;
        }
    }
    else
    {
        NextToken();

        if (ParseExpression(rhs) == false)
        {
            result.lhs.release();

            return false;
        }
    }

    result.rhs.reset(rhs);

    result.length =
        (tokenStream.front().GetLocation().sourcePointer + tokenStream.front().GetLength() - result.location.sourcePointer);

    return true;
}

bool Parser::ParseRegExpr(RegisterExpr& result)
{
    result = RegisterExpr(tokenStream.front().GetAsNum()->GetRegId());

    result.location = tokenStream.front().GetLocation();
    result.length = tokenStream.front().GetLength();

    return true;
}

bool Parser::ParseParenExpr(ParenExpr& result)
{
    assert(tokenStream.front().Is(Token::Kind::l_paren) || tokenStream.front().Is(Token::Kind::l_square));

    result.location = tokenStream.front().GetLocation();

    if (LookAhead().IsSameLine(tokenStream.front()) == false)
    {
        context->Error("Invalid paren expression", tokenStream.front().GetLocation());

        return false;
    }

    Token::Kind firstParen = tokenStream.front().GetKind();
    SourceLocation beginLocation = tokenStream.front().GetLocation();

    NextToken();

    Expression* expression;

    if (ParsePrimary(expression) == false)
        return false;

    result.expression.reset(expression);

    Token& next = LookAhead();

    if (next.IsSameLine(tokenStream.front()) == false ||
        next.Is(static_cast<Token::Kind>(static_cast<uint8_t>(firstParen) + 1)) == false)
    {
        if (firstParen == Token::Kind::l_paren)
            context->Error("Expected \')\', but paren is missing", beginLocation);
        else
            context->Error("Expected \']\', but paren is missing", beginLocation);

        return false;
    }

    NextToken();

    result.length =
        (tokenStream.front().GetLocation().sourcePointer + tokenStream.front().GetLength() - result.location.sourcePointer);

    return true;
}

bool Parser::ParseLiteralExpr(LiteralExpr& result)
{
    result = LiteralExpr(tokenStream.front().GetAsString()->GetValue());

    result.location = tokenStream.front().GetLocation();
    result.length = tokenStream.front().GetLength();

    return true;
}

bool Parser::ParseSymbolExpr(SymbolExpr& result)
{
    result = SymbolExpr(tokenStream.front().GetAsString()->GetValue());

    result.location = tokenStream.front().GetLocation();
    result.length = tokenStream.front().GetLength();

    return true;
}

bool Parser::ParseSymbolDecl(SymbolDecl& result)
{
    assert(tokenStream.front().IsKeyword());

    result.location = tokenStream.front().GetLocation();
    result.scope = static_cast<SymbolDecl::Scope>(tokenStream.front().GetKind());

    Token& identifier = NextToken();

    if (identifier.Is(Token::Kind::identifier) == false)
    {
        context->Error("Invalid symbol name or unexpected end of file", identifier.GetLocation(), identifier.GetLength());

        return false;
    }

    result.name = identifier.GetAsString()->GetValue();
    result.length =
        (tokenStream.front().GetLocation().sourcePointer + tokenStream.front().GetLength() - result.location.sourcePointer);

    context->GetSymbolTable().AddSymbol(Symbol(&result));

    return true;
}

bool Parser::ParseLableDecl(LableDecl& result)
{
    result.location = tokenStream.front().GetLocation();
    result.name = tokenStream.front().GetAsString()->GetValue();
    result.sectionStmtOffset = currentStmtOffset;

    NextToken();

    result.length =
        (tokenStream.front().GetLocation().sourcePointer + tokenStream.front().GetLength() - result.location.sourcePointer);

    //Local 
    if (result.name[0] == '.') {
        if (currentParentLable == nullptr)
        {
            context->Error("Local lable declaration is outside of any parent lable", result.location, result.length);
            return false;
        }

        result.name.insert(result.name.begin(), currentParentLable->name.begin(), currentParentLable->name.end());
        currentParentLable->childLables.push_back(&result);
    }
    else {
        currentParentLable = &result;
    }

    context->GetSymbolTable().AddSymbol(Symbol(&result));

    return true;
}

bool Parser::ParseConstantDecl(ConstantDecl& result)
{
    result.location = tokenStream.front().GetLocation();
    result.name = tokenStream.front().GetAsString()->GetValue();
    
    //Skip equ
    NextToken();
    NextToken();

    Expression* expression;

    if (ParsePrimary(expression) == false)
        return false;

    result.expression.reset(expression);

    context->GetSymbolTable().AddSymbol(Symbol(&result));

    result.length =
        (tokenStream.front().GetLocation().sourcePointer + tokenStream.front().GetLength() - result.location.sourcePointer);

    return true;
}

bool Parser::ParseSectionDecl(SectionDecl& result)
{
    assert(tokenStream.front().Is(TokKind::kw_section) || tokenStream.front().Is(TokKind::kw_segment));

    result.location = tokenStream.front().GetLocation();

    if (NextToken().Is(TokKind::identifier) == false)
    {
        context->Error("Unexpected token or end of file", tokenStream.front().GetLocation(), tokenStream.front().GetLength());

        return false;
    }

    result.name = tokenStream.front().GetAsString()->GetValue();
    result.length =
        (tokenStream.front().GetLocation().sourcePointer + tokenStream.front().GetLength() - result.location.sourcePointer);

    return true;
}

bool Parser::ParseInstructionStmt(InstructionStmt& result)
{
    assert(tokenStream.front().Is(TokKind::identifier));

    result.location = tokenStream.front().GetLocation();
    result.mnemonic = tokenStream.front().GetAsString()->GetValue();

    Token* next = &LookAhead();

    while (next->Is(TokKind::eof) == false && next->GetLocation().line == tokenStream.front().GetLocation().line)
    {
        NextToken();

        Expression* expression;

        if (ParsePrimary(expression) == false)
            return false;

        result.operands.push_back(nullptr);
        result.operands.back().reset(expression);

        next = &LookAhead();

        if (next->Is(TokKind::comma) == false || next->GetLocation().line != tokenStream.front().GetLocation().line)
            break;

        NextToken();
        next = &LookAhead();
    }

    result.sectionStmtOffset = currentStmtOffset;
    result.length =
        (tokenStream.front().GetLocation().sourcePointer + tokenStream.front().GetLength() - result.location.sourcePointer);

    return true;
}

bool Parser::ParseDefineDataStmt(DefineDataStmt& result)
{
    result.location = tokenStream.front().GetLocation();
    result.dataUnitSize = Arch::Arch8086::DefineDataMnemonics.at(tokenStream.front().GetAsString()->GetValue());

    Token* next = &LookAhead();

    while (next->Is(TokKind::eof) == false && next->GetLocation().line == tokenStream.front().GetLocation().line)
    {
        NextToken();

        Expression* expression;

        if (ParsePrimary(expression) == false)
            return false;

        if (expression->Is<RegisterExpr>())
        {
            delete expression;
            context->Error("Register can't be a data unit", tokenStream.front().GetLocation());

            NextToken();

            return false;
        }

        result.units.push_back(nullptr);
        result.units.back().reset(expression);

        next = &LookAhead();

        if (next->Is(TokKind::kw_dup))
        {
            if (result.units.size() < 1)
            {
                context->Error("Data define with \'dup\' requires count defenition before \'dup\' keyword", tokenStream.front().GetLocation());

                return false;
            }

            //result.dupValue = std::make_unique<ParenExpr>(nullptr);
            Expression* countExpr = result.units.back().get();
            result.units.back().release();

            SourceLocation dupLocation = next->GetLocation();

            //Skip to kw_dup
            NextToken();
            //Skip kw_dup to '(' paren
            NextToken();

            ParenExpr* valueExpr = new ParenExpr(nullptr);

            if (ParseParenExpr(*valueExpr) == false)
            {
                result.units.pop_back();
                return false;
            }

            DuplicateExpr* dupExpr = new DuplicateExpr(countExpr, valueExpr);

            dupExpr->location = dupLocation;
            dupExpr->length = valueExpr->GetLocation().sourcePointer + valueExpr->GetLength() - dupLocation.sourcePointer;

            result.units.back().reset(dupExpr);

            next = &LookAhead();
        }

        if (next->Is(TokKind::comma) == false || next->GetLocation().line != tokenStream.front().GetLocation().line)
            break;

        NextToken();
        next = &LookAhead();
    }

    result.length =
        (tokenStream.front().GetLocation().sourcePointer + tokenStream.front().GetLength() - result.location.sourcePointer);

    return true;
}

bool Parser::ParseParametricStmt(AST::ParametricStmt& result)
{
    result.location = tokenStream.front().GetLocation();

    Expression* value = nullptr;

    NextToken();

    bool success = ParsePrimary(value);

    if (success)
    {
        result.value.reset(value);
        result.length = value->GetLocation().sourcePointer + value->GetLength() - result.location.sourcePointer;
    }

    return success;
}