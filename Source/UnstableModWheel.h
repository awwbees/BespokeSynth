/*
  ==============================================================================

    UnstableModWheel.h
    Created: 2 Mar 2021 7:48:49pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "Checkbox.h"
#include "ModulationChain.h"
#include "Transport.h"
#include "UnstablePitch.h"

class UnstableModWheel : public NoteEffectBase, public IDrawableModule, public IFloatSliderListener, public IAudioPoller
{
public:
   UnstableModWheel();
   virtual ~UnstableModWheel();
   static IDrawableModule* Create() { return new UnstableModWheel(); }

   string GetTitleLabel() override { return "unstable modwheel"; }
   void CreateUIControls() override;
   void Init() override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;

   //IAudioPoller
   void OnTransportAdvanced(float amount) override;

   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void CheckboxUpdated(Checkbox* checkbox) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   bool Enabled() const override { return mEnabled; }

   void FillModulationBuffer(double time, int voiceIdx);

   UnstablePerlinModulation mPerlin;
   FloatSlider* mAmountSlider;
   FloatSlider* mWarbleSlider;
   FloatSlider* mNoiseSlider;
   float mWidth;
   float mHeight;
   std::array<bool, kNumVoices> mIsVoiceUsed{ false };
   std::array<int, 128> mPitchToVoice;
   int mVoiceRoundRobin;

   Modulations mModulation;
};

