#include "melegi/AudioEngine.hpp"
#include <fstream>
#include <cstring>
#include <cmath>
#include <stdexcept>

namespace melegi {

// ── Little-endian I/O helpers ──────────────────────────────────────────────
void AudioEngine::writeLE16(uint8_t* dst, uint16_t v) {
    dst[0] = v & 0xFF;
    dst[1] = (v >> 8) & 0xFF;
}
void AudioEngine::writeLE32(uint8_t* dst, uint32_t v) {
    dst[0] = v & 0xFF; dst[1] = (v >> 8) & 0xFF;
    dst[2] = (v >> 16) & 0xFF; dst[3] = (v >> 24) & 0xFF;
}
uint16_t AudioEngine::readLE16(const uint8_t* src) {
    return static_cast<uint16_t>(src[0]) | (static_cast<uint16_t>(src[1]) << 8);
}
uint32_t AudioEngine::readLE32(const uint8_t* src) {
    return static_cast<uint32_t>(src[0])
         | (static_cast<uint32_t>(src[1]) << 8)
         | (static_cast<uint32_t>(src[2]) << 16)
         | (static_cast<uint32_t>(src[3]) << 24);
}

// ── WAV decode ─────────────────────────────────────────────────────────────
std::optional<AudioBuffer> AudioEngine::decodeWav(const std::string& path,
                                                   std::string& err) const {
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) { err = "Cannot open: " + path; return {}; }

    // Read entire file
    f.seekg(0, std::ios::end);
    size_t fileSize = static_cast<size_t>(f.tellg());
    f.seekg(0, std::ios::beg);
    std::vector<uint8_t> data(fileSize);
    f.read(reinterpret_cast<char*>(data.data()), fileSize);
    if (!f) { err = "Read error: " + path; return {}; }

    // RIFF header
    if (fileSize < 12 || std::memcmp(data.data(), "RIFF", 4) != 0
                      || std::memcmp(data.data() + 8, "WAVE", 4) != 0) {
        err = "Not a WAVE file: " + path; return {};
    }

    uint16_t audioFmt{0}, numCh{0}, bitsPerSample{0};
    uint32_t sampleRate{0};
    size_t   dataOffset{0};
    uint32_t dataSize{0};

    // Chunk walk
    size_t pos = 12;
    while (pos + 8 <= fileSize) {
        char    chunkId[5] = {};
        std::memcpy(chunkId, data.data() + pos, 4);
        uint32_t chunkSize = readLE32(data.data() + pos + 4);
        pos += 8;

        if (std::memcmp(chunkId, "fmt ", 4) == 0 && chunkSize >= 16) {
            audioFmt      = readLE16(data.data() + pos);
            numCh         = readLE16(data.data() + pos + 2);
            sampleRate    = readLE32(data.data() + pos + 4);
            // byteRate = readLE32(pos+8), blockAlign = readLE16(pos+12)
            bitsPerSample = readLE16(data.data() + pos + 14);
        } else if (std::memcmp(chunkId, "data", 4) == 0) {
            dataOffset = pos;
            dataSize   = chunkSize;
            break; // data chunk found, stop walking
        }
        // Advance to next chunk (pad to even byte boundary)
        pos += chunkSize + (chunkSize & 1);
    }

    if (dataOffset == 0 || numCh == 0 || sampleRate == 0) {
        err = "Incomplete WAV header: " + path; return {};
    }
    if (audioFmt != 1 && audioFmt != 3) {
        err = "Unsupported format (need PCM=1 or IEEE float=3): " + path; return {};
    }
    if (bitsPerSample != 16 && bitsPerSample != 24 && bitsPerSample != 32) {
        err = "Unsupported bit depth " + std::to_string(bitsPerSample); return {};
    }

    uint32_t bytesPerFrame = (bitsPerSample / 8) * numCh;
    if (bytesPerFrame == 0) { err = "Zero bytes per frame"; return {}; }
    uint64_t numFrames = dataSize / bytesPerFrame;

    AudioBuffer buf(numCh, numFrames, sampleRate);

