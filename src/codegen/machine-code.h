#ifndef __ASM_MACHINE_CODE_H
#define __ASM_MACHINE_CODE_H

#include <cstdint>
#include <cstddef>
#include <vector>

namespace ASM::Codegen
{
    struct MachineCode
    {
        MachineCode() = default;
        MachineCode(size_t size) : code(size) {}
        MachineCode(const std::vector<uint8_t>& otherCode);

        std::vector<uint8_t> code;

        MachineCode& operator<<(uint8_t byte);
        MachineCode& operator<<(const MachineCode& otherCode);
        MachineCode& operator<<(const std::vector<uint8_t>& data);

        void Push(const uint8_t* data, size_t size);

        inline uint8_t& operator[](size_t index)
        {
            return code[index];
        }

        inline const uint8_t& operator[](size_t index) const
        {
            return code[index];
        }

        inline std::vector<uint8_t>* operator->()
        {
            return &code;
        }

        inline const std::vector<uint8_t>* operator->() const
        {
            return &code;
        }
    };
}

#endif