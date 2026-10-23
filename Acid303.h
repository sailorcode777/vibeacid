#pragma once
#include <algorithm>
#include <cmath>

// Monophonic acid bass voice:
//   PolyBLEP saw/square -> 4-pole tanh ladder low-pass (2x oversampled) -> VCA
// Slide = overlapping notes (no envelope retrigger, ~60 ms glide).
// Accent = MIDI velocity above ~100/127 (boosts filter env depth and level).
// No JUCE dependency, so it can be unit-tested on its own.
class Acid303
{
public:
    // ---- knobs (set these from your plugin parameters each block) ----
    float cutoffHz  = 300.0f;   // base filter cutoff
    float resonance = 0.7f;     // 0..1
    float envMod    = 0.6f;     // 0..1, filter envelope depth
    float decayMs   = 300.0f;   // filter envelope decay (~200 ms to 2 s on a 303)
    float accent    = 0.6f;     // 0..1, how strong accented notes are
    float tuneCents = 0.0f;
    bool  square    = false;    // false = saw, true = square

    void prepare (double sampleRate)
    {
        sr = (float) sampleRate;
        reset();
        updateCoefs();
    }

    void reset()
    {
        phase = 0.0f; freq = targetFreq = 110.0f;
        filterEnv = ampEnv = 0.0f;
        for (auto& s : y) s = 0.0f;
        gate = accentOn = false;
        currentNote = -1;
    }

    // call once per block after changing knobs
    void updateCoefs()
    {
        envDecayCoef = std::exp (-1.0f / (std::max (decayMs, 5.0f) * 0.001f * sr));
        glideCoef    = 1.0f - std::exp (-1.0f / (0.060f * sr));
        ampAttack    = 1.0f - std::exp (-1.0f / (0.002f * sr));
        ampRelease   = 1.0f - std::exp (-1.0f / (0.010f * sr));
    }

    void noteOn (int note, float velocity01)
    {
        const bool slide = gate;   // a note arriving while another is held = slide
        targetFreq  = 440.0f * std::pow (2.0f, ((float) note - 69.0f + tuneCents / 100.0f) / 12.0f);
        accentOn    = velocity01 > 0.78f;
        currentNote = note;
        if (! slide)
        {
            freq      = targetFreq;
            filterEnv = 1.0f;      // slid notes do NOT retrigger the envelope
        }
        gate = true;
    }

    void noteOff (int note)
    {
        if (note == currentNote)   // ignore the release of a note we already slid away from
            gate = false;
    }

    float process()
    {
        // pitch glide
        freq += (targetFreq - freq) * glideCoef;

        // oscillator
        const float dt = freq / sr;
        float osc;
        if (square)
        {
            float p2 = phase + 0.5f; if (p2 >= 1.0f) p2 -= 1.0f;
            osc = (phase < 0.5f ? 1.0f : -1.0f) + polyBlep (phase, dt) - polyBlep (p2, dt);
        }
        else
        {
            osc = 2.0f * phase - 1.0f - polyBlep (phase, dt);
        }
        phase += dt;
        if (phase >= 1.0f) phase -= 1.0f;

        // filter envelope -> cutoff (exponential, up to 4 octaves of sweep)
        filterEnv *= envDecayCoef;
        const float accentBoost = accentOn ? (1.0f + accent) : 1.0f;
        const float mod = envMod * filterEnv * accentBoost;
        float fc = cutoffHz * std::exp2 (mod * 4.0f);
        fc = std::clamp (fc, 20.0f, 0.45f * sr);

        // 4-pole ladder with tanh saturation in the feedback path, run at 2x rate
        const float f = 1.0f - std::exp (-6.2831853f * fc / (2.0f * sr));
        const float k = resonance * 4.0f;   // ~4 = self-oscillation
        for (int i = 0; i < 2; ++i)
        {
            const float x = std::tanh (osc - k * y[3]);
            y[0] += f * (x    - y[0]);
            y[1] += f * (y[0] - y[1]);
            y[2] += f * (y[1] - y[2]);
            y[3] += f * (y[2] - y[3]);
        }

        // VCA
        ampEnv += ((gate ? 1.0f : 0.0f) - ampEnv) * (gate ? ampAttack : ampRelease);
        const float level = accentOn ? (1.0f + 0.5f * accent) : 1.0f;
        return y[3] * ampEnv * level * 0.8f;
    }

private:
    static float polyBlep (float t, float dt)
    {
        if (t < dt)         { t /= dt;          return t + t - t * t - 1.0f; }
        if (t > 1.0f - dt)  { t = (t - 1.0f) / dt; return t * t + t + t + 1.0f; }
        return 0.0f;
    }

    float sr = 44100.0f;
    float phase = 0.0f, freq = 110.0f, targetFreq = 110.0f;
    float filterEnv = 0.0f, ampEnv = 0.0f;
    float y[4] = { 0, 0, 0, 0 };
    float envDecayCoef = 0.999f, glideCoef = 0.001f, ampAttack = 0.01f, ampRelease = 0.01f;
    bool  gate = false, accentOn = false;
    int   currentNote = -1;
};
