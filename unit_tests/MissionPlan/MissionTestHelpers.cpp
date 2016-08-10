#include <ostream>
#include "mission/state.h"

void showValue(const SystemState& state, std::ostream& os)
{
    os << "time=" << TimePointToTimeSpan(state.Time).value << ", "
       << "antennaDeployed=" << state.AntennaDeployed;
}
