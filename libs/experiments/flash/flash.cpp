#include "flash.hpp"
#include "comm/ITransmitter.hpp"
#include "logger/logger.h"
#include "n25q/n25q.h"
#include "telecommunication/downlink.h"
#include "yaffs.hpp"

using telecommunication::downlink::DownlinkAPID;
using telecommunication::downlink::CorrelatedDownlinkFrame;

namespace experiment
{
    namespace erase_flash
    {
        EraseFlashExperiment::EraseFlashExperiment(devices::n25q::RedundantN25QDriver& n25q, devices::comm::ITransmitter& transmitter)
            : _n25q(n25q), _transmitter(transmitter), _correlationId(0xBC)
        {
        }

        experiments::ExperimentCode EraseFlashExperiment::Type()
        {
            return Code;
        }

        experiments::StartResult EraseFlashExperiment::Start()
        {
            return experiments::StartResult::Success;
        }

        experiments::IterationResult EraseFlashExperiment::Iteration()
        {
            yaffsfs_Lock();

            auto result = _n25q.EraseChip();

            yaffsfs_Unlock();

            CorrelatedDownlinkFrame frame(DownlinkAPID::Operation, 0, 0x67);
            if (result == devices::n25q::OperationResult::Success)
            {
                frame.PayloadWriter().WriteByte(0);
                frame.PayloadWriter().WriteByte(2);
            }
            else
            {
                frame.PayloadWriter().WriteByte(3);
                frame.PayloadWriter().WriteByte(num(result));
            }

            _transmitter.SendFrame(frame.Frame());

            return experiments::IterationResult::Finished;
        }

        void EraseFlashExperiment::Stop(experiments::IterationResult /*lastResult*/)
        {
        }
    }
}
