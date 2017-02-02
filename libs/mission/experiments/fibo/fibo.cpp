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

        FibonacciExperiment::FibonacciExperiment(IFileSystem& fileSystem) : _fileSystem(fileSystem), _iterations(1)
        {
        }

        mission::experiments::ExperimentCode FibonacciExperiment::Type()
        {
            return Code;
        }

        void FibonacciExperiment::Iterations(std::uint32_t iterations)
        {
            this->_iterations = iterations;
        }

        mission::experiments::StartResult FibonacciExperiment::Start()
        {
            this->_file = services::fs::File(this->_fileSystem, "/fibo.dat", FileOpen::CreateAlways, FileAccess::WriteOnly);
            if (!this->_file)
            {
                LOG(LOG_LEVEL_ERROR, "Opening experiment file failed");
                return mission::experiments::StartResult::Failure;
            }

            this->_fibo = Fibonacci();

            return mission::experiments::StartResult::Success;
        }

        mission::experiments::IterationResult FibonacciExperiment::Iteration()
        {
            auto v = this->_fibo.Current();

            std::array<std::uint8_t, sizeof(v)> buf;
            Writer w(buf);
            w.WriteDoubleWordLE(v);

            this->_file.Write(buf);

            this->_fibo.Next();

            this->_iterations--;

            if (this->_iterations == 0)
            {
                return mission::experiments::IterationResult::Finished;
            }

            return mission::experiments::IterationResult::WaitForNextCycle;
        }

        void FibonacciExperiment::Stop(mission::experiments::IterationResult lastResult)
        {
            UNREFERENCED_PARAMETER(lastResult);

            this->_file.Close();
        }
    }
}
