/*
 * semihosting.h
 *
 *  Created on: 16 gru 2015
 *      Author: Novakov
 */

#ifndef SRC_SEMIHOSTING_H_
#define SRC_SEMIHOSTING_H_

#if SEMIHOSTING
void initialize_semihosting();
#else

void initialize_semihosting() {}
#endif



#endif /* SRC_SEMIHOSTING_H_ */
