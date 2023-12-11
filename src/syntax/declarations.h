#ifndef __AST_DECLARATIONS_H
#define __AST_DECLARATIONS_H

#include <memory>

#include "token.h"
#include "expressions.h"
#include "node.h"

namespace ASM
{
    class Parser;
}

namespace ASM::AST
{
    struct Declaration : public Node {};

    struct OrgDecl : public Declaration
    {
    private:
        std::unique_ptr<Expression> expression;

        friend class ASM::Parser;
    public:
        OrgDecl(Expression* expression);

        inline Expression& GetExpression() { return *expression.get(); }
        inline const Expression& GetExpression() const { return *expression.get(); }

        inline int64_t GetValue() const { return expression->Resolve(); }
    };

    struct NamedDecl : public Declaration
    {
    protected:
        std::string name;

        friend class ASM::Parser;
    public:
        NamedDecl(const std::string& name) : name(name) {};

        inline const std::string& GetName() const { return name; };
    };

    struct SymbolDecl : public NamedDecl
    {
    public:
        enum class Scope : uint8_t
        {
            Local,
            Extern = static_cast<uint8_t>(Token::Kind::kw_extern),
            Global = static_cast<uint8_t>(Token::Kind::kw_global)
        };
    protected:
        Scope scope = Scope::Local;

        friend class ASM::Parser;
    public:
        SymbolDecl(const std::string& name, Scope scope = Scope::Local) : NamedDecl(name), scope(scope) {}

        inline Scope GetScope() const { return scope; }

        virtual bool IsAddress() const { return true; }
    };

    struct ConstantDecl : public SymbolDecl
    {
    private:
        std::unique_ptr<Expression> expression;

        friend class ASM::Parser;
    public:
        ConstantDecl(const std::string& name, Expression* expression) : SymbolDecl(name, Scope::Local) {this->expression.reset(expression); }

        inline Expression& GetExpression() { return *expression.get(); }
        inline const Expression& GetExpression() const { return *expression.get(); }

        bool IsAddress() const override { return false; }
    };

    struct SectionDecl;

    struct LableDecl : public SymbolDecl
    {
    private:
        const SectionDecl* relatedSection = nullptr;
        size_t sectionStmtOffset = 0;

        friend class ASM::Parser;
    public:
        LableDecl(const std::string& name, const SectionDecl* relatedSection, size_t stmtOffset = 0)
            : SymbolDecl(name, Scope::Local), relatedSection(relatedSection), sectionStmtOffset(stmtOffset) {}

        inline const SectionDecl* GetRelatedSection() const { return relatedSection; }
        inline size_t GetSectionStmtOffset() const { return sectionStmtOffset; }
    };

    struct SectionDecl : public NamedDecl
    {
    private:
        friend class ASM::Parser;
    public:
        SectionDecl(const std::string& name) : NamedDecl(name) {}
    };
}

#endif