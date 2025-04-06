#include "Core.h"

using namespace patch;

void Core::registerInstance(Instance* ptr)
{
    mInstances.push_back(ptr);
}

void Core::tryDeleteInstance(Instance* ptr)
{
    for (int i = 0; i < mInstances.size(); i++)
    {
        if (mInstances[i] == ptr)
        {
            mInstances.erase(mInstances.begin() + i);
            return;
        }
    }
}


void Core::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    mSampleRate = sampleRate;
    mMaxBufferSize = samplesPerBlock;

    mTransitBuffer.setSize(2, mMaxBufferSize);
    mDelayBuffer.setSize(2, mMaxBufferSize);
}

void Core::processRouting(int incomingSize)
{
    for (int ch = 0; ch < 2; ch++)
        for (int s = 0; s < mTransitLength; s++)
            mDelayBuffer.getChannel(ch)->push(mTransitBuffer.getSample(ch, s));

    mTransitBuffer.clear();
    mTransitLength = 0;
        
    for (auto inst : mInstances)
    {
        inst->coreFinished();
        if (inst->getMode() != Mode::recieve) continue;

        for (int ch = 0; ch < 2; ch++)
        {
            for (ptrdiff_t s = 0; s < incomingSize; s++)
            {
                float sample = mDelayBuffer.getChannel(ch)->operator[](s);
                inst->getRecieveBuffer()->setSample(ch, (int)s, sample);
            }
        }
    }
}

void Core::releaseResources() {}

void Core::bufferForNextBlock(juce::AudioBuffer<float>& buffer)
{
    mTransitLength = buffer.getNumSamples();

    for (int ch = 0; ch < 2; ch++)
    {
        mTransitBuffer.addFrom(
            ch,
            0,
            buffer.getReadPointer(ch, 0),
            mTransitLength
        );
    }
}
