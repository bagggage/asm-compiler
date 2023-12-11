#include "message.h"

using namespace ASM;

std::string Message::GetKindString() const
{
    switch (kind)
    {
    case Kind::Info:
        return "\033[1m[Info]\033[0m";
        break;
    case Kind::Warning:
        return "\033[1;33m[Warning]\033[0m";
        break;
    case Kind::Error:
        return "\033[1;31m[Error]\033[0m";
        break;
    default:
        return "[Unknown message]";
        break;
    }
}

std::string Message::What() const
{
    return (GetKindString() + ": " + content);
}

std::string PointedMessage::What() const
{
    std::string result = GetKindString() + " \033[1mline:" + std::to_string(location.line + 1) + ":\033[0m " +
        content + " \"\033[1;35m";

    const char* endPos;

    if (length == 0)
    {
        endPos = location.sourcePointer;

        while(*endPos != '\n' && *endPos != '\0')
            ++endPos;
    }
    else
    {
        endPos = location.sourcePointer + length;
    }
    
    size_t padding = 0;
    
    if (length == 0 && location.sourcePointer + 1 < endPos)
    {
        result.push_back(*location.sourcePointer);

        result += "\033[1;0m";
        result.append(location.sourcePointer + 1, endPos);
    }
    else
    {
        result.append(location.sourcePointer, endPos);
    }
    
    result += "\033[0m\"";//+ std::string(padding, ' ') + std::string((endPos - location.sourcePointer), '^');

    return result;
}