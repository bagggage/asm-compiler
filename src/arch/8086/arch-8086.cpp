#include "arch-8086.h"

using namespace ASM::Arch;

const std::unordered_map<std::string, std::vector<Instruction>> Arch8086::InstructionSet = 
{
    { "AAA", {{{0x37}, OpEn::ZO, {}}} },
    {
        "AAD",
        {
            {{0xD5, 0x0A}, OpEn::ZO, {}},
            {{0xD5},       OpEn::ZO, {}}
        }
    },
    {
        "AAM",
        {
            {{0xD4, 0x0A}, OpEn::ZO, {}},
            {{0xD4},       OpEn::ZO, {}}
        }
    },
    {
        "ADD",
        {
            {{0x04}, OpEn::I,  {{OpType::AL},     {OpType::imm, 8}}},
            {{0x05}, OpEn::I,  {{OpType::AX},     {OpType::imm, 16}}},
            {{0x80}, OpEn::MI, {{OpType::rm, 8},  {OpType::imm, 8}},  '\0'},
            {{0x83}, OpEn::MI, {{OpType::rm, 16}, {OpType::imm, 8}},  '\0'},
            {{0x81}, OpEn::MI, {{OpType::rm, 16}, {OpType::imm, 16}}, '\0'},
            {{0x00}, OpEn::MR, {{OpType::rm, 8},  {OpType::r,   8}}},
            {{0x01}, OpEn::MR, {{OpType::rm, 16}, {OpType::r,   16}}},
            {{0x02}, OpEn::RM, {{OpType::r,  8},  {OpType::rm,  8}}},
            {{0x03}, OpEn::RM, {{OpType::r,  16}, {OpType::rm,  16}}}
        }
    },
    {
        "AND",
        {
            {{0x24}, OpEn::I,  {{OpType::AL},     {OpType::imm, 8}}},
            {{0x25}, OpEn::I,  {{OpType::AX},     {OpType::imm, 16}}},
            {{0x80}, OpEn::MI, {{OpType::rm, 8},  {OpType::imm, 8}},  '\4'},
            {{0x81}, OpEn::MI, {{OpType::rm, 16}, {OpType::imm, 16}}, '\4'},
            {{0x83}, OpEn::MI, {{OpType::rm, 16}, {OpType::imm, 8}},  '\4'},
            {{0x20}, OpEn::MR, {{OpType::rm, 8},  {OpType::r,   8}}},
            {{0x21}, OpEn::MR, {{OpType::rm, 16}, {OpType::r,   16}}},
            {{0x22}, OpEn::RM, {{OpType::r,  8},  {OpType::rm,  8}}},
            {{0x23}, OpEn::RM, {{OpType::r,  16}, {OpType::rm,  16}}}
        }
    },
    {
        "BT",
        {
            {{0x0F, 0xA3}, OpEn::MR, {{OpType::rm, 16}, {OpType::r,   16}}},
            {{0x0F, 0xBA}, OpEn::MI, {{OpType::r,  16}, {OpType::imm, 8}}, '\4'}
        }
    },
    {
        "BTC",
        {
            {{0x0F, 0xBB}, OpEn::MR, {{OpType::rm, 16}, {OpType::r, 16}}},
            {{0x0F, 0xBA}, OpEn::MI, {{OpType::rm, 16}, {OpType::imm, 8}}, '\7'}
        }
    },
    {
        "CALL",
        {
            {{0xE8}, OpEn::D, {{OpType::rel, 16}}},
            {{0xFF}, OpEn::M, {{OpType::rm,  16}}, '\2'},
            {{0x9A}, OpEn::D, {{OpType::ptr, 16}}},
            {{0xFF}, OpEn::M, {{OpType::m,   16}}, '\3'}
        }
    },
    { "CLC", {{{0xF8}, OpEn::ZO, {}}} },
    {
        "CMP",
        {
            {{0x3C}, OpEn::I,  {{OpType::AL},     {OpType::imm, 8}}},
            {{0x3D}, OpEn::I,  {{OpType::AX},     {OpType::imm, 16}}},
            {{0x3D}, OpEn::I,  {{OpType::EAX},    {OpType::imm, 32}}},
            {{0x80}, OpEn::MI, {{OpType::rm, 8},  {OpType::imm, 8}},  '\7'},
            {{0x81}, OpEn::MI, {{OpType::rm, 16}, {OpType::imm, 16}}, '\7'},
            {{0x83}, OpEn::MI, {{OpType::rm, 16}, {OpType::imm, 8}},  '\7'},
            {{0x38}, OpEn::MR, {{OpType::rm, 8},  {OpType::r,   8}}},
            {{0x39}, OpEn::MR, {{OpType::rm, 16}, {OpType::r,   16}}},
            {{0x3A}, OpEn::RM, {{OpType::r,  8},  {OpType::rm, 8}}},
            {{0x3B}, OpEn::RM, {{OpType::r,  16}, {OpType::rm, 16}}}
        }
    },
    {
        "CPUID",
        {
            {{0x0F, 0xA2}, OpEn::ZO, {}}
        }
    },
    {
        "DEC",
        {
            {{0xFE}, OpEn::M, {{OpType::rm, 8}},  '\1'},
            {{0xFF}, OpEn::M, {{OpType::rm, 16}}, '\1'},
            {{0x48}, OpEn::O, {{OpType::r,  16}}}
        }
    },
    {
        "DIV",
        {
            {{0xF6}, OpEn::M, {{OpType::rm, 8}},  '\6'},
            {{0xF7}, OpEn::M, {{OpType::rm, 16}}, '\6'}
        }
    },
    {
        "IDIV",
        {
            {{0xF6}, OpEn::M, {{OpType::rm, 8}},  '\7'},
            {{0xF7}, OpEn::M, {{OpType::rm, 16}}, '\7'}
        }
    },
    {
        "IMUL",
        {
            {{0xF6},       OpEn::M,   {{OpType::rm, 8}},  '\5'},
            {{0xF7},       OpEn::M,   {{OpType::rm, 16}}, '\5'},
            {{0x0F, 0xAF}, OpEn::RM,  {{OpType::r,  16}, {OpType::rm, 16}}},
            {{0x6B},       OpEn::RMI, {{OpType::r,  16}, {OpType::rm, 16}, {OpType::imm, 8}}},
            {{0x69},       OpEn::RMI, {{OpType::r,  16}, {OpType::rm, 16}, {OpType::imm, 16}}}
        }
    },
    {
        "IN",
        {
            {{0xE4}, OpEn::I,  {{OpType::AL}, {OpType::imm, 8}}},
            {{0xE5}, OpEn::I,  {{OpType::AX}, {OpType::imm, 8}}},
            {{0xEC}, OpEn::ZO, {{OpType::AL}, {OpType::DX}}},
            {{0xED}, OpEn::ZO, {{OpType::AX}, {OpType::DX}}}
        }
    },
    {
        "INC",
        {
            {{0xFE}, OpEn::M, {{OpType::rm, 8}},  '\0'},
            {{0xFF}, OpEn::M, {{OpType::rm, 16}}, '\0'},
            {{0x40}, OpEn::O, {{OpType::r,  16}}, '\0'}
        }
    },
    { "INT",  {{ {0xCD}, OpEn::I,  {{OpType::imm, 8}} }} },
    { "INT0", {{ {0xCE}, OpEn::ZO, {} }} },
    { "INT1", {{ {0xF1}, OpEn::ZO, {} }} },
    { "INT3", {{ {0xCC}, OpEn::ZO, {} }} },
    { "IRET", {{ {0xCF}, OpEn::ZO, {} }} },
    { "IRED", {{ {0xCF}, OpEn::ZO, {} }} },
    {
        "JA",
        {
            {{0x77},       OpEn::D, {{OpType::rel, 8}}},
            {{0x0F, 0x87}, OpEn::D, {{OpType::rel, 16}}}
        }
    },
    {
        "JAE",
        {
            {{0x73},       OpEn::D, {{OpType::rel, 8}}},
            {{0x0F, 0x83}, OpEn::D, {{OpType::rel, 16}}}
        } 
    },
    {
        "JB",
        {
            {{0x72},       OpEn::D, {{OpType::rel, 8}}},
            {{0x0F, 0x82}, OpEn::D, {{OpType::rel, 16}}}
        }
    },
    {
        "JBE",
        {
            {{0x76},       OpEn::D, {{OpType::rel, 8}}},
            {{0x0F, 0x86}, OpEn::D, {{OpType::rel, 16}}}
        } 
    },
    {
        "JC",
        {
            {{0x72},       OpEn::D, {{OpType::rel, 8}}},
            {{0x0F, 0x82}, OpEn::D, {{OpType::rel, 16}}}
        }
    },
    { "JCXZ",  {{ {0xE3}, OpEn::D, {{OpType::rel, 8}} }} },
    { "JECXZ", {{ {0xE3}, OpEn::D, {{OpType::rel, 8}} }} },
    { 
        "JE",
        {
            {{0x74},       OpEn::D, {{OpType::rel, 8}}},
            {{0x0F, 0x84}, OpEn::D, {{OpType::rel, 16}}}
        }
    },
    {
        "JG",
        {
            {{0x7F},       OpEn::D, {{OpType::rel, 8}}},
            {{0x0F, 0x8F}, OpEn::D, {{OpType::rel, 16}}}
        }
    },
    {
        "JGE",
        {
            {{0x7D},       OpEn::D, {{OpType::rel, 8}}},
            {{0x0F, 0x8D}, OpEn::D, {{OpType::rel, 16}}}
        }
    },
    {
        "JL",
        {
            {{0x7C},       OpEn::D, {{OpType::rel, 8}} },
            {{0x0F, 0x8C}, OpEn::D, {{OpType::rel, 16}}}
        }
    },
    {
        "JLE",
        {
            {{0x7E},       OpEn::D, {{OpType::rel, 8}}},
            {{0x0F, 0x8E}, OpEn::D, {{OpType::rel, 16}}}
        }
    },
    {
        "JNA",
        {
            {{0x76},       OpEn::D, {{OpType::rel, 8}}},
            {{0x0F, 0x86}, OpEn::D, {{OpType::rel, 16}}}
        }
    },
    {
        "JNAE",
        {
            {{0x72},       OpEn::D, {{OpType::rel, 8}}},
            {{0x0F, 0x82}, OpEn::D, {{OpType::rel, 16}}}
        }
    },
    {
        "JNB", 
        {
            {{0x73},       OpEn::D, {{OpType::rel, 8}}},
            {{0x0F, 0x83}, OpEn::D, {{OpType::rel, 16}}}
        }
    },
    {
        "JNBE",
        {
            {{0x77},       OpEn::D, {{OpType::rel, 8}}},
            {{0x0F, 0x87}, OpEn::D, {{OpType::rel, 16}}}
        }
    },
    {
        "JNC", 
        {
            {{0x73},       OpEn::D, {{OpType::rel, 8}}},
            {{0x0F, 0x83}, OpEn::D, {{OpType::rel, 16}}}
        }
    },
    {
        "JNE", 
        {
            {{0x75},       OpEn::D, {{OpType::rel, 8}}},
            {{0x0F, 0x85}, OpEn::D, {{OpType::rel, 16}}}
        }
    },
    {
        "JNG", 
        {
            {{0x7E},       OpEn::D, {{OpType::rel, 8}}},
            {{0x0F, 0x8E}, OpEn::D, {{OpType::rel, 16}}}
        }
    },
    {
        "JNGE",
        {
            {{0x7C},       OpEn::D, {{OpType::rel, 8}}},
            {{0x0F, 0x8C}, OpEn::D, {{OpType::rel, 16}}}
        }
    },
    {
        "JNL", 
        {
            {{0x7D},       OpEn::D, {{OpType::rel, 8}}},
            {{0x0F, 0x8D}, OpEn::D, {{OpType::rel, 16}}}
        }
    },
    {
        "JNLE",
        {
            {{0x7F},       OpEn::D, {{OpType::rel, 8}}},
            {{0x0F, 0x8F}, OpEn::D, {{OpType::rel, 16}}}
        }
    },
    {
        "JNO", 
        {
            {{0x71},       OpEn::D, {{OpType::rel, 8}}},
            {{0x0F, 0x81}, OpEn::D, {{OpType::rel, 16}}}
        }
    },
    {
        "JNP", 
        {
            {{0x7B},       OpEn::D, {{OpType::rel, 8}}},
            {{0x0F, 0x8B}, OpEn::D, {{OpType::rel, 16}}}
        }
    },
    {
        "JNS", 
        {
            {{0x79},       OpEn::D, {{OpType::rel, 8}}},
            {{0x0F, 0x89}, OpEn::D, {{OpType::rel, 16}}}
        }
    },
    {
        "JNZ", 
        {
            {{0x75},       OpEn::D, {{OpType::rel, 8}}},
            {{0x0F, 0x85}, OpEn::D, {{OpType::rel, 16}}}
        }
    },
    {
        "JO",  
        {
            {{0x70},       OpEn::D, {{OpType::rel, 8}}},
            {{0x0F, 0x80}, OpEn::D, {{OpType::rel, 16}}}
        }
    },
    {
        "JP",  
        {
            {{0x7A},       OpEn::D, {{OpType::rel, 8}}},
            {{0x0F, 0x8A}, OpEn::D, {{OpType::rel, 16}}}
        }
    },
    {
        "JPE", 
        {
            {{0x7A},       OpEn::D, {{OpType::rel, 8}}},
            {{0x0F, 0x8A}, OpEn::D, {{OpType::rel, 16}}}
        }
    },
    {
        "JPO", 
        {
            {{0x7B},       OpEn::D, {{OpType::rel, 8}}},
            {{0x0F, 0x8B}, OpEn::D, {{OpType::rel, 16}}}
        }
    },
    {
        "JS",  
        {
            {{0x78},       OpEn::D, {{OpType::rel, 8}}},
            {{0x0F, 0x88}, OpEn::D, {{OpType::rel, 16}}}
        }
    },
    {
        "JZ",  
        {
            {{0x74},       OpEn::D, {{OpType::rel, 8}}},
            {{0x0F, 0x84}, OpEn::D, {{OpType::rel, 16}}}
        }
    },
    {
        "JMP",
        {
            {{0xEB}, OpEn::D, {{OpType::rel, 8}}},
            {{0xE9}, OpEn::D, {{OpType::rel, 16}}},
            {{0xFF}, OpEn::D, {{OpType::rm,  16}}, '\4'},
            {{0xEA}, OpEn::S, {{OpType::ptr, 16}}},
            {{0xFF}, OpEn::M, {{OpType::rm,  16}}, '\5'}   
        }
    },
    { "LEA",    {{ {0x8D}, OpEn::RM, {{OpType::r, 16}, {OpType::m}} }} },
    { "LEAVE",  {{ {0xC9}, OpEn::ZO, {} }} },
    { "LOOP",   {{ {0xE2}, OpEn::D,  {{OpType::rel, 8}} }} },
    { "LOOPE",  {{ {0xE1}, OpEn::D,  {{OpType::rel, 8}} }} },
    { "LOOPNE", {{ {0xE2}, OpEn::D,  {{OpType::rel, 8}} }} },
    {
        "MOV",
        {
            {{0x88}, OpEn::MR, {{OpType::rm,    8},  {OpType::r,     8}}},
            {{0x89}, OpEn::MR, {{OpType::rm,    16}, {OpType::r,     16}}},
            {{0x89}, OpEn::MR, {{OpType::rm,    32}, {OpType::r,     32}}},
            {{0x8A}, OpEn::RM, {{OpType::r,     8},  {OpType::rm,    8}}},
            {{0x8B}, OpEn::RM, {{OpType::r,     16}, {OpType::rm,    16}}},
            {{0x8B}, OpEn::RM, {{OpType::r,     32}, {OpType::rm,    32}}},
            {{0x8C}, OpEn::MR, {{OpType::rm,    16}, {OpType::sreg     }}},
            {{0x8E}, OpEn::RM, {{OpType::sreg},      {OpType::rm,    16}}},
            {{0xA0}, OpEn::FD, {{OpType::AL,    8},  {OpType::moffs, 8}}},
            {{0xA1}, OpEn::FD, {{OpType::AX,    16}, {OpType::moffs, 16}}},
            {{0xA2}, OpEn::TD, {{OpType::moffs, 8},  {OpType::AL, 8}}},
            {{0xA3}, OpEn::TD, {{OpType::moffs, 16}, {OpType::AX, 16}}},
            {{0xB0}, OpEn::OI, {{OpType::r,     8},  {OpType::imm,   8}}},
            {{0xB8}, OpEn::OI, {{OpType::r,     16}, {OpType::imm,   16}}},
            {{0xB8}, OpEn::OI, {{OpType::r,     32}, {OpType::imm,   32}}},
            {{0xC6}, OpEn::MI, {{OpType::rm,    8},  {OpType::imm,   8}},  '\0'},
            {{0xC7}, OpEn::MI, {{OpType::rm,    16}, {OpType::imm,   16}}, '\0'},
            {{0xC7}, OpEn::MI, {{OpType::rm,    32}, {OpType::imm,   32}}, '\0'},
            {{0x0F, 0x20}, OpEn::MR, {{OpType::r, 32},  {OpType::creg}}},
            {{0x0F, 0x22}, OpEn::RM, {{OpType::creg},   {OpType::r, 32}}}
        }
    },
    {
        "MUL",
        {
            {{0xF6}, OpEn::M, {{OpType::rm, 8}},  '\4'},
            {{0xF7}, OpEn::M, {{OpType::rm, 16}}, '\4'}
        }
    },
    { "NOP", {{{0x90}, OpEn::ZO, {}}} },
    {
        "NOT",
        {
            {{0xF6}, OpEn::M, {{OpType::rm, 8}},  '\2'},
            {{0xF7}, OpEn::M, {{OpType::rm, 16}}, '\2'}
        }
    },
    {
        "OR",
        {
            {{0x0C}, OpEn::I,  {{OpType::AL},     {OpType::imm, 8}}},
            {{0x0D}, OpEn::I,  {{OpType::AX},     {OpType::imm, 16}}},
            {{0x80}, OpEn::MI, {{OpType::rm, 8},  {OpType::imm, 8}},  '\1'},
            {{0x81}, OpEn::MI, {{OpType::rm, 16}, {OpType::imm, 16}}, '\1'},
            {{0x83}, OpEn::MI, {{OpType::rm, 16}, {OpType::imm, 8}},  '\1'},
            {{0x08}, OpEn::MR, {{OpType::rm, 8},  {OpType::r, 8}}},
            {{0x09}, OpEn::MR, {{OpType::rm, 16}, {OpType::r, 16}}},
            {{0x0A}, OpEn::RM, {{OpType::r,  8},  {OpType::rm, 8}}},
            {{0x0B}, OpEn::RM, {{OpType::r,  16}, {OpType::rm, 16}}}
        }
    },
    {
        "OUT",
        {
            {{0xE6}, OpEn::I, {{OpType::imm, 8}, {OpType::AL}}},
            {{0xE7}, OpEn::I, {{OpType::imm, 8}, {OpType::AX}}},
            {{0xEE}, OpEn::I, {{OpType::DX},     {OpType::AL}}},
            {{0xEF}, OpEn::I, {{OpType::DX},     {OpType::AX}}}
        }
    },
    {
        "POP",
        {
            {{0x8F}, OpEn::M,  {{OpType::rm, 16}}, '\0'},
            {{0x58}, OpEn::O,  {{OpType::r,  16}}},
            {{0x1F}, OpEn::ZO, {{OpType::DS}}},
            {{0x07}, OpEn::ZO, {{OpType::ES}}},
            {{0x17}, OpEn::ZO, {{OpType::SS}}},
            {{0x0F, 0xA1}, OpEn::ZO, {{OpType::FS}}},
            {{0x0F, 0xA9}, OpEn::ZO, {{OpType::GS}}},
        }
    },
    { "POPA", {{{0x61}, OpEn::ZO, {}}} },
    { "POPAD", {{{0x61}, OpEn::ZO, {}}} },
    {
        "PUSH",
        {
            {{0xFF}, OpEn::M,  {{OpType::rm,  16}}, '\6'},
            {{0x50}, OpEn::O,  {{OpType::r,   16}}},
            {{0x6A}, OpEn::I,  {{OpType::imm, 8}}},
            {{0x68}, OpEn::I,  {{OpType::imm, 16}}},
            {{0x0E}, OpEn::ZO, {{OpType::CS}}},
            {{0x16}, OpEn::ZO, {{OpType::SS}}},
            {{0x1E}, OpEn::ZO, {{OpType::DS}}},
            {{0x06}, OpEn::ZO, {{OpType::ES}}},
            {{0x0F, 0xA0}, OpEn::ZO, {{OpType::FS}}},
            {{0x0F, 0xA8}, OpEn::ZO, {{OpType::GS}}},
        }
    },
    { "PUSHA",  {{ {0x60}, OpEn::ZO, {} }} },
    { "PUSHAD", {{ {0x60}, OpEn::ZO, {} }} },
    { "PUSHF",  {{ {0x9C}, OpEn::ZO, {} }} },
    { "PUSHFD", {{ {0x9C}, OpEn::ZO, {} }} },
    {
        "RET",
        {
            {{0xC3}, OpEn::ZO, {}}
        }
    },
    {
        "SAL",
        {
            {{0xD0}, OpEn::M1, {{OpType::rm, 8},  {OpType::ONE}},     '\4'},
            {{0xD2}, OpEn::MC, {{OpType::rm, 8},  {OpType::CL}},      '\4'},
            {{0xC0}, OpEn::MC, {{OpType::rm, 8},  {OpType::imm, 8}},  '\4'},
            {{0xD1}, OpEn::M1, {{OpType::rm, 16}, {OpType::ONE}},     '\4'},
            {{0xD3}, OpEn::MC, {{OpType::rm, 16}, {OpType::CL}},      '\4'},
            {{0xC1}, OpEn::MI, {{OpType::rm, 16}, {OpType::imm, 8}},  '\4'}
        }
    },
    {
        "SAR",
        {
            {{0xD0}, OpEn::M1, {{OpType::rm, 8},  {OpType::ONE}},     '\7'},
            {{0xD2}, OpEn::MC, {{OpType::rm, 8},  {OpType::CL}},      '\7'},
            {{0xC0}, OpEn::MC, {{OpType::rm, 8},  {OpType::imm, 8}},  '\7'},
            {{0xD1}, OpEn::M1, {{OpType::rm, 16}, {OpType::ONE}},     '\7'},
            {{0xD3}, OpEn::MC, {{OpType::rm, 16}, {OpType::CL}},      '\7'},
            {{0xC1}, OpEn::MI, {{OpType::rm, 16}, {OpType::imm, 8}},  '\7'}
        }
    },
    {
        "SHL",
        {
            {{0xD0}, OpEn::M1, {{OpType::rm, 8},  {OpType::ONE}},     '\4'},
            {{0xD2}, OpEn::MC, {{OpType::rm, 8},  {OpType::CL}},      '\4'},
            {{0xC0}, OpEn::MC, {{OpType::rm, 8},  {OpType::imm, 8}},  '\4'},
            {{0xD1}, OpEn::M1, {{OpType::rm, 16}, {OpType::ONE}},     '\4'},
            {{0xD3}, OpEn::MC, {{OpType::rm, 16}, {OpType::CL}},      '\4'},
            {{0xC1}, OpEn::MI, {{OpType::rm, 16}, {OpType::imm, 8}},  '\4'}
        }
    },
    {
        "SHR",
        {
            {{0xD0}, OpEn::M1, {{OpType::rm, 8},  {OpType::ONE}},     '\5'},
            {{0xD2}, OpEn::MC, {{OpType::rm, 8},  {OpType::CL}},      '\5'},
            {{0xC0}, OpEn::MC, {{OpType::rm, 8},  {OpType::imm, 8}},  '\5'},
            {{0xD1}, OpEn::M1, {{OpType::rm, 16}, {OpType::ONE}},     '\5'},
            {{0xD3}, OpEn::MC, {{OpType::rm, 16}, {OpType::CL}},      '\5'},
            {{0xC1}, OpEn::MI, {{OpType::rm, 16}, {OpType::imm, 8}},  '\5'}
        }
    },
    { "STC", {{{0xF9}, OpEn::ZO, {}}} },
    {
        "SUB",
        {
            {{0x2C}, OpEn::I,  {{OpType::AL},     {OpType::imm, 8}}},
            {{0x2D}, OpEn::I,  {{OpType::AX},     {OpType::imm, 16}}},
            {{0x2D}, OpEn::I,  {{OpType::EAX},    {OpType::imm, 32}}},
            {{0x80}, OpEn::MI, {{OpType::rm, 8},  {OpType::imm, 8}},  '\5'},
            {{0x81}, OpEn::MI, {{OpType::rm, 16}, {OpType::imm, 16}}, '\5'},
            {{0x83}, OpEn::MI, {{OpType::rm, 16}, {OpType::imm, 8}},  '\5'},
            {{0x28}, OpEn::MR, {{OpType::rm, 8},  {OpType::r,   8}}},
            {{0x29}, OpEn::MR, {{OpType::rm, 16}, {OpType::r,   16}}},
            {{0x2A}, OpEn::RM, {{OpType::r,  8},  {OpType::rm,  8}}},
            {{0x2B}, OpEn::RM, {{OpType::r,  16}, {OpType::rm, 16}}}
        }
    },
    {
        "TEST",
        {
            {{0xA8}, OpEn::I,  {{OpType::AL},     {OpType::imm, 8}}},
            {{0xA9}, OpEn::I,  {{OpType::AX},     {OpType::imm, 16}}},
            {{0xF6}, OpEn::MI, {{OpType::rm, 8},  {OpType::imm, 8}},  '\0'},
            {{0xF7}, OpEn::MI, {{OpType::rm, 16}, {OpType::imm, 16}}, '\0'},
            {{0x84}, OpEn::MR, {{OpType::rm, 8},  {OpType::r,   8}}},
            {{0x85}, OpEn::MR, {{OpType::rm, 16}, {OpType::r,   16}}}
        }
    },
    {
        "XCHG",
        {
            {{0x90}, OpEn::O,  {{OpType::AX},     {OpType::r,  16}}},
            {{0x90}, OpEn::O,  {{OpType::r,  16}, {OpType::AX}}},
            {{0x86}, OpEn::MR, {{OpType::rm, 8},  {OpType::r,  8}}},
            {{0x86}, OpEn::RM, {{OpType::r,  8},  {OpType::rm, 8}}},
            {{0x87}, OpEn::MR, {{OpType::rm, 16}, {OpType::r,  16}}},
            {{0x87}, OpEn::RM, {{OpType::r,  16}, {OpType::rm, 16}}}
        }
    },
    {
        "XOR",
        {
            {{0x34}, OpEn::I,  {{OpType::AL},     {OpType::imm, 8}}},
            {{0x35}, OpEn::I,  {{OpType::AX},     {OpType::imm, 16}}},
            {{0x35}, OpEn::I,  {{OpType::EAX},    {OpType::imm, 32}}},
            {{0x80}, OpEn::MI, {{OpType::rm, 8},  {OpType::imm, 8}},  '\6'},
            {{0x81}, OpEn::MI, {{OpType::rm, 16}, {OpType::imm, 16}}, '\6'},
            {{0x83}, OpEn::MI, {{OpType::rm, 16}, {OpType::imm, 8}},  '\6'},
            {{0x30}, OpEn::MR, {{OpType::rm, 8},  {OpType::r,   8}}},
            {{0x31}, OpEn::MR, {{OpType::rm, 16}, {OpType::r,   16}}},
            {{0x32}, OpEn::RM, {{OpType::r,  8},  {OpType::rm,  8}}},
            {{0x33}, OpEn::RM, {{OpType::r,  16}, {OpType::rm,  16}}},
        }
    }
};

