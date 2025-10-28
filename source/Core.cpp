#include "Core.h"
#include "Logger.h"

using namespace patch;

void Core::registerInstance(Instance* ptr)
{
    while(checkForUuidMatch(ptr->getId()))
    {
        juce::Uuid id;
        ptr->setId(InstanceAccessToken{}, id);
    }

    mBypassedInstances.emplace(ptr->getId(), ptr);
}

void Core::tryDeleteInstance(juce::Uuid id)
{
    Instance* instancePtr = findInstanceById(id);
    if(instancePtr == nullptr) return;

    instancePtr->setMode(Mode::bypass);

    mBypassedInstances.erase(id);
}

void Core::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ScopedLock lock(mBufferOperation);

    mSampleRate = sampleRate;
    mMaxBufferSize = samplesPerBlock;

    for(auto& kv : mBuffers)
    {
        auto& bufferPair = kv.second;

        bufferPair.first.setSize(2, mMaxBufferSize);
        bufferPair.second.setSize(2, mMaxBufferSize);
    }
}

void Core::processRouting(int incomingSize)
{
    juce::ScopedLock lock(mBufferOperation);

    for(auto& kv : mBuffers)
    {
        auto& bufferPair = kv.second;
        auto& delayBuffer = bufferPair.first;
        auto& transitBuffer = bufferPair.second;

        for (int ch = 0; ch < 2; ch++)
            for (int s = 0; s < mTransitLength; s++)
                delayBuffer.getChannel(ch)->push(transitBuffer.getSample(ch, s));

        transitBuffer.clear();
    }

    mTransitLength = 0;

    for (auto& instkv : mBypassedInstances)
    {
        instkv.second->setCoreFinished();
    }

    for (auto& instkv : mRecieverInstances)
    {
        instkv.second->setCoreFinished();

        for(auto& bufferkv : mBuffers)
        {
            const juce::Uuid& transmitterId = bufferkv.first;
            const juce::Uuid& recieverId = instkv.first;
            const ConnectionParameters* params = getConnectionParameters(transmitterId, recieverId);
            const bool isConnected = params->on.getValue();
            if(!isConnected) continue;

            const float gain = params->gain.getValue();
            
            const auto& bufferPair = bufferkv.second;

            for (int ch = 0; ch < 2; ch++)
            {
                for (ptrdiff_t s = 0; s < incomingSize; s++)
                {
                    float sample = bufferPair.first.getChannel(ch)->operator[](s);
                    sample *= gain;
                    instkv.second->getRecieveBuffer()->setSample(ch, (int)s, sample);
                }
            }
        }
    }

    for (auto instkv : mTransmitterInstances)
    {
        instkv.second->setCoreFinished();
    }
}

void Core::releaseResources() 
{
    // Make sure this is safe to call multiple times, because every instance
    // will call this is their constructor
}

void Core::instanceSwitchedMode(Instance* ptr, Mode previousMode)
{    
    juce::ScopedLock lock(mBufferOperation);

    if(previousMode == ptr->getMode()) return;

    // deassign from previous Mode responsibilities
    switch (previousMode)
    {
        case Mode::transmit :
            mTransmitterInstances.erase(ptr->getId());
            mMatrix.erase(ptr->getId());
            mBuffers.erase(ptr->getId());
            updateConnectionList(Mode::recieve);
            break;
        case Mode::recieve :
            mRecieverInstances.erase(ptr->getId());
            for (auto& parameterVectorKv : mMatrix)
            {
                auto& parameterVector = parameterVectorKv.second;
                parameterVector.erase(ptr->getId());
            }
            updateConnectionList(Mode::transmit);
            break;
        case Mode::bypass :
            mBypassedInstances.erase(ptr->getId());
        default :
            break;
    }

    // assign to new Mode responsibilities
    switch(ptr->getMode())
    {
        case Mode::bypass :
            mBypassedInstances.emplace(ptr->getId(), ptr);
            break;
        case Mode::recieve :
            mRecieverInstances.emplace(ptr->getId(), ptr);
            for (auto& parameterVectorKv : mMatrix)
            {
                auto& parameterVector = parameterVectorKv.second;
                parameterVector.emplace(ptr->getId(), 
                                        std::make_unique<ConnectionParameters>());
            }
            updateConnectionList(Mode::transmit);
            break;
        case Mode::transmit :
            mTransmitterInstances.emplace(ptr->getId(), ptr);
            {
                ParameterVector vector{};
                vector.reserve(mRecieverInstances.size());
                for(auto& recieverkv : mRecieverInstances)
                {
                    vector.emplace (recieverkv.first, 
                                    std::make_unique<ConnectionParameters>());
                }
                mMatrix.emplace(ptr->getId(), std::move(vector));
            
                Buffer transmitterBufferPair = {MCCBuffer(), juce::AudioBuffer<float>()};
                transmitterBufferPair.first.setSize(2 /*hardcoded for now*/, mMaxBufferSize);
                transmitterBufferPair.second.setSize(2 /*hardcoded for now*/, mMaxBufferSize);
                transmitterBufferPair.second.clear();
                mBuffers.emplace(ptr->getId(), std::move(transmitterBufferPair));
            }
            updateConnectionList(Mode::recieve);
            break;
        default :
            break;
    }
}

// Transmitter Instance -> Core
void Core::bufferForNextBlock(juce::Uuid id, juce::AudioBuffer<float>& buffer)
{
    juce::ScopedLock lock(mBufferOperation);

    mTransitLength = buffer.getNumSamples();

    for (int ch = 0; ch < 2; ch++)
    {
        mBuffers[id].second.addFrom(
            ch,
            0,
            buffer.getReadPointer(ch, 0),
            mTransitLength
        );
    }
}

bool Core::checkForUuidMatch(const juce::Uuid& id)
{
    if(mBypassedInstances.contains(id)) return true;
    if(mTransmitterInstances.contains(id)) return true;
    if(mRecieverInstances.contains(id)) return true;

    return false;
}

ConnectionParameters* Core::getConnectionParameters(juce::Uuid transmitter, juce::Uuid reciever)
{
    if(!mMatrix.contains(transmitter)) return nullptr;
    if(!mMatrix[transmitter].contains(reciever)) return nullptr;
    return mMatrix[transmitter][reciever].get();
}

Instance* Core::findInstanceById(juce::Uuid id)
{
    for(auto instancelist : {mBypassedInstances, mTransmitterInstances, mRecieverInstances})
    {
        for(auto ptrkv : instancelist)
        {
            if(ptrkv.first == id)
                return ptrkv.second;
        }
    }
    return nullptr;
}

void Core::updateConnectionList(Mode mode)
{
    switch(mode)
    {
        case Mode::recieve :
            for(auto ptrkv : mRecieverInstances)
            {
                auto func = ptrkv.second->updateConnectionList;
                if(func)
                    func();
            }
            return;
        case Mode::transmit :
            for(auto ptrkv : mTransmitterInstances)
            {
                auto func = ptrkv.second->updateConnectionList;
                if(func)
                    func();
            }
            return;
        case Mode::bypass :
        default:
            return;
    }
}
