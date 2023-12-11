#ifndef __ASM_SYMBOL_TABLE_H
#define __ASM_SYMBOL_TABLE_H

#include <unordered_map>

#include "symbol.h"

namespace ASM
{
    class SymbolTable
    {
    private:
        std::unordered_map<std::string, Symbol> symbolsMap;

        size_t origin = 0;
    public:
        inline void AddSymbol(Symbol&& symbol) { symbolsMap[symbol.GetDeclaration().GetName()] = symbol; }
        inline void EvaluateSymbol(const std::string& symbolName, SymbolValue& value)
        {
            symbolsMap.at(symbolName).Evaluate(value);
        }

        inline void EvaluateSymbol(const std::string& symbolName, SymbolValue&& value)
        {
            symbolsMap.at(symbolName).Evaluate(value);
        }

        inline const std::unordered_map<std::string, Symbol>& GetSymbolsMap() const {
            return symbolsMap;
        }

        inline const Symbol& GetSymbol(const std::string& symbolName) const { return symbolsMap.at(symbolName); }

        inline bool HasSymbol(const std::string& symbolName) const { return symbolsMap.count(symbolName) > 0; }

        inline size_t GetOrigin() const { return origin; }
        inline void SetOrigin(size_t value) { origin = value; }
    };
}

#endif