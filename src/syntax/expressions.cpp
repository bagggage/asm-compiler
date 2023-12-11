#include "expressions.h"

using namespace ASM;
using namespace ASM::AST;

int64_t NumberExpr::Resolve(const std::unordered_map<std::string, int64_t>& symbolsMap) const { return value; }

bool NumberExpr::IsDependent() const { return false; }
bool NumberExpr::Simplify()          { return false; }

RegisterExpr::RegisterExpr(Arch::RegisterIdentifier reg) : identifier(reg)
{
    if (reg >= Arch::RegisterIdentifier::AL && reg <= Arch::RegisterIdentifier::RDI)
        group = Arch::RegisterGroup::GeneralPerpose;
    else if (reg >= Arch::RegisterIdentifier::ES && reg < Arch::RegisterIdentifier::CR0)
        group = Arch::RegisterGroup::Segment;
    else if (reg >= Arch::RegisterIdentifier::CR0 && reg <= Arch::RegisterIdentifier::CR7)
        group = Arch::RegisterGroup::Control;
    else if (reg >= Arch::RegisterIdentifier::MM0 && reg <= Arch::RegisterIdentifier::XMM7)
        group = Arch::RegisterGroup::MMX;
    else
        group = Arch::RegisterGroup::Unknown;
}

uint8_t RegisterExpr::GetSize() const
{
    switch (group)
    {
    case Arch::RegisterGroup::GeneralPerpose:
        return (8 << (static_cast<uint8_t>(identifier) / 8));
        break;
    case Arch::RegisterGroup::Segment:
        return (8 << 1);
        break;
    case Arch::RegisterGroup::Control:
        return (8 << 2);
        break;
    case Arch::RegisterGroup::Extented64:
        return (8 << 3);
        break;
    case Arch::RegisterGroup::MMX:
        return (identifier < Arch::RegisterIdentifier::XMM0 ? 64 : 128);
        break;
    default:
        assert(false);
        break;
    }

    return 0;
}

Arch::Register RegisterExpr::GetEncoding() const
{
    if (group == Arch::RegisterGroup::GeneralPerpose || group == Arch::RegisterGroup::MMX)
        return static_cast<Arch::Register>(static_cast<uint8_t>(identifier) % 8);
    if (group == Arch::RegisterGroup::Segment)
        return static_cast<Arch::Register>(static_cast<uint8_t>(identifier) - static_cast<uint8_t>(Arch::RegisterIdentifier::ES));
    if (group == Arch::RegisterGroup::Control)
        return static_cast<Arch::Register>(static_cast<uint8_t>(identifier) - static_cast<uint8_t>(Arch::RegisterIdentifier::CR0));

    return Arch::Register::Invalid;
}

int64_t RegisterExpr::Resolve(const std::unordered_map<std::string, int64_t>& symbolsMap) const { return 0; }

bool RegisterExpr::IsDependent() const { return false; }
bool RegisterExpr::Simplify()          { return false; }

int64_t LiteralExpr::Resolve(const std::unordered_map<std::string, int64_t>& symbolsMap) const
{
    if (value.size() == 1)
        return value.back();

    return 0;
}

bool LiteralExpr::IsDependent() const
{
    return value.size() != 1;
}

bool LiteralExpr::Simplify()
{
    return value.size() == 1;
}

int64_t UnaryExpr::Resolve(const std::unordered_map<std::string, int64_t>& symbolsMap) const
{
    int64_t value = expression->Resolve(symbolsMap);

    switch (operation)
    {
    case '-':
        value *= -1;
        break;
    case '~':
        value = ~value;
        break;
    case NullOperation:
        break;
    default:
        assert(false);
        break;
    }

    return value;
}

bool UnaryExpr::IsDependent() const
{
    return expression->IsDependent();
}

bool UnaryExpr::Simplify()
{
    if (IsDependent() == false)
    {
        if (dynamic_cast<NumberExpr*>(expression.get()) && operation == NullOperation)
            return false;

        expression.reset(new NumberExpr(Resolve()));
        operation = NullOperation;

        return true;
    }
    else
    {
        return expression->Simplify();
    }
}

std::vector<const std::string*> UnaryExpr::GetDependecies() const {
    return expression->GetDependecies();
}

