#ifndef __MESSAGE_H
#define __MESSAGE_H

#include <string>

#include "source-location.h"

namespace ASM
{
    class Message
    {
    public:
        enum class Kind
        {
            Info,
            Warning,
            Error
        };
    protected:
        std::string content;

        Kind kind;

        std::string GetKindString() const;
    public:
        Message(Kind kind, const std::string& content) : kind(kind), content(content) {}
        Message(Kind kind, std::string&& content) : kind(kind), content(content) {}
        Message(Kind kind, const char* content) : kind(kind), content(content) {}

        virtual ~Message() = default;

        inline bool Is(Kind intendentKind) const { return (intendentKind == kind); }

        virtual std::string What() const;
    };

    class PointedMessage final : public Message
    {
    private:
        SourceLocation location;
        size_t length;
    public:
        PointedMessage(Kind kind, SourceLocation location, size_t length, const std::string& content) : 
            Message(kind, content), location(location), length(length) {}

        PointedMessage(Kind kind, SourceLocation location, size_t length, std::string&& content) :
            Message(kind, content), location(location), length(length) {}

        PointedMessage(Kind kind, SourceLocation location, size_t length, const char* content) :
            Message(kind, content), location(location), length(length) {}

        std::string What() const override;
    };
}

#endif