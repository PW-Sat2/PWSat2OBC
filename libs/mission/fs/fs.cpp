#include "fs.hpp"

using namespace std::literals;

namespace mission
{
    FileSystemTask::FileSystemTask(services::fs::IYaffsDeviceOperations& deviceOperations)
        : _deviceOperations{deviceOperations}, _lastUpdate{None<std::chrono::milliseconds>()}
    {
    }

    ActionDescriptor<SystemState> FileSystemTask::BuildAction()
    {
        ActionDescriptor<SystemState> descriptor;
        descriptor.name = "Create YAFFS Checkpoint Action";
        descriptor.param = this;
        descriptor.condition = CanCreateCheckpoint;
        descriptor.actionProc = CreateCheckpoint;
        return descriptor;
    }

    void FileSystemTask::CreateCheckpoint(SystemState& state, void* param)
    {
        auto that = static_cast<FileSystemTask*>(param);
        that->_deviceOperations.Sync();

        auto currentTime = state.Time;
        that->_lastUpdate = Some(currentTime);
    }

    bool FileSystemTask::CanCreateCheckpoint(const SystemState& state, void* param)
    {
        auto that = static_cast<FileSystemTask*>(param);
        auto currentTime = state.Time;

        if (!that->_lastUpdate.HasValue)
        {
            that->_lastUpdate = Some(currentTime);
        }

        return currentTime - that->_lastUpdate.Value >= SyncPeriod;
    }
}
