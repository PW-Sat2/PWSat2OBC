#include "fibo.h"
#include "base/writer.h"
#include "logger/logger.h"

using services::fs::File;
using services::fs::IFileSystem;
using services::fs::FileOpen;
using services::fs::FileAccess;

namespace experiment
{
    namespace fibo
    {
        Fibonacci::Fibonacci() : _counter(1), _prev(1), _current(1)
        {
        }

        void Fibonacci::Next()
        {
            this->_counter++;

            switch (this->_counter)
            {
                case 1:
                    break;
                case 2:
                    break;
                default:
                    auto a = this->_prev;
                    this->_prev = this->_current;
                    this->_current += a;
                    break;
            }
        }

        std::uint32_t Fibonacci::Current() const
        {
            return this->_current;
        }

        FibonacciExperiment::FibonacciExperiment(IFileSystem& fileSystem) : _fileSystem(fileSystem)
        {
        }

        mission::experiments::Experiment FibonacciExperiment::Type()
        {
            return mission::experiments::Experiment::Fibo;
        }

        void FibonacciExperiment::Run(mission::experiments::ExperimentContext& context)
        {
            LOG(LOG_LEVEL_INFO, "Performing Fibo experiment");

            File f(this->_fileSystem, "/fibo.dat", FileOpen::CreateAlways, FileAccess::WriteOnly);

            if (!f)
            {
                LOG(LOG_LEVEL_ERROR, "Unable to open experiments file");
                return;
            }

            Fibonacci fibo;

            for (auto i = 0; i < 5; i++)
            {
                auto v = fibo.Current();

                std::array<std::uint8_t, sizeof(v)> buf;
                Writer w(buf);
                w.WriteDoubleWordLE(v);

                f.Write(buf);

                fibo.Next();

                context.WaitForNextCycle();
            }
        }
    }
}
