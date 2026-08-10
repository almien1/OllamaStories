#ifndef GGUF_INFO_H
#define GGUF_INFO_H
#include <QString>

struct GgufInfo
{
    bool valid = false;
    int blockCount = 0; // number of transformer layers - the max useful -ngl (GPU layers) value
    bool hasChatTemplate = false; // whether the file has a "tokenizer.chat_template" key
};

// Reads just the metadata header of a GGUF file - its "<architecture>.block_count"
// and "tokenizer.chat_template" keys - without touching the (large) tensor
// data that follows it in the file.
GgufInfo readGgufInfo(const QString &path);

#endif // GGUF_INFO_H
