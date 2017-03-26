#ifndef LIBS_OBC_STATE_FWD_HPP
#define LIBS_OBC_STATE_FWD_HPP

#pragma once

#include <cstdint>
#include "state/fwd.hpp"

namespace obc
{
    struct IStorageAccess;

    bool ReadPersistentState(state::PersistentState& state, std::uint32_t baseAddress, IStorageAccess& storage);

    bool WritePersistentState(const state::PersistentState& state, std::uint32_t baseAddress, IStorageAccess& storage);
}

#endif
