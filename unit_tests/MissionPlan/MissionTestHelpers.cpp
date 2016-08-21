#include <ostream>
#include "state/state.h"

void showValue(const SystemState& state, std::ostream& os)
{
    os << "time=" << state.Time << ", "
       << "antennaDeployed=" << state.AntennaDeployed;
}
