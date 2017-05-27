#include "TimeCorrectionProvider.hpp"

using namespace mission;

TimeCorrectionProvider::TimeCorrectionProvider(SystemState& systemState) : _systemState(systemState)
{
}

int16_t TimeCorrectionProvider::GetCurrentTimeCorrectionFactor()
{
    return _systemState.PersistentState.Get<state::TimeCorrectionConfiguration>().MissionTimeFactor();
}

int16_t TimeCorrectionProvider::GetCurrentExternalTimeCorrectionFactor()
{
    return _systemState.PersistentState.Get<state::TimeCorrectionConfiguration>().ExternalTimeFactor();
}

bool TimeCorrectionProvider::SetCurrentTimeCorrectionFactor(int16_t factor)
{
    auto newCorrector = state::TimeCorrectionConfiguration(factor, GetCurrentExternalTimeCorrectionFactor());
    _systemState.PersistentState.Set<state::TimeCorrectionConfiguration>(newCorrector);
    return true;
}

bool TimeCorrectionProvider::SetCurrentExternalTimeCorrectionFactor(int16_t factor)
{
    auto newCorrector = state::TimeCorrectionConfiguration(GetCurrentTimeCorrectionFactor(), factor);
    _systemState.PersistentState.Set<state::TimeCorrectionConfiguration>(newCorrector);
    return true;
}

/** @} */
