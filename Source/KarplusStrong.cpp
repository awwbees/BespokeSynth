//
//  KarplusStrong.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 2/11/13.
//
//

#include "KarplusStrong.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "IAudioReceiver.h"
#include "ModularSynth.h"
#include "Profiler.h"

KarplusStrong::KarplusStrong()
: IAudioProcessor(gBufferSize)
, mFilterSlider(nullptr)
, mFeedbackSlider(nullptr)
, mVolume(1)
, mVolSlider(nullptr)
, mSourceDropdown(nullptr)
, mInvertCheckbox(nullptr)
, mStretchCheckbox(nullptr)
, mExciterFreqSlider(nullptr)
, mExciterAttackSlider(nullptr)
, mExciterDecaySlider(nullptr)
, mPolyMgr(this)
, mNoteInputBuffer(this)
, mWriteBuffer(gBufferSize)
{
   mPolyMgr.Init(kVoiceType_Karplus, &mVoiceParams);

   AddChild(&mBiquad);
   mBiquad.SetPosition(150,15);
   mBiquad.SetEnabled(true);
   mBiquad.SetFilterType(kFilterType_Lowpass);
   mBiquad.SetFilterParams(3000, sqrt(2)/2);
   mBiquad.SetName("biquad");
   
   for (int i=0; i<ChannelBuffer::kMaxNumChannels; ++i)
   {
      mDCRemover[i].SetFilterParams(10, sqrt(2)/2);
      mDCRemover[i].SetFilterType(kFilterType_Highpass);
      mDCRemover[i].UpdateFilterCoeff();
   }
}

void KarplusStrong::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVolSlider = new FloatSlider(this,"vol",3,2,80,15,&mVolume,0,2);
   mInvertCheckbox = new Checkbox(this,"invert",mVolSlider,kAnchor_Right,&mVoiceParams.mInvert);
   mFilterSlider = new FloatSlider(this,"filter",mVolSlider,kAnchor_Below,140,15,&mVoiceParams.mFilter,0,5);
   mFeedbackSlider = new FloatSlider(this,"feedback",mFilterSlider,kAnchor_Below,140,15,&mVoiceParams.mFeedback,.9f,.9999f,4);
   mSourceDropdown = new DropdownList(this,"source type",mFeedbackSlider,kAnchor_Below,(int*)&mVoiceParams.mSourceType,52);
   mExciterFreqSlider = new FloatSlider(this,"x freq",mSourceDropdown,kAnchor_Right,85,15,&mVoiceParams.mExciterFreq,10,4000);
   mExciterAttackSlider = new FloatSlider(this,"x att",mSourceDropdown,kAnchor_Below,69,15,&mVoiceParams.mExciterAttack,0.01f,40);
   mExciterDecaySlider = new FloatSlider(this,"x dec",mExciterAttackSlider,kAnchor_Right,68,15,&mVoiceParams.mExciterDecay,0.01f,40);
   mPitchToneSlider = new FloatSlider(this, "pitchtone", mExciterAttackSlider, kAnchor_Below, 140, 15, &mVoiceParams.mPitchTone, -2, 2);
   mLiteCPUModeCheckbox = new Checkbox(this, "lite cpu", mPitchToneSlider, kAnchor_Right, &mVoiceParams.mLiteCPUMode);
   //mStretchCheckbox = new Checkbox(this,"stretch",mVolSlider,kAnchor_Right,&mVoiceParams.mStretch);
   
   mSourceDropdown->AddLabel("sin", kSourceTypeSin);
   mSourceDropdown->AddLabel("white", kSourceTypeNoise);
   mSourceDropdown->AddLabel("mix", kSourceTypeMix);
   mSourceDropdown->AddLabel("saw", kSourceTypeSaw);
   mSourceDropdown->AddLabel("input", kSourceTypeInput);
   mSourceDropdown->AddLabel("input*", kSourceTypeInputNoEnvelope);
   
   mFilterSlider->SetMode(FloatSlider::kSquare);
   mExciterFreqSlider->SetMode(FloatSlider::kSquare);
   mExciterAttackSlider->SetMode(FloatSlider::kSquare);
   mExciterDecaySlider->SetMode(FloatSlider::kSquare);
   
   mBiquad.CreateUIControls();
}

KarplusStrong::~KarplusStrong()
{
}

