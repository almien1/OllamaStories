#include "gguf_info.h"
#include <QFile>
#include <cstring>

// Minimal reader for the GGUF metadata format (magic + version + counts,
// then key/value pairs), just enough to find "<arch>.block_count".
// Spec: https://github.com/ggml-org/ggml/blob/master/docs/gguf.md
namespace {

enum GgufValueType : quint32
{
    GGUF_TYPE_UINT8 = 0,
    GGUF_TYPE_INT8 = 1,
    GGUF_TYPE_UINT16 = 2,
    GGUF_TYPE_INT16 = 3,
    GGUF_TYPE_UINT32 = 4,
    GGUF_TYPE_INT32 = 5,
    GGUF_TYPE_FLOAT32 = 6,
    GGUF_TYPE_BOOL = 7,
    GGUF_TYPE_STRING = 8,
    GGUF_TYPE_ARRAY = 9,
    GGUF_TYPE_UINT64 = 10,
    GGUF_TYPE_INT64 = 11,
    GGUF_TYPE_FLOAT64 = 12,
};

// Byte size of fixed-width scalar types; -1 for the variable-length ones
// (STRING, ARRAY) which need their own handling.
int fixedTypeSize(quint32 type)
{
    switch (type)
    {
    case GGUF_TYPE_UINT8: case GGUF_TYPE_INT8: case GGUF_TYPE_BOOL: return 1;
    case GGUF_TYPE_UINT16: case GGUF_TYPE_INT16: return 2;
    case GGUF_TYPE_UINT32: case GGUF_TYPE_INT32: case GGUF_TYPE_FLOAT32: return 4;
    case GGUF_TYPE_UINT64: case GGUF_TYPE_INT64: case GGUF_TYPE_FLOAT64: return 8;
    default: return -1;
    }
}

class GgufReader
{
public:
    explicit GgufReader(QFile &file) : m_file(file) {}

    bool ok() const { return m_ok; }

    quint32 readU32() { return readPod<quint32>(); }
    quint64 readU64() { return readPod<quint64>(); }

    QString readString()
    {
        quint64 len = readU64();
        // GGUF key/value strings are metadata, never multi-gigabyte; treat an
        // implausible length as a sign we've misparsed something.
        if (!m_ok || len > 1024 * 1024)
        {
            m_ok = false;
            return {};
        }
        QByteArray bytes = m_file.read(qint64(len));
        if (quint64(bytes.size()) != len)
        {
            m_ok = false;
            return {};
        }
        return QString::fromUtf8(bytes);
    }

    // Reads and discards one value of the given type, recursing into arrays.
    void skipValue(quint32 type)
    {
        if (!m_ok)
        {
            return;
        }
        if (type == GGUF_TYPE_STRING)
        {
            readString();
            return;
        }
        if (type == GGUF_TYPE_ARRAY)
        {
            quint32 elementType = readU32();
            quint64 count = readU64();
            for (quint64 i = 0; i < count && m_ok; ++i)
            {
                skipValue(elementType);
            }
            return;
        }
        int size = fixedTypeSize(type);
        if (size < 0 || !m_file.seek(m_file.pos() + size))
        {
            m_ok = false;
        }
    }

    // Reads a scalar value of the given (non-string, non-array) type as an
    // unsigned integer.
    quint64 readAsUInt(quint32 type)
    {
        switch (type)
        {
        case GGUF_TYPE_UINT8: return readPod<quint8>();
        case GGUF_TYPE_INT8: return quint64(readPod<qint8>());
        case GGUF_TYPE_UINT16: return readPod<quint16>();
        case GGUF_TYPE_INT16: return quint64(readPod<qint16>());
        case GGUF_TYPE_UINT32: return readPod<quint32>();
        case GGUF_TYPE_INT32: return quint64(readPod<qint32>());
        case GGUF_TYPE_UINT64: return readPod<quint64>();
        case GGUF_TYPE_INT64: return quint64(readPod<qint64>());
        default:
            m_ok = false;
            return 0;
        }
    }

private:
    template<typename T>
    T readPod()
    {
        T value{};
        if (!m_ok)
        {
            return value;
        }
        if (m_file.read(reinterpret_cast<char *>(&value), sizeof(T)) != qint64(sizeof(T)))
        {
            m_ok = false;
        }
        return value;
    }

    QFile &m_file;
    bool m_ok = true;
};

}

GgufInfo readGgufInfo(const QString &path)
{
    GgufInfo info;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        return info;
    }

    char magic[4];
    if (file.read(magic, 4) != 4 || memcmp(magic, "GGUF", 4) != 0)
    {
        return info;
    }

    GgufReader reader(file);
    reader.readU32(); // version - both v2 and v3 use the 64-bit counts/lengths read below
    quint64 tensorCount = reader.readU64();
    quint64 kvCount = reader.readU64();
    Q_UNUSED(tensorCount);

    // A GGUF file's metadata is a few hundred key/value pairs at most - cheap
    // enough to just scan all of them for the handful of suffixes we want,
    // rather than bailing out early once a subset is found.
    auto readSmallUInt = [&reader](quint32 type) -> int
    {
        quint64 value = reader.readAsUInt(type);
        return (reader.ok() && value < 1000000) ? int(value) : 0;
    };

    for (quint64 i = 0; i < kvCount && reader.ok(); ++i)
    {
        QString key = reader.readString();
        if (!reader.ok())
        {
            break;
        }
        quint32 type = reader.readU32();
        if (!reader.ok())
        {
            break;
        }

        if (key.endsWith(".block_count"))
        {
            int value = readSmallUInt(type);
            if (value > 0)
            {
                info.blockCount = value;
                info.valid = true;
            }
        }
        else if (key.endsWith(".context_length"))
        {
            info.trainedContextLength = readSmallUInt(type);
        }
        else if (key.endsWith(".embedding_length"))
        {
            info.embeddingLength = readSmallUInt(type);
        }
        else if (key.endsWith(".attention.head_count"))
        {
            info.headCount = readSmallUInt(type);
        }
        else if (key.endsWith(".attention.head_count_kv"))
        {
            info.headCountKv = readSmallUInt(type);
        }
        else if (key.endsWith(".attention.key_length"))
        {
            info.keyLength = readSmallUInt(type);
        }
        else if (key.endsWith(".attention.value_length"))
        {
            info.valueLength = readSmallUInt(type);
        }
        else if (key == "tokenizer.chat_template")
        {
            info.hasChatTemplate = true;
            reader.skipValue(type);
        }
        else
        {
            reader.skipValue(type);
        }
    }

    return info;
}
