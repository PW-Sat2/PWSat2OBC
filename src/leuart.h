/*
 * terminal.h
 *
 *  Created on: 15 mar 2016
 *      Author: terianil
 */

#ifndef SRC_LEUART_H_
#define SRC_LEUART_H_

void leuartInit(void);
void leuartPuts(uint8_t* buffer);
void leuartPutc(uint8_t c);

#endif /* SRC_LEUART_H_ */
