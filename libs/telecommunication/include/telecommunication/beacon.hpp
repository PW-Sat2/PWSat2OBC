#ifndef LIBS_TELECOMMUNICATION_INCLUDE_TELECOMMUNICATION_BEACON_HPP_
#define LIBS_TELECOMMUNICATION_INCLUDE_TELECOMMUNICATION_BEACON_HPP_

#pragma once

#include "base/fwd.hpp"
#include "telemetry/fwd.hpp"

/**
 * @brief Save to passed writer current beacon payload.
 * @param[in] state Reference to current system telemetry.
 * @param[out] writer Writer object that should be updated with latest beacon payload.
 * @return Operation status, true on success, false otherwise.
 */
bool WriteBeaconPayload(telemetry::TelemetryState& state, Writer& writer);

#endif /* LIBS_TELECOMMUNICATION_INCLUDE_TELECOMMUNICATION_BEACON_HPP_ */