    const uint8_t* src = data.data() + dataOffset;
    for (uint64_t f = 0; f < numFrames; ++f) {
        for (uint16_t c = 0; c < numCh; ++c) {
            Sample s = 0.f;
            if (audioFmt == 1) {
                if (bitsPerSample == 16) {
                    int16_t raw;
                    std::memcpy(&raw, src, 2);
                    s = raw / 32768.f;
                    src += 2;
                } else if (bitsPerSample == 24) {
                    int32_t raw = static_cast<int32_t>(src[0])
                                | (static_cast<int32_t>(src[1]) << 8)
                                | (static_cast<int32_t>(src[2]) << 16);
                    if (raw & 0x800000) raw |= 0xFF000000; // sign extend
                    s = raw / 8388608.f;
                    src += 3;
                } else { // 32-bit PCM (rare)
                    int32_t raw;
                    std::memcpy(&raw, src, 4);
                    s = raw / 2147483648.f;
                    src += 4;
                }
            } else { // IEEE float 32
                std::memcpy(&s, src, 4);
                src += 4;
            }
            buf.at(c, f) = clamp(s, -1.f, 1.f);
        }
    }
    return buf;
}

std::optional<AudioBuffer> AudioEngine::load(const std::string& path,
                                              std::string& err) const {
    return decodeWav(path, err);
}

std::optional<WavInfo> AudioEngine::inspect(const std::string& path,
                                             std::string& err) const {
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) { err = "Cannot open: " + path; return {}; }

    uint8_t hdr[44];
    f.read(reinterpret_cast<char*>(hdr), 44);
    if (!f || std::memcmp(hdr, "RIFF", 4) != 0 || std::memcmp(hdr + 8, "WAVE", 4) != 0) {
        err = "Not a WAVE file"; return {};
    }

    WavInfo info;
    info.audioFormat   = readLE16(hdr + 20);
    info.numChannels   = readLE16(hdr + 22);
    info.sampleRate    = readLE32(hdr + 24);
    info.bitsPerSample = readLE16(hdr + 34);

    // Walk to data chunk for frame count
    f.seekg(12);
    uint8_t chunk[8];
    while (f.read(reinterpret_cast<char*>(chunk), 8)) {
        uint32_t chunkSz = readLE32(chunk + 4);
        if (std::memcmp(chunk, "data", 4) == 0) {
            uint32_t bpf = (info.bitsPerSample / 8) * info.numChannels;
            info.numFrames   = bpf > 0 ? chunkSz / bpf : 0;
            info.durationSec = info.sampleRate > 0
                ? static_cast<float>(info.numFrames) / info.sampleRate : 0.f;
            return info;
        }
        f.seekg(chunkSz + (chunkSz & 1), std::ios::cur);
    }
    err = "data chunk not found"; return {};
}

// ── WAV write ──────────────────────────────────────────────────────────────
static void writeChunkHeader(std::ofstream& f, const char* id, uint32_t size) {
    f.write(id, 4);
    uint8_t tmp[4];
    tmp[0] = size & 0xFF; tmp[1] = (size >> 8) & 0xFF;
    tmp[2] = (size >> 16) & 0xFF; tmp[3] = (size >> 24) & 0xFF;
    f.write(reinterpret_cast<char*>(tmp), 4);
}

bool AudioEngine::writePCM(const AudioBuffer& buf, const std::string& path,
                            uint16_t bitDepth, std::string& err) const {
    if (!buf.valid()) { err = "Invalid buffer"; return false; }
    if (bitDepth != 16 && bitDepth != 24 && bitDepth != 32) {
        err = "Unsupported bitDepth"; return false;
    }
    std::ofstream f(path, std::ios::binary);
    if (!f.is_open()) { err = "Cannot create: " + path; return false; }

    uint16_t nCh   = static_cast<uint16_t>(buf.numChannels);
    uint32_t sr    = buf.sampleRate;
    uint16_t bps   = bitDepth;
    uint16_t blk   = nCh * (bps / 8);
    uint32_t br    = sr * blk;
    uint64_t nFr   = buf.numFrames;
    uint32_t dataSz = static_cast<uint32_t>(nFr * blk);

    // RIFF header
    f.write("RIFF", 4);
    uint32_t riffSz = 36 + dataSz;
    uint8_t tmp[4];
    AudioEngine::writeLE32(tmp, riffSz); f.write(reinterpret_cast<char*>(tmp), 4);
    f.write("WAVE", 4);

    // fmt chunk
    writeChunkHeader(f, "fmt ", 16);
    AudioEngine::writeLE16(tmp, 1); f.write(reinterpret_cast<char*>(tmp), 2); // PCM
    AudioEngine::writeLE16(tmp, nCh); f.write(reinterpret_cast<char*>(tmp), 2);
    AudioEngine::writeLE32(tmp, sr); f.write(reinterpret_cast<char*>(tmp), 4);
    AudioEngine::writeLE32(tmp, br); f.write(reinterpret_cast<char*>(tmp), 4);
    AudioEngine::writeLE16(tmp, blk); f.write(reinterpret_cast<char*>(tmp), 2);
    AudioEngine::writeLE16(tmp, bps); f.write(reinterpret_cast<char*>(tmp), 2);

    // data chunk
    writeChunkHeader(f, "data", dataSz);

    float maxVal = static_cast<float>((1 << (bps - 1)) - 1);
    for (uint64_t fr = 0; fr < nFr; ++fr) {
        for (uint16_t c = 0; c < nCh; ++c) {
            float s = clamp(buf.at(c, fr), -1.f, 1.f);
            int32_t q = static_cast<int32_t>(s * maxVal);
            if (bps == 16) {
                int16_t v = static_cast<int16_t>(q);
                f.write(reinterpret_cast<char*>(&v), 2);
            } else if (bps == 24) {
                tmp[0] = q & 0xFF; tmp[1] = (q >> 8) & 0xFF; tmp[2] = (q >> 16) & 0xFF;
                f.write(reinterpret_cast<char*>(tmp), 3);
            } else {
                f.write(reinterpret_cast<char*>(&q), 4);
            }
        }
    }
    return f.good();
}