void KarplusStrong::Process(double time)
{
   PROFILER(KarplusStrong);

   IAudioReceiver* target = GetTarget();

   if (!mEnabled || target == nullptr)
      return;
   
   SyncBuffers(2);

   mNoteInputBuffer.Process(time);

   ComputeSliders(0);

   int bufferSize = target->GetBuffer()->BufferSize();
   assert(bufferSize == gBufferSize);

   mWriteBuffer.Clear();
   mPolyMgr.Process(time, &mWriteBuffer, bufferSize);
   
   for (int ch = 0; ch < mWriteBuffer.NumActiveChannels(); ++ch)
   {
      Mult(mWriteBuffer.GetChannel(ch), mVolume, bufferSize);
      if (!mVoiceParams.mInvert)  //unnecessary if inversion is eliminating dc offset
         mDCRemover[ch].Filter(mWriteBuffer.GetChannel(ch), bufferSize);
   }

   mBiquad.ProcessAudio(time, &mWriteBuffer);

   SyncOutputBuffer(mWriteBuffer.NumActiveChannels());
   for (int ch=0; ch<mWriteBuffer.NumActiveChannels(); ++ch)
   {
      GetVizBuffer()->WriteChunk(mWriteBuffer.GetChannel(ch),mWriteBuffer.BufferSize(), ch);
      Add(target->GetBuffer()->GetChannel(ch), mWriteBuffer.GetChannel(ch), gBufferSize);
   }

   GetBuffer()->Reset();
}

void KarplusStrong::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (!mEnabled)
      return;

   if (!NoteInputBuffer::IsTimeWithinFrame(time))
   {
      mNoteInputBuffer.QueueNote(time, pitch, velocity, voiceIdx, modulation);
      return;
   }
   
   if (velocity > 0)
      mPolyMgr.Start(time, pitch, velocity/127.0f, voiceIdx, modulation);
   else
      mPolyMgr.Stop(time, pitch);
}

void KarplusStrong::SetEnabled(bool enabled)
{
   mEnabled = enabled;
}

void KarplusStrong::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mFilterSlider->Draw();
   mFeedbackSlider->Draw();
   mVolSlider->Draw();
   mSourceDropdown->Draw();
   mInvertCheckbox->Draw();
   mPitchToneSlider->Draw();
   mLiteCPUModeCheckbox->Draw();

   mExciterFreqSlider->SetShowing(mVoiceParams.mSourceType == kSourceTypeSin || mVoiceParams.mSourceType == kSourceTypeSaw || mVoiceParams.mSourceType == kSourceTypeMix);
   mExciterAttackSlider->SetShowing(mVoiceParams.mSourceType != kSourceTypeInputNoEnvelope);
   mExciterDecaySlider->SetShowing(mVoiceParams.mSourceType != kSourceTypeInputNoEnvelope);
   
   //mStretchCheckbox->Draw();
   mExciterFreqSlider->Draw();
   mExciterAttackSlider->Draw();
   mExciterDecaySlider->Draw();

   mBiquad.Draw();
}

void KarplusStrong::DrawModuleUnclipped()
{
   if (mDrawDebug)
   {
      mPolyMgr.DrawDebug(250, 0);
   }
}

void KarplusStrong::DropdownUpdated(DropdownList* list, int oldVal)
{
}

void KarplusStrong::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void KarplusStrong::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
   {
      mPolyMgr.KillAll();
      for (int ch = 0; ch < ChannelBuffer::kMaxNumChannels; ++ch)
         mDCRemover[ch].Clear();
      mBiquad.Clear();
   }
}

void KarplusStrong::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadInt("voicelimit", moduleInfo, -1, -1, kNumVoices);
   EnumMap oversamplingMap;
   oversamplingMap["1"] = 1;
   oversamplingMap["2"] = 2;
   oversamplingMap["4"] = 4;
   oversamplingMap["8"] = 8;
   mModuleSaveData.LoadEnum<int>("oversampling", moduleInfo, 1, nullptr, &oversamplingMap);
   mModuleSaveData.LoadBool("mono", moduleInfo, false);

   SetUpFromSaveData();
}

void KarplusStrong::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));

   int voiceLimit = mModuleSaveData.GetInt("voicelimit");
   if (voiceLimit > 0)
      mPolyMgr.SetVoiceLimit(voiceLimit);

   bool mono = mModuleSaveData.GetBool("mono");
   mWriteBuffer.SetNumActiveChannels(mono ? 1 : 2);

   int oversampling = mModuleSaveData.GetEnum<int>("oversampling");
   mPolyMgr.SetOversampling(oversampling);
}


