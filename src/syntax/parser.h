#ifndef __PARSER_H
#define __PARSER_H

#include <queue>
#include <mutex>
#include <unordered_map>
#include <list>

#include "context/context.h"
#include "token.h"
#include "ast.h"

namespace ASM
{
    class Parser
    {
    private:
        AssemblyContext* context = nullptr;

        std::list<Token> tokenStream;
        std::mutex tokenStreamMutex;

        uint32_t currentStmtOffset = 0;
        AST::SectionDecl* currentSection = nullptr;

        static const std::unordered_map<char, uint8_t> operatorPriorities;
        static const std::unordered_map<TokKind, char> kindOperatorToChar;

        Token& NextToken();
        Token& LookAhead();

        bool HasNextToken() const;
        inline bool IsNextTokSameLine() { return (tokenStream.back().GetLocation().line == LookAhead().GetLocation().line); }
    public:
        Parser(AssemblyContext& context) : context(&context) {}

        void PushToken(Token&& token);

        AbstractSyntaxTree Parse();

        bool ParsePrimary(AST::Expression*& result);
        bool ParseExpression(AST::Expression*& result);

        bool ParseUnaryExpr(AST::UnaryExpr& result);
        bool ParseBinaryExpr(AST::BinaryExpr& result, AST::Expression* lhs);
        bool ParseNumberExpr(AST::NumberExpr& result);
        bool ParseRegExpr(AST::RegisterExpr& result);
        bool ParseParenExpr(AST::ParenExpr& result);
        bool ParseLiteralExpr(AST::LiteralExpr& result);
        bool ParseSymbolExpr(AST::SymbolExpr& result);
        bool ParseSymbolDecl(AST::SymbolDecl& result);
        bool ParseLableDecl(AST::LableDecl& result);
        bool ParseConstantDecl(AST::ConstantDecl& result);
        bool ParseSectionDecl(AST::SectionDecl& result);
        bool ParseInstructionStmt(AST::InstructionStmt& result);
        bool ParseDefineDataStmt(AST::DefineDataStmt& result);
    };
}

#endif