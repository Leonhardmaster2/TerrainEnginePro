#pragma once

#include "Core/Types.h"
#include <vector>

namespace Terrain {

// Texture format
enum class TextureFormat {
    R8,          // Single channel 8-bit
    RG8,         // Two channel 8-bit
    RGB8,        // Three channel 8-bit
    RGBA8,       // Four channel 8-bit
    R16,         // Single channel 16-bit
    RGB16,       // Three channel 16-bit
    RGBA16,      // Four channel 16-bit
    R32F,        // Single channel 32-bit float
    RGB32F,      // Three channel 32-bit float
    RGBA32F      // Four channel 32-bit float
};

// Texture class for storing generated textures
class Texture {
public:
    Texture(uint32 width, uint32 height, TextureFormat format);
    ~Texture();

    // Getters
    uint32 GetWidth() const { return m_Width; }
    uint32 GetHeight() const { return m_Height; }
    TextureFormat GetFormat() const { return m_Format; }
    uint32 GetChannelCount() const;
    uint32 GetBytesPerPixel() const;
    size_t GetDataSize() const;

    // Data access
    uint8* GetData() { return m_Data.data(); }
    const uint8* GetData() const { return m_Data.data(); }

    // Pixel access (normalized 0-1)
    void SetPixel(uint32 x, uint32 y, float32 r, float32 g = 0.0f, float32 b = 0.0f, float32 a = 1.0f);
    void GetPixel(uint32 x, uint32 y, float32& r, float32& g, float32& b, float32& a) const;

    // Pixel access (raw bytes)
    void SetPixelRaw(uint32 x, uint32 y, const uint8* data);
    void GetPixelRaw(uint32 x, uint32 y, uint8* data) const;

    // Export
    bool ExportPNG(const String& filepath) const;
    bool ExportTGA(const String& filepath) const;

private:
    uint32 m_Width;
    uint32 m_Height;
    TextureFormat m_Format;
    std::vector<uint8> m_Data;
};

// Utility functions
inline uint32 GetFormatChannels(TextureFormat format) {
    switch (format) {
        case TextureFormat::R8:
        case TextureFormat::R16:
        case TextureFormat::R32F:
            return 1;
        case TextureFormat::RG8:
            return 2;
        case TextureFormat::RGB8:
        case TextureFormat::RGB16:
        case TextureFormat::RGB32F:
            return 3;
        case TextureFormat::RGBA8:
        case TextureFormat::RGBA16:
        case TextureFormat::RGBA32F:
            return 4;
        default:
            return 0;
    }
}

inline uint32 GetFormatBytesPerChannel(TextureFormat format) {
    switch (format) {
        case TextureFormat::R8:
        case TextureFormat::RG8:
        case TextureFormat::RGB8:
        case TextureFormat::RGBA8:
            return 1;
        case TextureFormat::R16:
        case TextureFormat::RGB16:
        case TextureFormat::RGBA16:
            return 2;
        case TextureFormat::R32F:
        case TextureFormat::RGB32F:
        case TextureFormat::RGBA32F:
            return 4;
        default:
            return 0;
    }
}

} // namespace Terrain
