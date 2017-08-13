#include "PayloadExperimentTelemetryProvider.hpp"

using namespace experiment::payload;
using experiments::fs::ExperimentFile;

using namespace devices::eps;

PayloadExperimentTelemetryProvider::PayloadExperimentTelemetryProvider(IEpsTelemetryProvider& epsProvider,
    error_counter::IErrorCountingTelemetryProvider* errorCounterProvider,
    temp::ITemperatureReader* temperatureProvider,
    experiments::IExperimentController* experimentProvider)
    : _epsProvider(epsProvider), _errorCounterProvider(errorCounterProvider), _temperatureProvider(temperatureProvider),
      _experimentProvider(experimentProvider)
{
}

void PayloadExperimentTelemetryProvider::Save(ExperimentFile& file)
{
    std::array<uint8_t, 232> buffer;
    buffer.fill(0xEE);
    BitWriter w(buffer);

    _telemetry.Set(hk::ControllerATelemetry(_epsProvider.ReadHousekeepingA().Value));
    _telemetry.Set(hk::ControllerBTelemetry(_epsProvider.ReadHousekeepingB().Value));

    ErrorCountingTelemetry::Container counters;
    counters.fill(0);

    if (_errorCounterProvider)
    {
        const auto& fdir = _errorCounterProvider->ErrorCounting();
        for (size_t i = 0; i < counters.size(); ++i)
        {
            counters[i] = fdir.Current(i);
        }
    }

    _telemetry.Set(ErrorCountingTelemetry(counters));

    const auto value = this->_temperatureProvider->ReadRaw();
    _telemetry.Set(McuTemperature(value));

    _telemetry.Set(ExperimentTelemetry(this->_experimentProvider->CurrentState()));

    _telemetry.Write(w);
    file.Write(ExperimentFile::PID::PayloadExperimentMainTelemetry, w.Capture());
}
