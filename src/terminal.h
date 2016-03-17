/*
 * terminal.h
 *
 *  Created on: 17 mar 2016
 *      Author: terianil
 */

#ifndef SRC_TERMINAL_H_
#define SRC_TERMINAL_H_

void terminal_send_new_line();
void terminal_handle_command(uint8_t* buffer, uint32_t length);

#endif /* SRC_TERMINAL_H_ */
