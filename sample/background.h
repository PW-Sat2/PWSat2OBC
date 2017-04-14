/*
 * background.h
 *
 *  Created on: 11 Jul 2013
 *      Author: pjbotma
 */

#ifndef BACKGROUND_H_
#define BACKGROUND_H_

#include "includes.h"

uint32_t BACKGROUND_displayResetCause(void);
uint32_t BACKGROUND_getResetCause(void);
void BACKGROUND_displayWelcomeMessage(void);

#endif /* BACKGROUND_H_ */
