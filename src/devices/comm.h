#ifndef SRC_DEVICES_COMM_H_
#define SRC_DEVICES_COMM_H_

#include <stdint.h>
#include <stdbool.h>

#define COMM_MAX_FRAME_CONTENTS_SIZE 235

typedef struct
{
    uint16_t Size;
    uint16_t Doppler;
    uint16_t RSSI;
    uint8_t Contents[COMM_MAX_FRAME_CONTENTS_SIZE];
} Frame;


void CommInit(void);
void CommSendFrame(uint8_t* data, uint8_t length);
uint8_t CommGetFramesCount(void);
void CommReceiveFrame(Frame* frame);
void CommRemoveFrame(void);
void CommAutoHandling(bool enable);

#endif /* SRC_DEVICES_COMM_H_ */
