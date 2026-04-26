/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

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
//  LooperBank.h
//  Bespoke
//
//  Created by Ryan Challinor on 1/22/15.
//
//

#pragma once

#include "EnvOscillator.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "Slider.h"
#include "DropdownList.h"
#include "Transport.h"
#include "ClickButton.h"
#include "OpenFrameworksPort.h"

class Sample;
class Looper;
class ChannelBuffer;

class LooperBank : public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener, public IButtonListener, public ITimeListener
{
public:
   LooperBank();
   ~LooperBank();
   static IDrawableModule* Create() { return new LooperBank(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Init() override;

   void Poll() override;
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   int GetRowY(int idx);
   Looper* GetLooper() { return mLooper; }
   int GetQueuedBufferIndex() { return mQueuedBufferIndex; }

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void OnTimeEvent(double time) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void DropdownClicked(DropdownList* list) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   bool ShouldSerializeForSnapshot() const override { return true; }
   void SaveSnapshotData(FileStreamOut& out, int snapshotIndex) override;
   void LoadSnapshotData(FileStreamIn& in, int snapshotIndex) override;
   int GetModuleSaveStateRev() const override { return 0; }

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;

   class LoopData
   {
   public:
      LoopData();
      ~LoopData();

      void Init(LooperBank* bank, int index);
      void Draw();
      void StoreTo();
      void LoadFrom();

      ChannelBuffer* mBuffer{ nullptr };
      int mNumBars{ 1 };
      ClickButton* mStoreButton{ nullptr };
      ClickButton* mLoadButton{ nullptr };
      LooperBank* mLooperBank{ nullptr };
      int mIndex{ 0 };
   };

   Looper* mLooper{ nullptr };
   DropdownList* mQuantizationDropdown{ nullptr };
   NoteInterval mQuantization{ NoteInterval::kInterval_None };
   int mQueuedBufferIndex{ -1 };

   std::vector<LoopData*> mLoops;

   PatchCableSource* mLooperCable{ nullptr };
};
