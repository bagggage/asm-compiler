#ifndef __AST_EXPRESSIONS_H
#define __AST_EXPRESSIONS_H

#include <memory>
#include <string>
#include <optional>
#include <cassert>
#include <vector>
#include <unordered_map>

#include "node.h"
#include "arch/8086/regs.h"
#include "arch/8086/encoding.h"

namespace ASM
{
    class Parser;
}

namespace ASM::AST
{
    struct SymbolExpr;

    struct Expression : public Node
    {
    private:
        friend class ASM::Parser;
    public:
        virtual int64_t Resolve(const std::unordered_map<std::string, int64_t>& symbolsMap = {}) const = 0;
        virtual bool IsDependent() const = 0;
        virtual bool Simplify() = 0;

        virtual std::vector<const std::string*> GetDependecies() const {
            return std::vector<const std::string*>();
        }
    };

    struct NumberExpr : public Expression
    {
    private:
        friend class ASM::Parser;
    public:
        NumberExpr() = default;
        NumberExpr(int64_t val) : value(val) {}

        int64_t value;

        int64_t Resolve(const std::unordered_map<std::string, int64_t>& symbolsMap = {}) const override;

        bool IsDependent() const override;
        bool Simplify() override;
    };

    struct RegisterExpr : public Expression
    {
    private:
        Arch::RegisterIdentifier identifier;
        Arch::RegisterGroup group;

        friend class ASM::Parser;
    public:
        RegisterExpr(Arch::RegisterIdentifier reg);

        inline Arch::RegisterIdentifier GetIdentifier() const { return identifier; }
        inline Arch::RegisterGroup GetGroup() const { return group; }

        uint8_t GetSize() const;
        Arch::Register GetEncoding() const;
        
        int64_t Resolve(const std::unordered_map<std::string, int64_t>& symbolsMap = {}) const override;

        bool IsDependent() const override;
        bool Simplify() override;
    };

    struct LiteralExpr : public Expression
    {
    private:
        friend class ASM::Parser;
    public:
        LiteralExpr(const std::string& string) : value(string) {}

        std::string value;

        int64_t Resolve(const std::unordered_map<std::string, int64_t>& symbolsMap = {}) const override;

        bool IsDependent() const override;
        bool Simplify() override;
    };

    struct UnaryExpr : public Expression
    {
    private:
        static constexpr char NullOperation = '+';

        std::unique_ptr<Expression> expression;

        char operation = NullOperation;

        friend class ASM::Parser;
    public:
        int64_t Resolve(const std::unordered_map<std::string, int64_t>& symbolsMap = {}) const override;

        bool IsDependent() const override;
        bool Simplify() override;

        std::vector<const std::string*> GetDependecies() const override;

        char GetOperation() const { return operation; }
        Expression* GetExpression() const { return expression.get(); }
    };

    struct BinaryExpr : public Expression
    {
    private:
        friend class ASM::Parser;
    public:
        std::unique_ptr<Expression> lhs, rhs;

        char operation;

        int64_t Resolve(const std::unordered_map<std::string, int64_t>& symbolsMap = {}) const override;

        bool IsDependent() const override;
        bool Simplify() override;

        std::vector<const std::string*> GetDependecies() const override;
    };

    struct ParenExpr : public Expression
    {
    protected:
        std::unique_ptr<Expression> expression;

        friend class ASM::Parser;
    public:
        ParenExpr(Expression* child);

        int64_t Resolve(const std::unordered_map<std::string, int64_t>& symbolsMap = {}) const override;

        bool IsDependent() const override;
        bool Simplify() override;

        std::vector<const std::string*> GetDependecies() const override;

        inline Expression* GetExpression() const { return expression.get(); }
    };

    struct MemoryExpr : public ParenExpr
    {
    private:
        static void MakeRmRegsCombination(std::vector<Arch::RegisterIdentifier>& combination, Expression* Expression);

        std::unique_ptr<RegisterExpr> segOverride;
        //in bytes
        uint8_t sizeOverride = 0;

        friend class ASM::Parser;
    public:
        MemoryExpr(Expression* child) : ParenExpr(child) {}

        std::vector<Arch::RegisterIdentifier> GetRmRegsCombination() const;

        inline RegisterExpr* GetSegOverride() const { return segOverride.get(); }
        inline uint8_t GetSizeOverride() const { return sizeOverride; }
    };

    struct SymbolExpr : public Expression
    {
    private:
        std::string name;

        friend class ASM::Parser;
    public:
        SymbolExpr(const std::string& symbolName) : name(symbolName) {}

        int64_t Resolve(const std::unordered_map<std::string, int64_t>& symbolsMap = {}) const override;

        bool IsDependent() const override;
        bool Simplify() override;

        std::vector<const std::string*> GetDependecies() const override;

        inline const std::string& GetName() const { return name; }
    };

    struct DuplicateExpr : public Expression
    {
    private:
        friend class ASM::Parser;

        std::unique_ptr<Expression> countExpression;
        std::unique_ptr<Expression> valueExpression;        
    public:
        DuplicateExpr(Expression* countExpr, Expression* valueExpr)
        {
            countExpression.reset(countExpr);
            valueExpression.reset(valueExpr);
        }

        int64_t Resolve(const std::unordered_map<std::string, int64_t>& symbolsMap = {}) const override;

        bool IsDependent() const override;
        bool Simplify() override;

        std::vector<const std::string*> GetDependecies() const override;

        inline Expression* GetCountExpression() { return countExpression.get(); }
        inline Expression* GetValueExpression() { return valueExpression.get(); }
    };
}

#endif