#include "Texture.h"
#include "Core/Logger.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <algorithm>
#include <cstring>

namespace Terrain {

Texture::Texture(uint32 width, uint32 height, TextureFormat format)
    : m_Width(width), m_Height(height), m_Format(format) {

    size_t dataSize = width * height * GetBytesPerPixel();
    m_Data.resize(dataSize, 0);
}

Texture::~Texture() {
}

uint32 Texture::GetChannelCount() const {
    return GetFormatChannels(m_Format);
}

uint32 Texture::GetBytesPerPixel() const {
    return GetFormatChannels(m_Format) * GetFormatBytesPerChannel(m_Format);
}

size_t Texture::GetDataSize() const {
    return m_Data.size();
}

void Texture::SetPixel(uint32 x, uint32 y, float32 r, float32 g, float32 b, float32 a) {
    if (x >= m_Width || y >= m_Height) return;

    uint32 channels = GetChannelCount();
    uint32 bytesPerChannel = GetFormatBytesPerChannel(m_Format);
    uint32 pixelOffset = (y * m_Width + x) * GetBytesPerPixel();

    float32 values[4] = { r, g, b, a };

    for (uint32 c = 0; c < channels; c++) {
        float32 value = std::clamp(values[c], 0.0f, 1.0f);

        if (bytesPerChannel == 1) {
            m_Data[pixelOffset + c] = static_cast<uint8>(value * 255.0f);
        } else if (bytesPerChannel == 2) {
            uint16 val16 = static_cast<uint16>(value * 65535.0f);
            std::memcpy(&m_Data[pixelOffset + c * 2], &val16, 2);
        } else if (bytesPerChannel == 4) {
            std::memcpy(&m_Data[pixelOffset + c * 4], &value, 4);
        }
    }
}

void Texture::GetPixel(uint32 x, uint32 y, float32& r, float32& g, float32& b, float32& a) const {
    r = g = b = 0.0f;
    a = 1.0f;

    if (x >= m_Width || y >= m_Height) return;

    uint32 channels = GetChannelCount();
    uint32 bytesPerChannel = GetFormatBytesPerChannel(m_Format);
    uint32 pixelOffset = (y * m_Width + x) * GetBytesPerPixel();

    float32* values[4] = { &r, &g, &b, &a };

    for (uint32 c = 0; c < channels && c < 4; c++) {
        if (bytesPerChannel == 1) {
            *values[c] = static_cast<float32>(m_Data[pixelOffset + c]) / 255.0f;
        } else if (bytesPerChannel == 2) {
            uint16 val16;
            std::memcpy(&val16, &m_Data[pixelOffset + c * 2], 2);
            *values[c] = static_cast<float32>(val16) / 65535.0f;
        } else if (bytesPerChannel == 4) {
            std::memcpy(values[c], &m_Data[pixelOffset + c * 4], 4);
        }
    }
}

void Texture::SetPixelRaw(uint32 x, uint32 y, const uint8* data) {
    if (x >= m_Width || y >= m_Height) return;

    uint32 pixelOffset = (y * m_Width + x) * GetBytesPerPixel();
    std::memcpy(&m_Data[pixelOffset], data, GetBytesPerPixel());
}

void Texture::GetPixelRaw(uint32 x, uint32 y, uint8* data) const {
    if (x >= m_Width || y >= m_Height) return;

    uint32 pixelOffset = (y * m_Width + x) * GetBytesPerPixel();
    std::memcpy(data, &m_Data[pixelOffset], GetBytesPerPixel());
}

bool Texture::ExportPNG(const String& filepath) const {
    // Convert to 8-bit if needed
    std::vector<uint8> exportData;
    uint32 channels = GetChannelCount();
    uint32 bytesPerChannel = GetFormatBytesPerChannel(m_Format);

    if (bytesPerChannel == 1) {
        // Already 8-bit, use directly
        exportData = m_Data;
    } else {
        // Convert to 8-bit
        exportData.resize(m_Width * m_Height * channels);

        for (uint32 y = 0; y < m_Height; y++) {
            for (uint32 x = 0; x < m_Width; x++) {
                float32 r, g, b, a;
                GetPixel(x, y, r, g, b, a);

                uint32 outOffset = (y * m_Width + x) * channels;
                exportData[outOffset + 0] = static_cast<uint8>(r * 255.0f);
                if (channels > 1) exportData[outOffset + 1] = static_cast<uint8>(g * 255.0f);
                if (channels > 2) exportData[outOffset + 2] = static_cast<uint8>(b * 255.0f);
                if (channels > 3) exportData[outOffset + 3] = static_cast<uint8>(a * 255.0f);
            }
        }
    }

    int result = stbi_write_png(filepath.c_str(), m_Width, m_Height, channels, exportData.data(), m_Width * channels);

    if (result == 0) {
        LOG_ERROR("Failed to write PNG: %s", filepath.c_str());
        return false;
    }

    LOG_INFO("Exported texture to PNG: %s", filepath.c_str());
    return true;
}

bool Texture::ExportTGA(const String& filepath) const {
    // Convert to 8-bit if needed
    std::vector<uint8> exportData;
    uint32 channels = GetChannelCount();
    uint32 bytesPerChannel = GetFormatBytesPerChannel(m_Format);

    if (bytesPerChannel == 1) {
        exportData = m_Data;
    } else {
        exportData.resize(m_Width * m_Height * channels);

        for (uint32 y = 0; y < m_Height; y++) {
            for (uint32 x = 0; x < m_Width; x++) {
                float32 r, g, b, a;
                GetPixel(x, y, r, g, b, a);

                uint32 outOffset = (y * m_Width + x) * channels;
                exportData[outOffset + 0] = static_cast<uint8>(r * 255.0f);
                if (channels > 1) exportData[outOffset + 1] = static_cast<uint8>(g * 255.0f);
                if (channels > 2) exportData[outOffset + 2] = static_cast<uint8>(b * 255.0f);
                if (channels > 3) exportData[outOffset + 3] = static_cast<uint8>(a * 255.0f);
            }
        }
    }

    int result = stbi_write_tga(filepath.c_str(), m_Width, m_Height, channels, exportData.data());

    if (result == 0) {
        LOG_ERROR("Failed to write TGA: %s", filepath.c_str());
        return false;
    }

    LOG_INFO("Exported texture to TGA: %s", filepath.c_str());
    return true;
}

} // namespace Terrain
