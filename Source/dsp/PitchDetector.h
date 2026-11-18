#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

class PitchDetector {
public:
    void prepare(double sr) { sampleRate = sr; }
    float detect(const float* x, int n, double sr) {
        sampleRate = sr;
        const int half = n / 2;
        if (half < 64) return 0.f;
        float rms = 0.f;
        for (int i = 0; i < n; ++i) rms += x[i] * x[i];
        rms = std::sqrt(rms / n);
        if (rms < 0.005f) return 0.f;
        buf.assign(half, 0.f);
        float running = 0.f;
        for (int tau = 0; tau < half; ++tau) {
            float d = 0.f;
            for (int j = 0; j < half; ++j) {
                float diff = x[j] - x[j + tau];
                d += diff * diff;
            }
            running += d;
            buf[tau] = (tau == 0) ? 1.f : d * tau / running;
        }
        const float threshold = 0.15f;
        int tauEst = -1;
        for (int tau = 2; tau < half; ++tau) {
            if (buf[tau] < threshold) {
                while (tau + 1 < half && buf[tau + 1] < buf[tau]) ++tau;
                tauEst = tau; break;
            }
        }
        if (tauEst <= 0) return 0.f;
        float s0 = buf[tauEst - 1], s1 = buf[tauEst], s2 = buf[tauEst + 1];
        float denom = (2.f * s1 - s2 - s0);
        float shift = (denom != 0.f) ? (0.5f * (s2 - s0) / denom) : 0.f;
        float period = tauEst + shift;
        return (float)(sampleRate / period);
    }
private:
    double sampleRate = 44100.0;
    std::vector<float> buf;
};
