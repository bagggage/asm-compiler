#ifndef __ASM_TRANSLATION_UNIT
#define __ASM_TRANSLATION_UNIT

#include <unordered_map>
#include <map>
#include <string>

#include "section.h"

namespace ASM
{
    class TranslationUnit
    {
    private:
        std::unordered_map<std::string, Section> sections;

        size_t addressSymbolsOrigin = 0;
        size_t requiredStackSize = 0;
    public:
        inline std::unordered_map<std::string, Section>& GetSectionMap() { return sections; }

        inline Section& GetOrMakeSection(const std::string_view& name)
        { 
            return GetOrMakeSection(std::string(name.data()));
        }

        inline Section& GetOrMakeSection(const std::string& name)
        {
            if (sections.count(name.data()) == 0) [[unlikely]]
                sections.insert({ name.data(), Section(name.data()) });

            return sections.at(name);
        }

        inline size_t GetRequiredStackSize() const { return requiredStackSize; }
        inline void SetStackSize(size_t size) { requiredStackSize = size; }
    };
}

#endif