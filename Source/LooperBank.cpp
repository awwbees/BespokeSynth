/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2025 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
//
//  LooperBank.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 1/22/15.
//
//

#include "LooperBank.h"
#include "IAudioReceiver.h"
#include "Sample.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Looper.h"
#include "FillSaveDropdown.h"
#include "PatchCableSource.h"
#include "ChannelBuffer.h"

LooperBank::LooperBank()
{
}

void LooperBank::Init()
{
   IDrawableModule::Init();

   TheTransport->AddListener(this, kInterval_None, OffsetInfo(0, true), false);
}

void LooperBank::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mQuantizationDropdown = new DropdownList(this, "quantization", 135, 2, ((int*)&mQuantization));

   mQuantizationDropdown->AddLabel("none", kInterval_None);
   mQuantizationDropdown->AddLabel("4n", kInterval_4n);
   mQuantizationDropdown->AddLabel("1n", kInterval_1n);

   mLooperCable = new PatchCableSource(this, kConnectionType_Special);
   mLooperCable->AddTypeFilter("looper");
   AddPatchCableSource(mLooperCable);
}

LooperBank::~LooperBank()
{
   for (int i = 0; i < mLoops.size(); ++i)
      delete mLoops[i];
   TheTransport->RemoveListener(this);
}

void LooperBank::Poll()
{
   if (mLooper == nullptr)
      return;

   assert(mLoops[0]->mBuffer != mLoops[1]->mBuffer);

   assert(mLoops[0]->mBuffer != mLoops[1]->mBuffer);
}

void LooperBank::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mQuantizationDropdown->Draw();

   for (int i = 0; i < mLoops.size(); ++i)
      mLoops[i]->Draw();
   assert(mLoops[0]->mBuffer != mLoops[1]->mBuffer);
}

void LooperBank::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   mLooper = dynamic_cast<Looper*>(cableSource->GetTarget());
}

int LooperBank::GetRowY(int idx)
{
   return 20 + idx * 40;
}

void LooperBank::OnTimeEvent(double time)
{
   /*if (mQueuedSwapBufferIdx != -1)
   {
      SwapBuffer(mQueuedSwapBufferIdx);
      mQueuedSwapBufferIdx = -1;
   }*/
}

void LooperBank::DropdownClicked(DropdownList* list)
{
}

void LooperBank::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mQuantizationDropdown)
   {
      TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
      if (transportListenerInfo != nullptr)
         transportListenerInfo->mInterval = mQuantization;
   }
}

void LooperBank::ButtonClicked(ClickButton* button, double time)
{
   for (auto& loop : mLoops)
   {
      if (button == loop->mStoreButton)
         loop->StoreTo();
      if (button == loop->mLoadButton)
         loop->LoadFrom();
   }
}

void LooperBank::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void LooperBank::GetModuleDimensions(float& width, float& height)
{
   width = 210;
   height = GetRowY((int)mLoops.size());
}

void LooperBank::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void LooperBank::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void LooperBank::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("looper", moduleInfo, "", FillDropdown<Looper*>);
   mModuleSaveData.LoadInt("numloops", moduleInfo, 4, 1, 16, K(isTextField));
   mModuleSaveData.LoadEnum<NoteInterval>("quantization", moduleInfo, kInterval_None, mQuantizationDropdown);

   SetUpFromSaveData();
}

void LooperBank::SaveLayout(ofxJSONElement& moduleInfo)
{
   moduleInfo["looper"] = mLooper ? mLooper->Name() : "";
}

void LooperBank::SetUpFromSaveData()
{
   mLooperCable->SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("looper"), false));

   for (int i = 0; i < mLoops.size(); ++i)
      delete mLoops[i];
   mLoops.resize(mModuleSaveData.GetInt("numloops"));
   for (int i = 0; i < mLoops.size(); ++i)
   {
      mLoops[i] = new LoopData();
      mLoops[i]->Init(this, i);
   }

   mQuantization = mModuleSaveData.GetEnum<NoteInterval>("quantization");
   TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
   if (transportListenerInfo != nullptr)
      transportListenerInfo->mInterval = mQuantization;
}

void LooperBank::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   for (auto* loopData : mLoops)
   {
      out << loopData->mNumBars;
      loopData->mBuffer->Save(out, loopData->mBuffer->BufferSize());
   }
}

void LooperBank::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   for (auto* loopData : mLoops)
   {
      in >> loopData->mNumBars;
      int readLength;
      loopData->mBuffer->Load(in, readLength, ChannelBuffer::LoadMode::kAnyBufferSize);
   }
}

void LooperBank::SaveSnapshotData(FileStreamOut& out, int snapshotIndex)
{
   if (snapshotIndex >= 0 && snapshotIndex < mLoops.size())
      mLoops[snapshotIndex]->StoreTo();
}

void LooperBank::LoadSnapshotData(FileStreamIn& in, int snapshotIndex)
{
   if (snapshotIndex >= 0 && snapshotIndex < mLoops.size())
      mLoops[snapshotIndex]->LoadFrom();
}

LooperBank::LoopData::LoopData()
{
}

LooperBank::LoopData::~LoopData()
{
}

void LooperBank::LoopData::Init(LooperBank* bank, int index)
{
   mLooperBank = bank;
   mIndex = index;

   mBuffer = new ChannelBuffer(MAX_BUFFER_SIZE);
   mNumBars = 1;

   std::string indexStr = ofToString(index + 1);

   int y = bank->GetRowY(index);
   mStoreButton = new ClickButton(bank, ("store " + indexStr).c_str(), 110, y + 20);
   mLoadButton = new ClickButton(bank, ("load " + indexStr).c_str(), 170, y + 20);
}

void LooperBank::LoopData::Draw()
{
   ofPushMatrix();
   ofTranslate(5, mLooperBank->GetRowY(mIndex));
   int bufferLength = int(mNumBars * TheTransport->MsPerBar() * gSampleRateMs);
   DrawAudioBuffer(100, 36, mBuffer, 0, bufferLength, -1);
   DrawTextNormal(ofToString(mNumBars), 4, 12);
   if (mLooperBank->GetQueuedBufferIndex() == mIndex)
   {
      ofPushStyle();
      ofSetColor(255, 100, 0);
      ofFill();
      ofRect(107, 24, 8, 8);
      ofPopStyle();
   }
   ofPopMatrix();
   mStoreButton->Draw();
   mLoadButton->Draw();
}

void LooperBank::LoopData::StoreTo()
{
   int numSamples;
   ChannelBuffer* buffer = mLooperBank->GetLooper()->GetLoopBuffer(numSamples);
   mNumBars = mLooperBank->GetLooper()->GetNumBars();
   mBuffer->CopyFrom(buffer, numSamples);
}

void LooperBank::LoopData::LoadFrom()
{
   mLooperBank->GetLooper()->SetNumBars(mNumBars);
   int loopLengthSamples = int(mNumBars * TheTransport->MsPerBar() * gSampleRateMs);
   mLooperBank->GetLooper()->Commit(mBuffer, nullptr, loopLengthSamples, K(replaceOnCommit), 0);
}
