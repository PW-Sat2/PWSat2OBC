#ifndef MOCK_PHOTO_SERVICE_MOCK_HPP
#define MOCK_PHOTO_SERVICE_MOCK_HPP

#pragma once

#include "gmock/gmock.h"
#include "photo/photo_service.hpp"

struct PhotoServiceMock : services::photo::IPhotoService
{
    PhotoServiceMock();

    ~PhotoServiceMock();

    MOCK_METHOD1(Schedule, void(services::photo::DisableCamera command));

    MOCK_METHOD1(Schedule, void(services::photo::EnableCamera command));

    MOCK_METHOD1(Schedule, void(services::photo::TakePhoto command));

    MOCK_METHOD1(Schedule, void(services::photo::DownloadPhoto command));

    MOCK_METHOD1(Schedule, void(services::photo::Reset command));

    MOCK_METHOD1(Schedule, void(services::photo::SavePhoto command));

    MOCK_METHOD1(Schedule, void(services::photo::Sleep command));

    MOCK_METHOD1(WaitForFinish, bool(std::chrono::milliseconds timeout));

    MOCK_CONST_METHOD1(IsEmpty, bool(std::uint8_t bufferId));
};

#endif
