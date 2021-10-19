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
/*
  ==============================================================================

    ChannelBuffer.cpp
    Created: 10 Oct 2017 8:53:51pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "ChannelBuffer.h"

ChannelBuffer::ChannelBuffer(int bufferSize)
{
   mActiveChannels = 1;
   mRecentActiveChannels = 1;
   mOwnsBuffers = true;
   
   Setup(bufferSize, kMaxNumChannels);
}

ChannelBuffer::ChannelBuffer(float* data, int bufferSize)
{
   //intended as a temporary holder for passing raw data to methods that want a ChannelBuffer
   
   mActiveChannels = 1;
   mNumChannels = 1;
   mOwnsBuffers = false;
   
   mBuffers[0] = data;
   mBufferSize = bufferSize;
}

ChannelBuffer::~ChannelBuffer()
{
   if (mOwnsBuffers)
   {
      for (int i=0; i<mNumChannels; ++i)
         delete[] mBuffers[i];
   }
}

void ChannelBuffer::Setup(int bufferSize, int numChannels)
{
   mNumChannels = numChannels;
   mBufferSize = bufferSize;
   
   for (int i = 0; i < mNumChannels; ++i)
   {
      assert(mOwnsBuffers);
      mBuffers[i] = new float[BufferSize()];
      ::Clear(mBuffers[i], BufferSize());
   }
}

void ChannelBuffer::Clear() const
{
   for (int i=0; i<mNumChannels; ++i)
      ::Clear(mBuffers[i], BufferSize());
}

void ChannelBuffer::CopyFrom(ChannelBuffer* src, int length /*= -1*/, int startOffset /*= 0*/)
{
   if (length == -1)
      length = mBufferSize;
   assert(length <= mBufferSize);
   assert(length + startOffset <= src->mBufferSize);
   mActiveChannels = src->mActiveChannels;
   mNumChannels = src->mNumChannels;
   for (int i=0; i < mNumChannels; ++i)
   {
      if (src->mBuffers[i])
      {
         if (mBuffers[i] == nullptr)
         {
            assert(mOwnsBuffers);
            mBuffers[i] = new float[mBufferSize];
            ::Clear(mBuffers[i], mBufferSize);
         }
         BufferCopy(mBuffers[i], src->mBuffers[i] + startOffset, length);
      }
      else
      {
         delete mBuffers[i];
         mBuffers[i] = nullptr;
      }
   }
}

void ChannelBuffer::SetChannelPointer(float* data, int channel, bool deleteOldData)
{
   if (deleteOldData)
      delete[] mBuffers[channel];
   mBuffers[channel] = data;
}

void ChannelBuffer::Resize(int numChannels, int bufferSize)
{
   assert(mOwnsBuffers);
   for (int i=0; i<mNumChannels; ++i)
      delete[] mBuffers[i];
   
   Setup(bufferSize, numChannels);
}

namespace
{
   const int kSaveStateRev = 2;
}

void ChannelBuffer::Save(FileStreamOut& out, int writeLength)
{
   out << kSaveStateRev;
   
   out << mNumChannels;
   out << writeLength;
   out << mActiveChannels;
   for (int i = 0; i < mActiveChannels; ++i)
   {
      bool hasBuffer = mBuffers[i] != nullptr;
      out << hasBuffer;
      if (hasBuffer)
         out.Write(mBuffers[i], writeLength);
   }
}

void ChannelBuffer::Load(FileStreamIn& in, int& readLength, LoadMode loadMode)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);
   
   if (rev >= 2)
      in >> mNumChannels;
   in >> readLength;
   if (loadMode == LoadMode::kSetBufferSize)
      Setup(readLength, mNumChannels);
   else if (loadMode == LoadMode::kRequireExactBufferSize)
      assert(readLength == mBufferSize);
   else
      assert(readLength <= mBufferSize);
   in >> mActiveChannels;
   for (int i = 0; i < mActiveChannels; ++i)
   {
      bool hasBuffer = true;
      if (rev >= 1)
         in >> hasBuffer;
      
      if (hasBuffer)
         in.Read(GetChannel(i), readLength);
   }
}
