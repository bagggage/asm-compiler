#include "machine-code.h"

using namespace ASM::Codegen;

MachineCode& MachineCode::operator<<(uint8_t byte)
{
    code.push_back(byte);

    return *this;
}

MachineCode& MachineCode::operator<<(const MachineCode& otherCode)
{
    code.insert(code.end(), otherCode.code.begin(), otherCode.code.end());

    return *this;
}

MachineCode& MachineCode::operator<<(const std::vector<uint8_t>& data)
{
    code.insert(code.end(), data.begin(), data.end());

    return *this;
}

void MachineCode::Push(const uint8_t* data, size_t size)
{
    code.insert(code.end(), data, data + size);
}