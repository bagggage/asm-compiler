#include "statements.h"

#include "context/context.h"
#include "codegen/code-generator.h"

using namespace ASM;
using namespace ASM::AST;
using namespace ASM::Codegen;
using namespace ASM::Arch;

bool InstructionStmt::IsCorrectMemoryExprPass(Expression* expression, bool registersCheck)
{
    bool result = true;

    if (expression->Is<BinaryExpr>())
    {
        BinaryExpr* binaryExpr = expression->GetAs<BinaryExpr>();

        bool lhsCheck = registersCheck;
        bool rhsCheck = registersCheck;

        if (binaryExpr->operation != '+' && binaryExpr->operation != '-')
            lhsCheck = rhsCheck = true;
        else if (binaryExpr->operation == '-')
            rhsCheck = true; 

        result = IsCorrectMemoryExprPass(binaryExpr->lhs.get(), lhsCheck);
        result &= IsCorrectMemoryExprPass(binaryExpr->rhs.get(), rhsCheck);
    }
    else if (expression->Is<UnaryExpr>())
    {
        UnaryExpr* unaryExpr = expression->GetAs<UnaryExpr>();

        result = IsCorrectMemoryExprPass(unaryExpr->GetExpression(), registersCheck || unaryExpr->GetOperation() != '+');
    }
    else if (registersCheck && expression->Is<RegisterExpr>())
    {
        result = false;
    }
    else if (expression->Is<ParenExpr>())
    {
        result = IsCorrectMemoryExprPass(expression->GetAs<ParenExpr>()->GetExpression(), registersCheck);
    }

    return result;
}

bool InstructionStmt::IsCorrectMemoryExpr(MemoryExpr* expression)
{
    return IsCorrectMemoryExprPass(expression->GetExpression());
}

void InstructionStmt::EncodeModRM(ModRM modrm, const std::unique_ptr<Expression>& operand, CodeGenerator& generator, MachineCode& code)
{
    int64_t displacement = 0;
    uint8_t dispSize = 0;

    if (operand->Is<RegisterExpr>())
    {
        modrm.mod = Mod::REG;
        modrm.rm = static_cast<RM>(operand->GetAs<RegisterExpr>()->GetEncoding());
    }
    else if (operand->Is<MemoryExpr>())
    {
        MemoryExpr* memoryExpr = operand->GetAs<MemoryExpr>();

        if (IsCorrectMemoryExpr(memoryExpr) == false ||
            Arch8086::RmRegsCombinations.count(memoryExpr->GetRmRegsCombination()) == 0)
        {
            generator.GetContext().Error("Invalid memory expression");
            return;
        }

        auto regsCombination = memoryExpr->GetRmRegsCombination();

        if (regsCombination.empty())
        {
            modrm.mod = Mod::MEM;
            modrm.rm = RM::DISP_16;

            //Disp
            if (memoryExpr->GetExpression()->IsDependent())
            {
                if (generator.IsExpressionHasAddressSymbol(memoryExpr->GetExpression()))
                    generator.MakeAbsoluteLinkTarget(memoryExpr->GetExpression(), code->size() + 1, maxModRm16DisplacementSize);
                else
                    generator.MakeValueLinkTarget(memoryExpr->GetExpression(), code->size() + 1, maxModRm16DisplacementSize, LinkingTarget::Type::Integer);
            }
            else
            {
                displacement = memoryExpr->GetExpression()->Resolve();

                if (displacement != 0)
                    dispSize = maxModRm16DisplacementSize;
            }
        }
        else
        {
            modrm.rm = Arch8086::RmRegsCombinations.at(regsCombination);

            //Disp
            if (memoryExpr->GetExpression()->IsDependent())
            {
                modrm.mod = Mod::MEM_EX;

                if (generator.IsExpressionHasAddressSymbol(memoryExpr->GetExpression()))
                    generator.MakeAbsoluteLinkTarget(memoryExpr->GetExpression(), code->size() + 1, maxModRm16DisplacementSize);
                else
                    generator.MakeValueLinkTarget(memoryExpr->GetExpression(), code->size() + 1, maxModRm16DisplacementSize, LinkingTarget::Type::Integer);
            }
            else
            {
                displacement = memoryExpr->GetExpression()->Resolve();

                if (displacement != 0)
                {
                    dispSize = CodeGenerator::EvaluateLiteralByteSize(displacement);
                    modrm.mod = (dispSize == 1 ? Mod::MEM8 : Mod::MEM_EX);
                }
            }
        }
    }
    else
    {
        generator.GetContext().Error("Invalid operand for ModRegRM byte");
    }

    code << modrm;

    if (dispSize)
        code.Push(reinterpret_cast<uint8_t*>(&displacement), dispSize);

    return;
}

void InstructionStmt::EncodeImm(Expression* operand, Operand operandPrototype, CodeGenerator& generator, MachineCode& code)
{
    uint8_t size = operandPrototype.size / 8;
    int64_t value = 0;

    if (operandPrototype.type == OpType::moffs || operandPrototype.type == OpType::ptr)
        size = 2;

    if (operand->IsDependent())
    {
        switch (operandPrototype.type)
        {
        case OpType::rel:
            generator.MakeRelativeLinkTarget(operand, code->size(), size, code->size() + size);
            break;
        case OpType::imm:
            generator.MakeValueLinkTarget(operand, code->size(), size, ASM::LinkingTarget::Type::Integer);
            break;
        case OpType::ptr:
            generator.MakeAbsoluteLinkTarget(operand, code->size(), size);
            break;
        default:
            generator.MakeAbsoluteLinkTarget(operand, code->size(), size);
            break;
        }
    }
    else
    {
        value = operand->Resolve();
    }

    code.Push(reinterpret_cast<uint8_t*>(&value), size);
}

