#include "Instance.h"

using namespace patch;

Instance::Instance()
    : maxBufferSize(0)
    , fs(0)
    , mMode(Mode::bypass)
    , mCorePtr(Core::getInstance())
{
    mCorePtr->registerInstance(this);
}

Instance::~Instance() 
{
    mCorePtr->tryDeleteInstance(this);
}


void Instance::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    maxBufferSize = samplesPerBlock;
    fs = sampleRate;
    mCorePtr->prepareToPlay(sampleRate, samplesPerBlock);
    
    // only two-channels setups are supported in this version
    mRecieveBuffer.setSize(2, maxBufferSize);
}

void Instance::releaseResources()
{
    mCorePtr->releaseResources();
}

void Instance::processBlock(juce::AudioBuffer<float>& buffer)
{
    if (fCoreState == hasNotFinished) 
    {
        mCorePtr->processRouting(buffer.getNumSamples());
    }

    if (mMode == Mode::send)
    {
        mCorePtr->bufferForNextBlock(buffer);
        buffer.clear();
    }
    else if (mMode == Mode::recieve)
    {
        for (int ch = 0; ch < 2; ch++)
        {
            buffer.copyFrom (
                ch,
                0,
                mRecieveBuffer.getReadPointer(ch),
                buffer.getNumSamples()
            );
        }
    }
    
    fCoreState = hasNotFinished;
}

void Instance::setMode(Mode mode)
{
    mMode = mode;
}
