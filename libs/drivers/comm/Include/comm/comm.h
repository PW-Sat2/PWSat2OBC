#ifndef SRC_DEVICES_COMM_H_
#define SRC_DEVICES_COMM_H_

#include <stdbool.h>
#include <stdint.h>
#include <em_i2c.h>

#ifdef __cplusplus
extern "C" {
#endif

#define COMM_MAX_FRAME_CONTENTS_SIZE 235

typedef struct
{
    uint16_t Size;
    uint16_t Doppler;
    uint16_t RSSI;
    uint8_t Contents[COMM_MAX_FRAME_CONTENTS_SIZE];
} Frame;

typedef struct
{
    uint16_t TransmitterCurrentConsumption;
    uint16_t ReceiverCurrentConsumption;
    uint16_t DopplerOffset;
    uint16_t Vcc;
    uint16_t OscilatorTemperature;
    uint16_t AmplifierTemperature;
    uint16_t SignalStrength;
} CommReceiverTelemetry;

typedef struct
{
    uint16_t RFReflectedPower;
    uint16_t AmplifierTemperature;
    uint16_t RFForwardPower;
    uint16_t TransmitterCurrentConsumption;
} CommTransmitterTelemetry;

typedef struct
{
    uint16_t Period;
    uint8_t DataSize;
    uint8_t Data[COMM_MAX_FRAME_CONTENTS_SIZE];
} CommBeacon;

typedef I2C_TransferReturn_TypeDef (*I2CWriteProcedure)(uint8_t address, uint8_t* inData, uint16_t length);

typedef I2C_TransferReturn_TypeDef (*I2CWriteReadProcedure)(
    uint8_t address, uint8_t* inData, uint16_t inLength, uint8_t* outData, uint16_t outLength);

typedef struct
{
    I2CWriteProcedure writeProc;
    I2CWriteReadProcedure readProc;
} CommLowInterface;

typedef struct
{
    CommLowInterface low;
    void* commTask;
} CommObject;

typedef struct
{
    bool status;
    uint8_t frameCount;
} CommReceiverFrameCount;

typedef enum {
    CommTransmitterOff = 0,
    CommTransmitterOn = 1,
} CommTransmitterIdleState;

typedef enum {
    Comm1200bps = 1,
    Comm2400bps = 2,
    Comm4800bps = 4,
    Comm9600bps = 8,
} CommTransmitterBitrate;

typedef struct
{
    CommTransmitterIdleState StateWhenIdle;
    CommTransmitterBitrate TransmitterBitRate;
    bool BeaconState;
} CommTransmitterState;

CommObject* CommInitialize(const CommLowInterface* lowerInterface);

CommReceiverFrameCount CommGetFrameCount(CommObject* object);

bool CommRemoveFrame(CommObject* comm);

bool CommGetReceiverTelemetry(CommObject* comm, CommReceiverTelemetry* telemetry);

bool CommGetTransmitterTelemetry(CommObject* comm, CommTransmitterTelemetry* telemetry);

bool CommSendFrame(CommObject* comm, uint8_t* data, uint8_t length);

bool CommReceiveFrame(CommObject* comm, Frame* frame);

bool CommSetBeacon(CommObject* comm, const CommBeacon* beaconData);

bool CommClearBeacon(CommObject* comm);

bool CommSetTransmitterStateWhenIdle(CommObject* com, CommTransmitterIdleState requestedState);

bool CommSetTransmitterBitRate(CommObject* com, CommTransmitterBitrate bitrate);

bool CommGetTransmitterState(CommObject* comm, CommTransmitterState* state);

#ifdef __cplusplus
}
#endif

#endif /* SRC_DEVICES_COMM_H_ */