int64_t BinaryExpr::Resolve(const std::unordered_map<std::string, int64_t>& symbolsMap) const
{
    int64_t lhsVal = lhs->Resolve(symbolsMap);
    int64_t rhsVal = rhs->Resolve(symbolsMap);

    switch (operation)
    {
    case '+':
        lhsVal += rhsVal;
        break;
    case '-':
        lhsVal -= rhsVal;
        break;
    case '*':
        lhsVal *= rhsVal;
        break;
    case '/':
        lhsVal /= rhsVal;
        break;
    case '>':
        lhsVal = lhsVal >> rhsVal;
        break;
    case '<':
        lhsVal = lhsVal << rhsVal;
        break;
    case '^':
        lhsVal ^= rhsVal;
        break;
    case '|':
        lhsVal |= rhsVal;
        break;
    case '&':
        lhsVal &= rhsVal;
        break;
    default:
        assert(false);
        break;
    }

    return lhsVal;
}

bool BinaryExpr::IsDependent() const
{
    return (lhs->IsDependent() || rhs->IsDependent());
}

bool BinaryExpr::Simplify()
{
    bool result = false;

    if (dynamic_cast<NumberExpr*>(lhs.get()) == nullptr && lhs->IsDependent() == false)
    {
        lhs.reset(new NumberExpr(lhs->Resolve()));

        result = true;
    }
    else
    {
        result = lhs->Simplify();
    }

    if (dynamic_cast<NumberExpr*>(rhs.get()) == nullptr && rhs->IsDependent() == false)
    {
        rhs.reset(new NumberExpr(rhs->Resolve()));

        result = true;
    }
    else
    {
        result |= rhs->Simplify();
    }

    NumberExpr* lhsNum = dynamic_cast<NumberExpr*>(lhs.get());
    NumberExpr* rhsNum = dynamic_cast<NumberExpr*>(rhs.get());

    if (lhsNum && rhsNum)
    {
        lhsNum->value = Resolve();
        rhsNum->value = 0;

        operation = '+';

        result = true;
    }

    return result;
}

std::vector<const std::string*> BinaryExpr::GetDependecies() const {
    auto result = lhs->GetDependecies();
    auto rhsDependencies = rhs->GetDependecies();

    result.insert(result.end(), rhsDependencies.begin(), rhsDependencies.end());

    return result;
}

ParenExpr::ParenExpr(Expression* child)
{
    expression.reset(child);
}

int64_t ParenExpr::Resolve(const std::unordered_map<std::string, int64_t>& symbolsMap) const { return expression->Resolve(symbolsMap); }

bool ParenExpr::IsDependent() const { return expression->IsDependent(); }
bool ParenExpr::Simplify()          { return expression->Simplify(); }

std::vector<const std::string*> ParenExpr::GetDependecies() const {
    return expression->GetDependecies();
}

void MemoryExpr::MakeRmRegsCombination(std::vector<Arch::RegisterIdentifier>& combination, Expression* expression)
{
    if (expression->Is<BinaryExpr>())
    {
        BinaryExpr* binaryExpr = expression->GetAs<BinaryExpr>();

        MakeRmRegsCombination(combination, binaryExpr->lhs.get());
        MakeRmRegsCombination(combination, binaryExpr->rhs.get());
    }
    else if (expression->Is<UnaryExpr>())
    {
        MakeRmRegsCombination(combination, expression->GetAs<UnaryExpr>()->GetExpression());
    }
    else if (expression->Is<ParenExpr>())
    {
        MakeRmRegsCombination(combination, expression->GetAs<ParenExpr>()->GetExpression());
    }
    else if (expression->Is<RegisterExpr>())
    {
        combination.push_back(expression->GetAs<RegisterExpr>()->GetIdentifier());
    }
}

std::vector<Arch::RegisterIdentifier> MemoryExpr::GetRmRegsCombination() const
{
    std::vector<Arch::RegisterIdentifier> result;

    MakeRmRegsCombination(result, expression.get());

    std::sort(result.begin(), result.end());

    return std::move(result);
}

int64_t SymbolExpr::Resolve(const std::unordered_map<std::string, int64_t>& symbolsMap) const {
    if (symbolsMap.count(name) == 0) [[unlikely]]
        return 0;

    return symbolsMap.at(name);
}

bool SymbolExpr::IsDependent() const { return true; }
bool SymbolExpr::Simplify()          { return false; }

std::vector<const std::string*> SymbolExpr::GetDependecies() const {
    return { &name };
}