const std::unordered_map<std::string, uint8_t> Arch8086::DefineDataMnemonics = 
{
    {"DB", 1}, {"DW", 2}, {"DD", 4}, {"DQ", 8}, {"DT", 10},
};

const std::unordered_map<std::string, uint8_t> Arch8086::ReserveDataMnemonics = 
{
    {"RESB", 1}, {"RESW", 2}, {"RESD", 4}, {"RESQ", 8}, {"REST", 10},
};

const std::unordered_map<std::vector<RegisterIdentifier>, RM> Arch8086::RmRegsCombinations =
{
    {{RegisterIdentifier::BX, RegisterIdentifier::SI}, RM::BX_SI },
    {{RegisterIdentifier::BX, RegisterIdentifier::DI}, RM::BX_DI },
    {{RegisterIdentifier::BP, RegisterIdentifier::SI}, RM::BP_SI },
    {{RegisterIdentifier::BP, RegisterIdentifier::DI}, RM::BP_DI },
    {{RegisterIdentifier::SI}, RM::SI },
    {{RegisterIdentifier::DI}, RM::DI },
    {{RegisterIdentifier::BP}, RM::BP },
    {{RegisterIdentifier::BX}, RM::BX },
    {{}, RM::DISP_16}
};

const std::unordered_map<RegisterIdentifier, InstructionPrefix> Arch8086::SregToSegOverride =
{
    { RegisterIdentifier::CS, InstructionPrefix::CS },
    { RegisterIdentifier::ES, InstructionPrefix::ES },
    { RegisterIdentifier::DS, InstructionPrefix::DS },
    { RegisterIdentifier::SS, InstructionPrefix::SS },
    { RegisterIdentifier::GS, InstructionPrefix::GS },
    { RegisterIdentifier::FS, InstructionPrefix::FS },
};