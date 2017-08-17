#ifndef MOCK_PHOTO_SERVICE_MOCK_HPP
#define MOCK_PHOTO_SERVICE_MOCK_HPP

#pragma once

#include "gmock/gmock.h"
#include "photo/photo_service.hpp"

struct PhotoServiceMock : services::photo::IPhotoService
{
    PhotoServiceMock();

    ~PhotoServiceMock();

    MOCK_METHOD1(DisableCamera, void(services::photo::Camera which));

    MOCK_METHOD1(EnableCamera, void(services::photo::Camera which));

    MOCK_METHOD2(TakePhoto, void(services::photo::Camera which, services::photo::PhotoResolution resolution));

    MOCK_METHOD2(DownloadPhoto, void(services::photo::Camera which, std::uint8_t bufferId));

    MOCK_METHOD0(Reset, void());

    MOCK_METHOD2(SavePhotoToFile, void(std::uint8_t bufferId, const char* pathFmt));

    MOCK_METHOD1(Sleep, void(std::chrono::milliseconds duration));

    MOCK_METHOD1(WaitForFinish, bool(std::chrono::milliseconds timeout));

    MOCK_METHOD1(GetLastSyncResult, const services::photo::SyncResult(services::photo::Camera which));

    MOCK_CONST_METHOD1(IsEmpty, bool(std::uint8_t bufferId));
    MOCK_METHOD0(PurgePendingCommands, void());

    virtual void SavePhoto(std::uint8_t bufferId, const char* pathFmt, ...) override
    {
        SavePhotoToFile(bufferId, pathFmt);
    }
};

#endif
