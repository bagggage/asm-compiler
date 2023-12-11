#include "code-generator.h"

using namespace ASM;
using namespace ASM::AST;
using namespace ASM::Codegen;

uint8_t CodeGenerator::EvaluateLiteralByteSize(int64_t value)
{
    if (value < 0)
        value *= -1;

    uint8_t size = 0;

    do
    {
        ++size;
        value = value >> 8;
    } while (value != 0);

    return size;
}

Arch::OpType CodeGenerator::GetDirectEncodingOfRegisterOpType(RegisterExpr* registerExpression)
{
    if (registerExpression->GetGroup() == Arch::RegisterGroup::GeneralPerpose)
    {
        switch (registerExpression->GetIdentifier())
        {
        case Arch::RegisterIdentifier::AL: return Arch::OpType::AL;
            break;
        case Arch::RegisterIdentifier::AH: return Arch::OpType::AL;
            break;
        case Arch::RegisterIdentifier::AX: return Arch::OpType::AX;
            break;
        case Arch::RegisterIdentifier::EAX: return Arch::OpType::EAX;
            break;
        case Arch::RegisterIdentifier::DX: return Arch::OpType::DX;
            break;
        case Arch::RegisterIdentifier::CL: return Arch::OpType::CL;
            break;
        default:
            break;
        }
    }
    else if (registerExpression->GetGroup() == Arch::RegisterGroup::Segment)
    {
        switch (registerExpression->GetIdentifier())
        {
        case Arch::RegisterIdentifier::CS: return Arch::OpType::CS;
            break;
        case Arch::RegisterIdentifier::DS: return Arch::OpType::DS;
            break;
        case Arch::RegisterIdentifier::ES: return Arch::OpType::ES;
            break;
        case Arch::RegisterIdentifier::SS: return Arch::OpType::SS;
            break;
        case Arch::RegisterIdentifier::FS: return Arch::OpType::FS;
            break;
        case Arch::RegisterIdentifier::GS: return Arch::OpType::GS;
            break;
        default:
            break;
        }
    }

    return Arch::OpType::none;
}

uint8_t CodeGenerator::GetOperandTypePriority(Arch::OpType type)
{
    if (type >= Arch::OpType::AL && type <= Arch::OpType::GS || type == Arch::OpType::ONE)
        return highPriority;

    return lowPriority;
}

SmallVector<Arch::OperandEvaluation, 4> CodeGenerator::EvaluateOperands(const InstructionStmt::Operands_t& operands)
{
    SmallVector<Arch::OperandEvaluation, 4> result = SmallVector<Arch::OperandEvaluation, 4>(operands.size());

    for (int i = 0; i < operands.size(); ++i)
    {
        auto& operand = operands[i];
        auto& evaluation = result[i];
        auto& types = result[i].expectedTypes;

        if (operand->Is<RegisterExpr>())
        {
            evaluation.kind = Arch::OperandEvaluation::Kind::Register;
            evaluation.minRequiredSize = operand->GetAs<RegisterExpr>()->GetSize();

            switch (operand->GetAs<RegisterExpr>()->GetGroup())
            {
            case Arch::RegisterGroup::Segment:
                types.push_back(Arch::OpType::sreg);
                break;
            case Arch::RegisterGroup::Control:
                types.push_back(Arch::OpType::creg);
                break;
            default:
                types.push_back(Arch::OpType::r);
                types.push_back(Arch::OpType::rm);
                break;
            }

            Arch::OpType directRegisterOpType = GetDirectEncodingOfRegisterOpType(operand->GetAs<RegisterExpr>());

            if (directRegisterOpType != Arch::OpType::none)
                types.push_back(directRegisterOpType);
        }
        else if (operand->Is<MemoryExpr>())
        {
            evaluation.kind = Arch::OperandEvaluation::Kind::Memory;

            types.push_back(Arch::OpType::m);
            types.push_back(Arch::OpType::rm);

            if (operand->GetAs<MemoryExpr>()->GetRmRegsCombination().empty())
                types.push_back(Arch::OpType::moffs);
        }
        else
        {
            evaluation.kind = Arch::OperandEvaluation::Kind::Immediate;
            evaluation.minRequiredSize = 16;

            if (operand->IsDependent() == false)
            {
                int64_t value = operand->Resolve();

                evaluation.minRequiredSize = EvaluateLiteralByteSize(value);

                if (value == 1)
                    types.push_back(Arch::OpType::ONE);
            }

            types.push_back(Arch::OpType::imm);
            types.push_back(Arch::OpType::rel);
            types.push_back(Arch::OpType::ptr);
        }
    }

    return std::move(result);
}

void CodeGenerator::ChangeCurrentSection(const std::string& sectionName)
{
    currentSection = &context->GetTranslationUnit().GetOrMakeSection(sectionName);
    currentSectionCode = &currentSection->GetCode();
}

