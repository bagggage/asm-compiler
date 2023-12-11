#ifndef __AST_H
#define __AST_H

#include "statements.h"
#include "expressions.h"
#include "declarations.h"

namespace ASM
{
    using AbstractSyntaxTree = std::vector<std::unique_ptr<ASM::AST::Node>>;
}

#endif