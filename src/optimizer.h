#include <cstddef>
#include <cinttypes>

class Optimizer
{
    virtual int64_t Resolve(const NumberExpr& numberExpr);
}