MachineCode InstructionStmt::CodeGen(CodeGenerator& generator) const 
{
    MachineCode result;
    const Instruction* instructionPrototype = generator.ChooseInstructionByOperands(mnemonic, operands);

    if (instructionPrototype == nullptr)
    {
        generator.GetContext().Error("Invalid operands combination", location, length);

        return std::move(result);
    }

    result.Push(instructionPrototype->opcode.data(), instructionPrototype->opcode.size());

    switch (instructionPrototype->opencode)
    {
    case OpEn::ZO:
    {
        break;
    }
    case OpEn::RM:
    {
        ModRM modrm;
        modrm.reg = operands[0]->GetAs<RegisterExpr>()->GetEncoding();

        EncodeModRM(modrm, operands[1], generator, result);

        break;
    }
    case OpEn::RMI:
    {
        throw std::runtime_error("Not implemented");
        break;
    }
    case OpEn::MR:
    {
        ModRM modrm;
        modrm.reg = operands[1]->GetAs<RegisterExpr>()->GetEncoding();

        EncodeModRM(modrm, operands[0], generator, result);

        break;
    }
    case OpEn::MI:
    {
        //Memory - Imm

        ModRM modrm;
        modrm.reg = static_cast<Register>(instructionPrototype->opcodeExtention);

        EncodeModRM(modrm, operands[0], generator, result);
        EncodeImm(operands[1].get(), instructionPrototype->operands[1], generator, result);

        break;
    }
    case OpEn::FD:
    {
        EncodeImm(operands[1]->GetAs<MemoryExpr>(), instructionPrototype->operands[1], generator, result);
        break;
    }
    case OpEn::TD:
    {
        EncodeImm(operands[0]->GetAs<MemoryExpr>(), instructionPrototype->operands[0], generator, result);
        break;
    }
    case OpEn::OI:
    {
        //Opcode - Imm

        result->back() += static_cast<uint8_t>(operands[0]->GetAs<RegisterExpr>()->GetEncoding());

        EncodeImm(operands[1].get(), instructionPrototype->operands[1], generator, result);

        break;
    }
    case OpEn::D:
    {
        //Relative or Pointer
        //Relative encoding based on the end of instruction code, to calcule relative address use ('absolute address' - 'instruction end position')
        //Pointer encoding uses absolute address in memory

        EncodeImm(operands[0].get(), instructionPrototype->operands[0], generator, result);

        break;
    }
    case OpEn::I:
    {
        //Imm
        //Some instructions use first operand as register, that encoded in opcode, so we should use last operand as imm

        EncodeImm(operands.back().get(), instructionPrototype->operands.back(), generator, result);

        break;
    }
    case OpEn::M:
    {
        //Memory

        ModRM modrm;
        modrm.reg = static_cast<Register>(instructionPrototype->opcodeExtention);

        EncodeModRM(modrm, operands[0], generator, result);

        break;
    }
    case OpEn::O:
    {
        //Opcode

        result->back() += static_cast<uint8_t>(operands[0]->GetAs<RegisterExpr>()->GetEncoding());

        break;
    }
    case OpEn::S:
    {
        //TODO
        throw std::runtime_error("Not implemented");
        break;
    }
    case OpEn::M1: case OpEn::MC:
    {
        //ModRM (only register mode), REG field must be opcode extention 

        ModRM modrm;
        modrm.reg = static_cast<Register>(instructionPrototype->opcodeExtention);

        EncodeModRM(modrm, operands[0], generator, result);

        break;
    }
    default:
        generator.GetContext().Error("Invalid instruction encoding");

        break;
    }

    return std::move(result);
}

MachineCode DefineDataStmt::CodeGen(CodeGenerator& generator) const
{
    MachineCode result;

    for (auto& dataUnit : units) {
        if (dataUnit->Is<LiteralExpr>()) {
            LiteralExpr* literal = dataUnit->GetAs<LiteralExpr>();

            if (literal->IsDependent()) [[likely]] {
                for (auto c : literal->value) {
                    size_t value = c;
                    result.Push(reinterpret_cast<uint8_t*>(&value), dataUnitSize);
                }
            } else {
                int64_t value = literal->Resolve();
                result.Push(reinterpret_cast<uint8_t*>(&value), dataUnitSize);
            }
        }
        else {
            int64_t value = 0;

            if (dataUnit->IsDependent())
                generator.MakeValueLinkTarget(dataUnit.get(), result->size(), dataUnitSize, LinkingTarget::Type::Integer);
            else
                value = dataUnit->Resolve();

            result.Push(reinterpret_cast<uint8_t*>(&value), dataUnitSize);
        }
    }

    return std::move(result);
}

//bool DuplicateStmt::IsDependent() const
//{
//    return valueExpression->IsDependent() && countExpression->IsDependent();
//}
//
//bool DuplicateStmt::Simplify()
//{
//    return valueExpression->Simplify() || countExpression->Simplify();
//}

MachineCode DuplicateStmt::CodeGen(CodeGenerator& generator) const
{
    MachineCode result;
}