#include "object-file.h"

using namespace ASM;

bool ObjectFile::Deserialize(std::istream& stream)
{
}

bool ObjectFile::Serialize(std::ostream& stream) const
{
    //Sections
    auto& sectionMap = context->GetTranslationUnit().GetSectionMap();
    size_t sectionsCount = sectionMap.size();

    stream.write(reinterpret_cast<char*>(&sectionsCount), sizeof(sectionsCount));
    
    for (auto& section : sectionMap)
    {
        //Section name
        size_t size = section.first.size();
        stream.write(reinterpret_cast<char*>(&size), sizeof(size));
        stream.write(section.first.c_str(), size);
        
        //Code
        size = section.second.GetCode()->size();
        stream.write(reinterpret_cast<char*>(&size), sizeof(size));
        stream.write(reinterpret_cast<char*>(section.second.GetCode()->data()), size);

        //Linking targets
        size = section.second.GetLinkingTargets().size();
        stream.write(reinterpret_cast<char*>(&size), sizeof(size));
        
        for (auto& target : section.second.GetLinkingTargets())
        {
            auto kind = target.GetKind();
            stream.write(reinterpret_cast<char*>(&kind), sizeof(kind));
            auto type = target.GetType();
            stream.write(reinterpret_cast<char*>(&type), sizeof(type));
            auto size = target.GetSize();
            stream.write(reinterpret_cast<char*>(&size), sizeof(size));
            auto relativeOrigin = target.GetRelativeOrigin();
            stream.write(reinterpret_cast<char*>(&relativeOrigin), sizeof(relativeOrigin));
            auto sectionOffset = target.GetSectionOffset();
            stream.write(reinterpret_cast<char*>(&sectionOffset), sizeof(sectionOffset));
        }
    }

    //Symbol table
    auto& symbolMap = context->GetSymbolTable().GetSymbolsMap();
    for (auto& symbol : symbolMap)
    {
        //TODO: Put declaration
        //---------------------

        auto isEvaluated = symbol.second.IsEvaluated();
        stream.write(reinterpret_cast<char*>(&isEvaluated), sizeof(isEvaluated));

        if (isEvaluated) {
            auto kind = symbol.second.GetValue().GetKind();
            stream.write(reinterpret_cast<char*>(&kind), sizeof(kind));
            auto value = symbol.second.GetValue().GetAsInt();
            stream.write(reinterpret_cast<char*>(&value), sizeof(value));
        }
    }

    return true;
}