bool AudioEngine::writeFloat(const AudioBuffer& buf, const std::string& path,
                              std::string& err) const {
    if (!buf.valid()) { err = "Invalid buffer"; return false; }
    std::ofstream f(path, std::ios::binary);
    if (!f.is_open()) { err = "Cannot create: " + path; return false; }

    uint16_t nCh   = static_cast<uint16_t>(buf.numChannels);
    uint32_t sr    = buf.sampleRate;
    uint16_t bps   = 32;
    uint16_t blk   = nCh * 4;
    uint32_t br    = sr * blk;
    uint32_t dataSz = static_cast<uint32_t>(buf.numFrames * blk);

    f.write("RIFF", 4);
    uint32_t riffSz = 36 + dataSz;
    uint8_t tmp[4]; AudioEngine::writeLE32(tmp, riffSz); f.write(reinterpret_cast<char*>(tmp), 4);
    f.write("WAVE", 4);
    writeChunkHeader(f, "fmt ", 16);
    AudioEngine::writeLE16(tmp, 3); f.write(reinterpret_cast<char*>(tmp), 2); // IEEE float
    AudioEngine::writeLE16(tmp, nCh); f.write(reinterpret_cast<char*>(tmp), 2);
    AudioEngine::writeLE32(tmp, sr); f.write(reinterpret_cast<char*>(tmp), 4);
    AudioEngine::writeLE32(tmp, br); f.write(reinterpret_cast<char*>(tmp), 4);
    AudioEngine::writeLE16(tmp, blk); f.write(reinterpret_cast<char*>(tmp), 2);
    AudioEngine::writeLE16(tmp, bps); f.write(reinterpret_cast<char*>(tmp), 2);
    writeChunkHeader(f, "data", dataSz);

    for (uint64_t fr = 0; fr < buf.numFrames; ++fr)
        for (uint16_t c = 0; c < nCh; ++c) {
            float s = buf.at(c, fr);
            f.write(reinterpret_cast<char*>(&s), 4);
        }
    return f.good();
}

AudioBuffer AudioEngine::generateTestTone(float freqHz, float durSec,
                                           float amp, SampleRate sr) const {
    uint64_t nFr = static_cast<uint64_t>(durSec * sr);
    AudioBuffer buf(2, nFr, sr);
    float phase = 0.f, inc = freqHz / static_cast<float>(sr);
    for (uint64_t f = 0; f < nFr; ++f) {
        // Fade in/out to avoid clicks
        float env = 1.f;
        float t   = static_cast<float>(f) / static_cast<float>(nFr);
        if (t < 0.01f) env = t / 0.01f;
        else if (t > 0.99f) env = (1.f - t) / 0.01f;
        float s = std::sin(phase * TWO_PI) * amp * env;
        buf.at(0, f) = s;
        buf.at(1, f) = s;
        phase += inc;
        if (phase >= 1.f) phase -= 1.f;
    }
    return buf;
}

AudioBuffer AudioEngine::generateSilence(float durSec, uint32_t channels,
                                          SampleRate sr) const {
    return AudioBuffer(channels, static_cast<uint64_t>(durSec * sr), sr);
}

} // namespace melegi
