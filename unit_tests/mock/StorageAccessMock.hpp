#ifndef STORAGE_ACCESS_MOCK_HPP
#define STORAGE_ACCESS_MOCK_HPP

#pragma once

#include "gmock/gmock.h"
#include "obc/IStorageAccess.hpp"

struct StorageAccessMock : public obc::IStorageAccess
{
    ~StorageAccessMock();

    MOCK_METHOD2(Read, void(std::uint32_t address, gsl::span<std::uint8_t> span));

    MOCK_METHOD2(Write, void(std::uint32_t address, gsl::span<const std::uint8_t> span));
};

#endif
