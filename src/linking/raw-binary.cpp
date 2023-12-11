#include "raw-binary.h"

using namespace ASM;

bool RawBinary::Deserialize(std::istream& stream)
{
    stream.seekg(0, stream.end);

    size_t length = stream.tellg();

    stream.seekg(0, stream.beg);

    if (length > 0) 
    {
        code->resize(length, static_cast<uint8_t>(0));
        stream.read(reinterpret_cast<char*>(code->data()), length);

        return stream.bad() == false;
    }

    return true;
}

bool RawBinary::Serialize(std::ostream& stream) const
{
    if (code->empty())
        return true;

    stream.write(reinterpret_cast<const char*>(code->data()), code->size());

    return stream.bad() == false;
}