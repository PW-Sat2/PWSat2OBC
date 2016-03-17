/*
 * terminal.h
 *
 *  Created on: 15 mar 2016
 *      Author: terianil
 */

#ifndef SRC_LEUART_H_
#define SRC_LEUART_H_

void leuart_init(void);
void leuart_send_string(uint8_t* buffer);
void leuart_send_char(uint8_t c);

#endif /* SRC_LEUART_H_ */
