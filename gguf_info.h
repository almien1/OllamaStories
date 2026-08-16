#ifndef GGUF_INFO_H
#define GGUF_INFO_H
#include <QString>

struct GgufInfo
{
    bool valid = false;
    int blockCount = 0; // number of transformer layers - the max useful -ngl (GPU layers) value
    bool hasChatTemplate = false; // whether the file has a "tokenizer.chat_template" key

    // Everything below is 0 if the file didn't have the key (older/unusual
    // conversions sometimes omit some of these) - callers need to handle
    // that rather than assume they're populated.
    int trainedContextLength = 0; // "<arch>.context_length" - the model's own max, not a hard cap on what -c can be set to but a reasonable one
    int embeddingLength = 0; // "<arch>.embedding_length" - hidden size, used to derive a head dimension if key/value_length are absent
    int headCount = 0; // "<arch>.attention.head_count"
    int headCountKv = 0; // "<arch>.attention.head_count_kv" - fewer than headCount under GQA; 0 means "not present, assume == headCount"
    int keyLength = 0; // "<arch>.attention.key_length" - per-head key dimension
    int valueLength = 0; // "<arch>.attention.value_length" - per-head value dimension
};

// Reads just the metadata header of a GGUF file - the handful of keys in
// GgufInfo - without touching the (large) tensor data that follows it in
// the file.
GgufInfo readGgufInfo(const QString &path);

#endif // GGUF_INFO_H
