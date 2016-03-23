/*
 * terminal.h
 *
 *  Created on: 15 mar 2016
 *      Author: terianil
 */

#ifndef SRC_LEUART_H_
#define SRC_LEUART_H_

void leuartInit(xQueueHandle sink);
void leuartPuts(const char* buffer);
void leuartPrintf(const char * text, ...);
void leuartPutc(const char c);

#endif /* SRC_LEUART_H_ */
