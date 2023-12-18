#include "linker.h"

#include <cmath>
#include <limits>

#include "raw-binary.h"

using namespace ASM;
using namespace ASM::AST;

void Linker::EvaluateSymbol(const Symbol& symbol, unsigned int depth) {
    if (symbol.GetDeclaration().Is<ConstantDecl>()) {
        const ConstantDecl* constantDecl = symbol.GetDeclaration().GetAs<ConstantDecl>();
        auto dependencies = constantDecl->GetExpression().GetDependecies();

        for (auto depencency : dependencies)
            if (symbolMap.count(*depencency) < 1) {
                if (depth < maxEvalDepth) {
                    EvaluateSymbol(context->GetSymbolTable().GetSymbol(*depencency), depth + 1);
                }
                else {
                    context->Error(
                        "Unable to evaluate all symbols, two symbols points to each other or recursive evaluating take too much passes",
                        symbol.GetDeclaration().GetLocation(),
                        symbol.GetDeclaration().GetLength()
                    );
                    return;
                }
            }

        int64_t value = constantDecl->GetExpression().Resolve(symbolMap);

        symbolMap.insert({ constantDecl->GetName(), value });
    }
    else if (symbol.GetDeclaration().Is<LableDecl>()) {
        if (symbol.IsEvaluated() == false) [[unlikely]] {
            std::string msg("Unevaluated address symbol at linking stage: \'");
            msg += symbol.GetDeclaration().GetName() + '\''; 

            context->Error(msg.c_str());
            return;
        }

        const LableDecl* lableDecl = symbol.GetDeclaration().GetAs<LableDecl>();
        
        int64_t value = symbol.GetValue().GetAsInt() + context->GetSymbolTable().GetOrigin();

        auto& targetSection = context->GetTranslationUnit().GetSectionMap().at(lableDecl->GetRelatedSection()->GetName());

        for (auto section : sectionOrder) {
            if (section == &targetSection)
                break;

            value += section->GetCode()->size();
        }

        symbolMap.insert({ lableDecl->GetName(), value });
    }
}

void Linker::LinkRawBinary(RawBinary& result) {
    Codegen::MachineCode& code = result.GetCode();

    {
        size_t index = 0;
        sectionOrder.resize(context->GetTranslationUnit().GetSectionMap().size());

        for (auto& pair : context->GetTranslationUnit().GetSectionMap()) {
            sectionOrder[index] = &pair.second;
            ++index;
        }
    }

    for (auto& pair : context->GetSymbolTable().GetSymbolsMap()) {
        if (pair.second.GetDeclaration().Is<SectionDecl>()) {
            int64_t value = 0;

            const SectionDecl* sectionDecl = pair.second.GetDeclaration().GetAs<SectionDecl>();
            auto targetIt = context->GetTranslationUnit().GetSectionMap().find(sectionDecl->GetName());

            for (size_t i = 0; sectionOrder[i] != &targetIt->second; ++i) {
                value += sectionOrder[i]->GetCode()->size();
            }

            symbolMap.insert({ sectionDecl->GetName(), value });
            continue;
        }

        EvaluateSymbol(pair.second);
    }

    for (auto& segment : context->GetTranslationUnit().GetSectionMap())
    {
        size_t sectionBeginCodeIndex = code->size();
        code << segment.second.GetCode();

        for (auto& linkingTarget : segment.second.GetLinkingTargets())
        {
            auto dependencies = linkingTarget.GetExpression()->GetDependecies();
            bool isValid = true;

            for (auto depencency : dependencies)
                if (symbolMap.count(*depencency) == 0) {
                    isValid = false;
                    context->Error("Undefined symbol", linkingTarget.GetExpression()->GetLocation(), linkingTarget.GetExpression()->GetLength());
                    break;
                }

            if (isValid == false) [[unlikely]]
                continue;

            uint64_t maxValueForCurrentSize = std::pow(std::numeric_limits<uint8_t>::max(), linkingTarget.GetSize());
            int64_t value = linkingTarget.GetExpression()->Resolve(symbolMap);

            if (linkingTarget.GetKind() == LinkingTarget::Kind::RelativeAddress)
                value -= (context->GetSymbolTable().GetOrigin() + linkingTarget.GetRelativeOrigin() + sectionBeginCodeIndex);

            if ((value < 0 ? -value : value) > maxValueForCurrentSize)
                context->Error("Value overflow while linking", linkingTarget.GetExpression()->GetLocation(), linkingTarget.GetExpression()->GetLength());

            uint8_t* valuePtr = reinterpret_cast<uint8_t*>(&value);
            
            for (uint8_t i = 0; i < linkingTarget.GetSize(); ++i)
                code[sectionBeginCodeIndex + linkingTarget.GetSectionOffset() + i] = valuePtr[i];
        }
    }
}

std::unique_ptr<AssembledObject> Linker::Link(LinkingFormat format)
{
    std::unique_ptr<AssembledObject> result;

    switch (format)
    {
    case LinkingFormat::RawBinary:
    {
        result = std::make_unique<RawBinary>();
        LinkRawBinary(*reinterpret_cast<RawBinary*>(result.get()));

        break;
    }
    default:
        throw std::runtime_error("Not implemented");
        break;
    }

    return std::move(result);
}