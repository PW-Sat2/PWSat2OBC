#ifndef LIBS_OBC_STATE_FWD_HPP
#define LIBS_OBC_STATE_FWD_HPP

#pragma once

#include <cstdint>
#include "state/fwd.hpp"

/**
 * @addtogroup StateDef
 * @{
 */
namespace obc
{
    struct IStorageAccess;

    /**
     * @brief This procedure is responsible for reading the system persistent state that is stored at provided
     * address using the provided memory controller.
     *
     * @param[out] state Object that should be used to receive the deserialized state read from the memory.
     * @param[in] baseAddress Persistent state base address.
     * @param[in] storage Memory controller that should be used to access that serialized state.
     */
    bool ReadPersistentState(state::SystemPersistentState& state, std::uint32_t baseAddress, IStorageAccess& storage);

    /**
     * @brief This procedure is responsible for writing the serialized format of system persistent state at specific
     * address using the provided memory controller.
     *
     * @param[in] state Object that should be serialized and saved to the memory at passed address.
     * @param[in] baseAddress Persistent state base address.
     * @param[in] storage Memory controller that should be used to save the serialized state.
     */
    bool WritePersistentState(const state::SystemPersistentState& state, std::uint32_t baseAddress, IStorageAccess& storage);
}

/** @} */
#endif
