#include "Instance.h"
#include "Logger.h"

using namespace patch;

Instance::Instance()
    : maxBufferSize(0)
    , fs(0)
    , mMode(Mode::bypass)
    , mPreviousMode(Mode::bypass)
    , mCorePtr(Core::getInstance())
    , id()
{
    mCorePtr->registerInstance(this);
}

Instance::~Instance() 
{
    mCorePtr->tryDeleteInstance(getId());
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
    juce::ScopedLock lock(mcs);

    if (fCoreState == hasNotFinished) 
    {
        MY_LOG_INFO ("Inst {}: Calling Core =========================",
                     id);
        mCorePtr->processRouting(buffer.getNumSamples());
    }

    if (mMode == Mode::transmit)
    {
        MY_LOG_INFO ("Inst {}: Sending buffer of size {}", 
                     id,
                     buffer.getNumSamples());
        mCorePtr->bufferForNextBlock(getId(), buffer);
        buffer.clear();
    }
    else if (mMode == Mode::recieve)
    {
        MY_LOG_INFO ("Inst {}: Loading buffer of size {}", 
                     id,
                     buffer.getNumSamples());
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
    mCorePtr->instanceSwitchedMode(this, mPreviousMode);
    mPreviousMode = mMode;
}
