#ifndef __SOURCE_LOCATION_H
#define __SOURCE_LOCATION_H

namespace ASM
{
    struct SourceLocation
    {
        SourceLocation() = default;
        SourceLocation(const char* sourcePtr) : sourcePointer(sourcePtr) {}

        const char* sourcePointer = nullptr;
        unsigned int line = 0;

        inline const char* operator--() { return (--sourcePointer); }
        inline const char* operator--(int) { return (sourcePointer--); }
        inline const char* operator++() { return (++sourcePointer); }
        inline const char* operator++(int) { return (sourcePointer++); }

        inline const char* operator=(const char* other) { return (sourcePointer = other); }

        inline operator const char*() const { return sourcePointer; }
    };
}

#endif