const Arch::Instruction* CodeGenerator::ChooseInstructionByOperands(const std::string& mnemonic, const InstructionStmt::Operands_t& operands) const
{
    auto& instructions = context->GetInstructionSet().at(mnemonic);

    const Arch::Instruction* mostAppropriateInstruction = nullptr;
    uint8_t priority = 0;

    auto evaluatedOperands = EvaluateOperands(operands);

    for (auto& instruction : instructions)
    {
        if (evaluatedOperands.size() != instruction.operands.size())
            continue;

        uint8_t currentPriority = evaluatedOperands.size() == 0 ? 1 : 0;

        for (int i = 0; i < instruction.operands.size(); ++i)
        {
            auto& operandPrototype = instruction.operands[i];
            auto& evaluatedOperand = evaluatedOperands[i];

            //Check if operands size matches
            if (operandPrototype.size != 0 && (
                    operandPrototype.size < evaluatedOperand.minRequiredSize ||
                    evaluatedOperand.Is(Arch::OperandEvaluation::Kind::Register) &&
                    evaluatedOperand.minRequiredSize != operandPrototype.size
                ))
            { currentPriority = 0; break; }

            //Check if operand types suitable 
            if (std::find
                (
                    evaluatedOperand.expectedTypes.begin(),
                    evaluatedOperand.expectedTypes.end(),
                    operandPrototype.type
                ) == evaluatedOperand.expectedTypes.end())
            { currentPriority = 0; break; }

            currentPriority += GetOperandTypePriority(operandPrototype.type);

            if (operandPrototype.size != 0 &&
                operandPrototype.size == evaluatedOperand.minRequiredSize)
                ++currentPriority;
        }

        if (currentPriority > priority)
        {
            mostAppropriateInstruction = &instruction;
            priority = currentPriority;
        }
    }

    return mostAppropriateInstruction;
}

bool CodeGenerator::IsExpressionHasAddressSymbol(Expression* expression) const
{
    bool result = false;

    if (expression->Is<BinaryExpr>())
    {
        BinaryExpr* binaryExpr = expression->GetAs<BinaryExpr>();

        result = IsExpressionHasAddressSymbol(binaryExpr->lhs.get());
        result |= IsExpressionHasAddressSymbol(binaryExpr->rhs.get());
    }
    else if (expression->Is<UnaryExpr>())
    {
        result = IsExpressionHasAddressSymbol(expression->GetAs<UnaryExpr>()->GetExpression());
    }
    else if (expression->Is<ParenExpr>())
    {
        result = IsExpressionHasAddressSymbol(expression->GetAs<ParenExpr>()->GetExpression());
    }
    else if (expression->Is<SymbolExpr>())
    {
        SymbolExpr* symbolExpr = expression->GetAs<SymbolExpr>();

        if (context->GetSymbolTable().HasSymbol(symbolExpr->GetName()) == false)
        {
            context->Error((std::string("Undefined symbol: ") + expression->GetAs<SymbolExpr>()->GetName()).c_str());
        }
        else
        {
            auto& symbol = context->GetSymbolTable().GetSymbol(symbolExpr->GetName());
            result = symbol.GetDeclaration().IsAddress();
        }
    }

    return result;
}

void CodeGenerator::MakeAbsoluteLinkTarget(Expression* expression, uint8_t offset, uint8_t size)
{
    currentSection->GetLinkingTargets().push_back(LinkingTarget
    (
        expression,
        LinkingTarget::Kind::AbsoluteAddress,
        currentSectionCode->code.size() + offset,
        size
    ));
}

void CodeGenerator::MakeRelativeLinkTarget(Expression* expression, uint8_t offset, uint8_t size, uint8_t relativeOrigin)
{
    currentSection->GetLinkingTargets().push_back(LinkingTarget
    (
        expression,
        currentSectionCode->code.size() + offset,
        size,
        currentSectionCode->code.size() + relativeOrigin
    ));
}

void CodeGenerator::MakeValueLinkTarget(Expression* expression, uint8_t offset, uint8_t size, LinkingTarget::Type type)
{
    currentSection->GetLinkingTargets().push_back(LinkingTarget
    (
        expression,
        LinkingTarget::Kind::Value,
        currentSectionCode->code.size() + offset,
        size
    ));
}

bool CodeGenerator::ResolveExpressionDependencies(
    const AST::Expression* expression,
    const std::string& symbolName,
    std::unordered_map<std::string, int64_t>& symbolMap
) const 
{
    auto dependencies = expression->GetDependecies();

    if (dependencies.empty() == false) {
        for (auto depenency : dependencies) {
            if (context->GetSymbolTable().HasSymbol(*depenency) == false)
                return false;

            const SymbolDecl& declaration = context->GetSymbolTable().GetSymbol(*depenency).GetDeclaration();

            if (declaration.Is<ConstantDecl>() == false)
                return false;

            if (ResolveExpressionDependencies(&declaration.GetAs<ConstantDecl>()->GetExpression(), declaration.GetName(), symbolMap) == false)
                return false;
        }
    }
    else if (symbolName.empty() == false && symbolMap.count(symbolName) == 0) {
        symbolMap.insert({ symbolName, expression->Resolve(symbolMap) });
    }

    return true;
}

std::optional<int64_t> CodeGenerator::ResolveExpression(const Expression* expression) const
{
    std::optional<int64_t> result;
    std::unordered_map<std::string, int64_t> symbolMap;
    
    if (ResolveExpressionDependencies(expression, std::string(), symbolMap) == false)
        return result;

    result = expression->Resolve(symbolMap);

    return result;
}

TranslationUnit& CodeGenerator::ProccessAST(AbstractSyntaxTree& ast)
{
    ChangeCurrentSection(context->UnnamedSection.data());

    for (auto& node : ast)
    {
        if (node->Is<Statement>())
        {
            currentSectionCode->operator<<(node->GetAs<Statement>()->CodeGen(*this));
        }
        else if (node->Is<SectionDecl>())
        {
            ChangeCurrentSection(node->GetAs<SectionDecl>()->GetName());
        }
        else if (node->Is<SymbolDecl>())
        {
            if (node->Is<LableDecl>())
                context->GetSymbolTable().EvaluateSymbol
                (
                    node->GetAs<SymbolDecl>()->GetName(),
                    SymbolValue(SymbolValue::Kind::Address, currentSectionCode->code.size())
                );
        }
    }

    return context->GetTranslationUnit();
}