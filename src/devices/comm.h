#ifndef SRC_DEVICES_COMM_H_
#define SRC_DEVICES_COMM_H_

#include <stdint.h>

void CommInit(void);
void CommSendFrame(uint8_t * data, uint8_t length);

#endif /* SRC_DEVICES_COMM_H_ */
