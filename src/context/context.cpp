#include "context.h"

#include <istream>
#include <cassert>

using namespace ASM;

AssemblyContext::AssemblyContext(std::istream& sourceStream, const InstructionSet_t& instructionSet) : instructionSet(&instructionSet)
{
    currentSource.resize(sourceStream.rdbuf()->in_avail());

    sourceStream.read(currentSource.begin().base(), currentSource.size());
}

Message* AssemblyContext::GetLastError()
{
    Message* error = lastError;

    lastError = nullptr;

    return error;
}

std::unique_ptr<Message> AssemblyContext::GetMessage()
{
    if (messageQueue.empty())
        return nullptr;

    std::unique_ptr<Message> message(messageQueue.front().release());

    messageQueue.pop();

    return std::move(message);
}

Message* AssemblyContext::MakeMessage(Message::Kind kind, const char* message, SourceLocation location, size_t length)
{
    Message* result;

    if (location.sourcePointer == nullptr)
        result = new Message(kind, message);
    else
        result = new PointedMessage(kind, location, length, message);

    return result;
}

void AssemblyContext::Info(const char* message, SourceLocation location, size_t length)
{
    messageQueue.push(nullptr);
    messageQueue.back().reset(MakeMessage(Message::Kind::Info, message, location, length));
}

void AssemblyContext::Warn(const char* message, SourceLocation location, size_t length)
{
    messageQueue.push(nullptr);
    messageQueue.back().reset(MakeMessage(Message::Kind::Warning, message, location, length));
}

void AssemblyContext::Error(const char* message, SourceLocation location, size_t length)
{
    messageQueue.push(nullptr);
    messageQueue.back().reset(MakeMessage(Message::Kind::Error, message, location, length));

    lastError = messageQueue.back().get();
}
