#ifndef UNIT_TESTS_MISSIONPLAN_MISSIONTESTHELPERS_H_
#define UNIT_TESTS_MISSIONPLAN_MISSIONTESTHELPERS_H_

#include <cstdint>
#include "rapidcheck.hpp"
#include "state/struct.h"

namespace rc
{
    template <> struct Arbitrary<SystemState>
    {
        static Gen<SystemState> arbitrary()
        {
            return gen::build<SystemState>(                                                //
                gen::set(&SystemState::Time, gen::arbitrary<std::chrono::milliseconds>()), //
                gen::set(&SystemState::SailOpened, gen::arbitrary<bool>())                 //
                );
        }
    };
}

void showValue(const SystemState& state, std::ostream& os);

#endif /* UNIT_TESTS_MISSIONPLAN_MISSIONTESTHELPERS_H_ */
