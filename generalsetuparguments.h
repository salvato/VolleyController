#pragma once
#include <QString>


class GeneralSetupArguments
{
public:
    GeneralSetupArguments();

public:
    int        maxTimeout;
    int        maxSet;
    int        iTimeoutDuration;

    QString    sSlideDir;
    QString    sSpotDir;
};
