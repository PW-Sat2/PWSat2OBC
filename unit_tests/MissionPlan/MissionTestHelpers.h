#ifndef UNIT_TESTS_MISSIONPLAN_MISSIONTESTHELPERS_H_
#define UNIT_TESTS_MISSIONPLAN_MISSIONTESTHELPERS_H_

#include <cstdint>
#include "rapidcheck.hpp"
#include "state/struct.h"

namespace rc
{
    template <> struct Arbitrary<TimeSpan>
    {
        static Gen<TimeSpan> arbitrary()
        {
            auto day = gen::nonNegative<uint16_t>();
            auto hour = gen::inRange(0, 23);
            auto minute = gen::inRange(0, 59);
            auto second = gen::inRange(0, 59);
            auto milisecond = gen::inRange(0, 999);

            return gen::apply(
                [](uint16_t day, uint16_t hour, uint16_t minute, uint16_t second, uint16_t milisecond) {
                    return TimePointToTimeSpan(TimePointBuild(day, hour, minute, second, milisecond));
                },
                day,
                hour,
                minute,
                second,
                milisecond);
        }
    };

    template <> struct Arbitrary<SystemState>
    {
        static Gen<SystemState> arbitrary()
        {
            return gen::build<SystemState>(                                //
                gen::set(&SystemState::Time, gen::arbitrary<TimeSpan>()),  //
                gen::set(&SystemState::SailOpened, gen::arbitrary<bool>()) //
                );
        }
    };
}

void showValue(const SystemState& state, std::ostream& os);

#endif /* UNIT_TESTS_MISSIONPLAN_MISSIONTESTHELPERS_H_ */
