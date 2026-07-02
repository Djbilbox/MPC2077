#pragma once

#include <JuceHeader.h>
#include "DSP/MpcConstants.h"
#include "DSP/DrumEngine.h"
#include "DSP/StepSequencer.h"
#include "DSP/FxChain.h"

//==============================================================================
class MPC2077AudioProcessor : public juce::AudioProcessor
{
public:
    MPC2077AudioProcessor();
    ~MPC2077AudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "MPC2077"; }
    bool acceptsMidi() const override  { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // --- UI access ---
    mpc::DrumEngine&    getEngine()    { return engine; }
    mpc::StepSequencer& getSequencer() { return sequencer; }
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    std::atomic<int> selectedPad { 0 };

    /** Live pad hit from the UI (queued, fired at next block start). */
    void triggerPadFromUI (int pad, float velocity, float extraSemis = 0.0f);

    /** Save / load a user preset file (.mpcpreset). */
    bool savePresetToFile (const juce::File& file);
    bool loadPresetFromFile (const juce::File& file);
    void initPatch();   // "INIT" button — clean slate

    static juce::String factoryPresetName (int index);

    // meter for master VU (editor)
    float getOutputLevel() const { return outputLevel.load(); }

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void applyFactoryPreset (int index);
    juce::ValueTree captureState();
    void restoreState (const juce::ValueTree& state);

    juce::AudioProcessorValueTreeState apvts;

    mpc::DrumEngine    engine;
    mpc::StepSequencer sequencer;
    mpc::FxChain       fx;

    juce::AudioBuffer<float> mixBuf, revBuf, delBuf;

    std::array<std::atomic<float>, mpc::kNumPads> uiPadTrigger;  // -1 = none, else velocity
    std::array<std::atomic<float>, mpc::kNumPads> uiPadPitch;    // extra semitones for UI trigger

    int currentProgram = 0;
    std::atomic<float> outputLevel { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MPC2077AudioProcessor)
};
