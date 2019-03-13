#ifndef LIBS_STATE_INCLUDE_STATE_STRUCT_H_
#define LIBS_STATE_INCLUDE_STATE_STRUCT_H_

#pragma once

#include <chrono>

/**
 * @ingroup StateDef
 * @brief State of the satellite
 */
struct SystemState
{
    SystemState();

    /** @brief Current time */
    std::chrono::milliseconds Time;
};

#endif /* LIBS_STATE_INCLUDE_STATE_STRUCT_H_ */
