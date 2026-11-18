#pragma once
#include <vector>
#include <cmath>

class PitchShifter {
public:
    void prepare(double sr) {
        sampleRate = sr;
        size = (int)(sr * 0.1);
        size = (size + 1023) & ~1023;
        ring.assign(size, 0.f);
        writePos = 0; delayPos = 0.f;
    }
    void reset() {
        std::fill(ring.begin(), ring.end(), 0.f);
        writePos = 0; delayPos = 0.f;
    }
    float process(float in, float ratio) {
        ring[writePos] = in;
        float d1 = delayPos;
        float d2 = delayPos + size * 0.5f;
        if (d2 >= size) d2 -= size;
        float phase = (delayPos / size) * 2.f * 3.14159265f;
        float w1 = 0.5f * (1.f + std::cos(phase));
        float w2 = 1.f - w1;
        float s1 = readInterp((float)writePos - d1);
        float s2 = readInterp((float)writePos - d2);
        float out = s1 * w1 + s2 * w2;
        delayPos -= (ratio - 1.f);
        if (delayPos < 0.f) delayPos += size;
        if (delayPos >= size) delayPos -= size;
        writePos = (writePos + 1) % size;
        return out;
    }
private:
    float readInterp(float pos) const {
        while (pos < 0.f) pos += size;
        while (pos >= size) pos -= size;
        int i0 = (int)pos;
        int i1 = (i0 + 1) % size;
        float f = pos - i0;
        return ring[i0] * (1.f - f) + ring[i1] * f;
    }
    std::vector<float> ring;
    int size = 0, writePos = 0;
    float delayPos = 0.f;
    double sampleRate = 44100.0;
};
