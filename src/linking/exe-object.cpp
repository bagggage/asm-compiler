#include "exe-object.h"

using namespace ASM;
using namespace ASM::Codegen;

bool ExeObject::Deserialize(std::istream& stream)
{
    return true;
}

bool ExeObject::Serialize(std::ostream& stream) const
{
    const char zeroParagraph[pageByteSize] = { 0 };

    const size_t headerSize = sizeof(mzHeader) + (sizeof(RelocationTarget) * relocationTable.size());
    const size_t headerAlign = (headerSize % pageByteSize != 0) ? (pageByteSize - (headerSize % pageByteSize)) : 0;
    const size_t headerAlignedSize = headerSize + headerAlign;

    const size_t fileSize = headerAlignedSize + code->size();

    if (fileSize % pageByteSize != 0) {
        mzHeader.fileSizeInPages = (fileSize / pageByteSize) + 1;
        mzHeader.bytesInTheLastPage = fileSize % pageByteSize;
    }
    else {
        mzHeader.fileSizeInPages = fileSize / pageByteSize;
        mzHeader.bytesInTheLastPage = pageByteSize;
    }

    mzHeader.headerSizeInParagraphs = headerAlignedSize / paragraphByteSize;
    mzHeader.numberOfRelocationTargets = relocationTable.size();
    mzHeader.minAllocatedParagprahs = (fileSize / paragraphByteSize) - mzHeader.headerSizeInParagraphs;

    stream.write(reinterpret_cast<const char*>(&mzHeader), sizeof(mzHeader));

    if (relocationTable.empty() == false)
        stream.write(reinterpret_cast<const char*>(relocationTable.data()), relocationTable.size() * sizeof(RelocationTarget));
    if (headerAlign > 0)
        stream.write(zeroParagraph, headerAlign);
    if (code->size() > 0)
        stream.write(reinterpret_cast<const char*>(code->data()), code->size());

    if (stream.bad()) [[unlikely]]
        return false;

    return true;
}