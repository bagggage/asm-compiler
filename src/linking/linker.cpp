#include "linker.h"

#include <cmath>
#include <limits>

#include "raw-binary.h"

using namespace ASM;
using namespace ASM::AST;

const std::unordered_map<std::string, unsigned int> Linker::segmentsPriorityMap =
{
    {".TEXT",  2},
    {".CODE",  2},
    {".DATA",  1},
    {".BSS",   1},
    {".STACK", 0},

    {"TEXT",  2},
    {"CODE",  2},
    {"DATA",  1},
    {"BSS",   1},
    {"STACK", 0}
};

size_t Linker::GetOrderedSectionOffset(const std::string& sectionName) {
    return symbolMap.at('@' + sectionName) * 16;
}

void Linker::EvaluateSymbol(const Symbol& symbol, bool absoluteValue, unsigned int depth) {
    if (symbol.GetDeclaration().Is<ConstantDecl>()) {
        const ConstantDecl* constantDecl = symbol.GetDeclaration().GetAs<ConstantDecl>();
        auto dependencies = constantDecl->GetExpression().GetDependecies();

        for (auto depencency : dependencies)
            if (symbolMap.count(*depencency) < 1) {
                if (depth < maxEvalDepth) {
                    EvaluateSymbol(context->GetSymbolTable().GetSymbol(*depencency), absoluteValue, depth + 1);
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
        int64_t value = symbol.GetValue().GetAsInt();

        if (absoluteValue)
            value += context->GetSymbolTable().GetOrigin() + GetOrderedSectionOffset(lableDecl->GetRelatedSection()->GetName());

        symbolMap.insert({ lableDecl->GetName(), value });
    }
}

void Linker::OrderSections()
{
    size_t index = 0;
    sectionOrder.resize(context->GetTranslationUnit().GetSectionMap().size());

    for (auto& pair : context->GetTranslationUnit().GetSectionMap()) {
        if (pair.second.GetCode()->size() == 0) {
            sectionOrder.pop_back();
            continue;
        }
        sectionOrder[index] = &pair.second;
        ++index;
    }

    std::sort(sectionOrder.begin(), sectionOrder.end(), [](const Section* a, const Section* b)
    {
        unsigned int aPriority = 0;
        unsigned int bPriority = 0;

        if (segmentsPriorityMap.count(a->GetName()) > 0)
            aPriority = segmentsPriorityMap.at(a->GetName());
        if (segmentsPriorityMap.count(b->GetName()) > 0)
            bPriority = segmentsPriorityMap.at(b->GetName());

        return aPriority > bPriority; 
    });

    size_t value = 0;

    for (auto section : sectionOrder) {
        if ((section != sectionOrder.back()) && section->GetCode()->size() % 16 != 0) [[unlikely]]
        {
            uint8_t mod = section->GetCode()->size() % 16;
            uint8_t align = mod > 0 ? (16 - mod) : 0;

            section->GetCode()->resize(section->GetCode()->size() + align, 0);
        }

        symbolMap.insert({ '@' + section->GetName(), value });
        value += section->GetCode()->size() / 16;
    }
}

bool Linker::IsValidDependencies(const Expression* expression, std::vector<const std::string*>& dependencies)
{
    for (auto depencency : dependencies) {
        if (symbolMap.count(*depencency) == 0) [[unlikely]] {
            context->Error("Undefined symbol", expression->GetLocation(), expression->GetLength());
            return false;
        }
    }

    return true;
}

bool Linker::IsValueCompatibleWithSize(int64_t value, const LinkingTarget& linkingTarget)
{
    uint64_t maxValueForCurrentSize = std::pow(std::numeric_limits<uint8_t>::max(), linkingTarget.GetSize());
            
    if ((value < 0 ? -value : value) > maxValueForCurrentSize) [[unlikely]] {
        context->Error("Value overflow while linking", linkingTarget.GetExpression()->GetLocation(), linkingTarget.GetExpression()->GetLength());
        return false;
    }
    else {
        const int64_t min = -(maxValueForCurrentSize / 2);
        const int64_t max = (maxValueForCurrentSize / 2) - 1;

        if (value < min || value > max)
            context->Warn("Signed value may be corrupted", linkingTarget.GetExpression()->GetLocation(), linkingTarget.GetExpression()->GetLength());
    }

    return true;
}

void Linker::LinkRawBinary(RawBinary& result)
{
    Codegen::MachineCode& code = result.GetCode();

    if (context->GetTranslationUnit().GetRequiredStackSize() != 0)
        context->Warn("\'STACK\' statement is not supported with .COM format - ignored");

    OrderSections();

    for (auto& pair : context->GetSymbolTable().GetSymbolsMap())
        EvaluateSymbol(pair.second, true);

    for (auto segment : sectionOrder)
    {
        size_t sectionBeginCodeIndex = code->size();
        code << segment->GetCode();

        for (auto& linkingTarget : segment->GetLinkingTargets())
        {
            auto dependencies = linkingTarget.GetExpression()->GetDependecies();
            if (IsValidDependencies(linkingTarget.GetExpression(), dependencies) == false) [[unlikely]]
                continue;

            int64_t value = linkingTarget.GetExpression()->Resolve(symbolMap);

            if (linkingTarget.GetKind() == LinkingTarget::Kind::RelativeAddress)
                value -= (context->GetSymbolTable().GetOrigin() + linkingTarget.GetRelativeOrigin() + sectionBeginCodeIndex); 

            if (IsValueCompatibleWithSize(value, linkingTarget)) [[likely]]
            {
                uint8_t* valuePtr = reinterpret_cast<uint8_t*>(&value);
                for (uint8_t i = 0; i < linkingTarget.GetSize(); ++i)
                    code[sectionBeginCodeIndex + linkingTarget.GetSectionOffset() + i] = valuePtr[i];
            }
        }
    }
}

void Linker::LinkExe(ExeObject& result)
{
    Codegen::MachineCode& code = result.code;

    OrderSections();

    if (context->GetSymbolTable().GetOrigin() != 0)
        context->Warn("Origin offset not allowed with .EXE format - ignored");

    for (auto& pair : context->GetSymbolTable().GetSymbolsMap())
        EvaluateSymbol(pair.second, false);

    for (auto segment : sectionOrder)
    {
        size_t sectionBeginCodeIndex = code->size();
        code << segment->GetCode();

        for (auto& linkingTarget : segment->GetLinkingTargets())
        {
            auto dependencies = linkingTarget.GetExpression()->GetDependecies();
            if (IsValidDependencies(linkingTarget.GetExpression(), dependencies) == false) [[unlikely]]
                continue;

            for (auto depencency : dependencies)
            {
                if (depencency->at(0) == '@' && context->GetTranslationUnit().GetSectionMap().count(depencency->c_str() + 1) > 0) {
                    result.relocationTable.push_back({ static_cast<uint16_t>(linkingTarget.GetSectionOffset() + sectionBeginCodeIndex), 0 });
                    break;
                }
            }

            int64_t value = linkingTarget.GetExpression()->Resolve(symbolMap);

            if (linkingTarget.GetKind() == LinkingTarget::Kind::RelativeAddress)
                value -= (linkingTarget.GetRelativeOrigin() + sectionBeginCodeIndex); 

            if (IsValueCompatibleWithSize(value, linkingTarget)) [[likely]]
            {
                uint8_t* valuePtr = reinterpret_cast<uint8_t*>(&value);
                for (uint8_t i = 0; i < linkingTarget.GetSize(); ++i)
                    code[sectionBeginCodeIndex + linkingTarget.GetSectionOffset() + i] = valuePtr[i];
            }
        }
    }

    if (context->GetTranslationUnit().GetRequiredStackSize() == 0) {
        context->Warn("Stack missing");
    }
    else {
        result.mzHeader.initialRelativeSS = code->size();
        
        if (code->size() % result.paragraphByteSize > 0)
            result.mzHeader.initialRelativeSS += (result.paragraphByteSize - (code->size() % result.paragraphByteSize));

        result.mzHeader.initialRelativeSS /= 16;
        result.mzHeader.initialSp = context->GetTranslationUnit().GetRequiredStackSize();
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
    case LinkingFormat::DosExecutable:
    {
        result = std::make_unique<ExeObject>();
        LinkExe(*reinterpret_cast<ExeObject*>(result.get()));
        break;
    }
    default:
        throw std::runtime_error("Not implemented");
        break;
    }

    return std::move(result);
}