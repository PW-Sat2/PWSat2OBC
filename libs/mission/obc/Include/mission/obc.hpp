#ifndef MISSION_OBC_OBC_HPP
#define MISSION_OBC_OBC_HPP

#pragma once

#include <chrono>

namespace mission
{
    bool IsInitialSilenPeriodFinished(const std::chrono::milliseconds& currentTime);
}

#endif
