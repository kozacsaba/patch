#include "Instance.h"
#include "Logger.h"
#include "Core.h"

using namespace patch;


Instance::Instance()
    : maxBufferSize(0)
    , fs(0)
    , mMode(Mode::bypass)
    , mPreviousMode(Mode::bypass)
    , mCorePtr(Core::getInstance())
    , id(juce::Uuid{})
{
    mCorePtr->registerInstance(this);
}

Instance::~Instance() 
{
    juce::ScopedLock lock(mcs);
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
    }
    else if (mMode == Mode::recieve)
    {
        MY_LOG_INFO ("Inst {}: Loading buffer of size {}", 
                     id,
                     buffer.getNumSamples());
        for (int ch = 0; ch < 2; ch++)
        {
            buffer.addFrom(
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

    mRecieveBuffer.clear();
}

void Instance::setId(InstanceAccessToken token, const juce::Uuid& uuid)
{
    juce::ignoreUnused(token); 
    id = uuid;
}

void Instance::setStateInfo(juce::ValueTree info)
{
    juce::ScopedLock lock(mcs);

    Core::getInstance()->tryDeleteInstance(id);
    const juce::Uuid nominalid(info.getProperty(id::uuid).toString());
    id = nominalid;
    Core::getInstance()->registerInstance(this);
    if(id != nominalid) return; // uuid (preset) already used, connections are discarded
    
    if(info.hasProperty(id::name))
        mName = info.getProperty(id::name).toString();

    const Mode mode = static_cast<Mode>((int)info.getProperty(id::mode));
    setMode(mode);

    for(const juce::ValueTree& connection : info)
    {
        const juce::Uuid transmitterid = mMode == Mode::transmit 
            ? id
            : connection.getProperty(id::uuid).toString();
        const juce::Uuid recieverid = mMode == Mode::recieve
            ? id
            : connection.getProperty(id::uuid).toString();

        ConnectionParameters* params = Core::getInstance()->getConnectionParameters(transmitterid, recieverid);
        if(params == nullptr) continue;

        params->deserialize(connection);
    }
}

juce::String Instance::getName() const
{
    return mName.value_or<juce::String>(id.toString());
}

juce::ValueTree Instance::getStateInfo()
{
    juce::ScopedLock lock(mcs);

    juce::ValueTree info(id::stateInfo);
    info.setProperty(id::uuid, id.toString(), nullptr);
    info.setProperty(id::mode, (int)mMode, nullptr);
    if(mName.has_value())
        info.setProperty(id::name, mName.value(), nullptr);

    if(mMode == Mode::transmit)
    {
        Map<Instance*>* recievers = Core::getInstance()->getRecievers();
        for(auto recieverkv : *recievers)
        {
            juce::Uuid recieverid = recieverkv.first;
            ConnectionParameters* params = Core::getInstance()->getConnectionParameters(id, recieverid);
            
            juce::ValueTree paramTree = params->serialize();
            paramTree.setProperty(id::uuid, recieverid.toString(), nullptr);
            info.addChild(paramTree, -1, nullptr);
        }
    }

    if(mMode == Mode::recieve)
    {
        Map<Instance*>* transmitters = Core::getInstance()->getTransmitters();
        for(auto transmitterkv : *transmitters)
        {
            juce::Uuid transmitterid = transmitterkv.first;
            ConnectionParameters* params = Core::getInstance()->getConnectionParameters(transmitterid, id);

            juce::ValueTree paramTree = params->serialize();
            paramTree.setProperty(id::uuid, transmitterid.toString(), nullptr);
            info.addChild(paramTree, -1, nullptr);
        }
    }

    return info;
}
