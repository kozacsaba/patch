#pragma once

/*  ConnectionParameter
    This struct shoule be used to describe the conneciton between a Transmitter
    instance and a Reciever instance.
*/

enum class OverdriveProtection
{
    off,
    clip
};

struct ConnectionParameters
{
    float gain = 0.f;

    // these are not implemented just yet

    int delay = 0;
    bool delayCorrection = false;
    OverdriveProtection protection = OverdriveProtection::off;
};