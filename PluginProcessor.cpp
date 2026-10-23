#include "PluginProcessor.h"

AcidAudioProcessor::AcidAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout AcidAudioProcessor::createLayout()
{
    using P = juce::AudioParameterFloat;
    using R = juce::NormalisableRange<float>;
    juce::AudioProcessorValueTreeState::ParameterLayout l;

    R cutoffRange (40.0f, 8000.0f);  cutoffRange.setSkewForCentre (800.0f);
    R decayRange  (50.0f, 2000.0f);  decayRange.setSkewForCentre (400.0f);

    l.add (std::make_unique<P> (juce::ParameterID { "cutoff",    1 }, "Cutoff",    cutoffRange,        300.0f));
    l.add (std::make_unique<P> (juce::ParameterID { "resonance", 1 }, "Resonance", R (0.0f, 1.0f),      0.7f));
    l.add (std::make_unique<P> (juce::ParameterID { "envmod",    1 }, "Env Mod",   R (0.0f, 1.0f),      0.6f));
    l.add (std::make_unique<P> (juce::ParameterID { "decay",     1 }, "Decay",     decayRange,         300.0f));
    l.add (std::make_unique<P> (juce::ParameterID { "accent",    1 }, "Accent",    R (0.0f, 1.0f),      0.6f));
    l.add (std::make_unique<P> (juce::ParameterID { "tune",      1 }, "Tune",      R (-100.0f, 100.0f), 0.0f));
    l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "square", 1 }, "Square", false));
    return l;
}

void AcidAudioProcessor::prepareToPlay (double sampleRate, int)
{
    synth.prepare (sampleRate);
}

bool AcidAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void AcidAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    synth.cutoffHz  = apvts.getRawParameterValue ("cutoff")->load();
    synth.resonance = apvts.getRawParameterValue ("resonance")->load();
    synth.envMod    = apvts.getRawParameterValue ("envmod")->load();
    synth.decayMs   = apvts.getRawParameterValue ("decay")->load();
    synth.accent    = apvts.getRawParameterValue ("accent")->load();
    synth.tuneCents = apvts.getRawParameterValue ("tune")->load();
    synth.square    = apvts.getRawParameterValue ("square")->load() > 0.5f;
    synth.updateCoefs();

    auto render = [&] (int start, int end)
    {
        for (int i = start; i < end; ++i)
        {
            const float s = synth.process();
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                buffer.setSample (ch, i, s);
        }
    };

    int pos = 0;
    for (const auto metadata : midi)
    {
        render (pos, metadata.samplePosition);
        pos = metadata.samplePosition;

        const auto msg = metadata.getMessage();
        if (msg.isNoteOn())
            synth.noteOn (msg.getNoteNumber(), msg.getFloatVelocity());
        else if (msg.isNoteOff())
            synth.noteOff (msg.getNoteNumber());
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
            synth.reset();
    }
    render (pos, buffer.getNumSamples());
}

juce::AudioProcessorEditor* AcidAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
}

void AcidAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void AcidAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AcidAudioProcessor();
}
