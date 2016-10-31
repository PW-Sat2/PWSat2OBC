#ifndef UNIT_TESTS_HEAP_H_
#define UNIT_TESTS_HEAP_H_

#include "system.h"

EXTERNC_BEGIN

size_t xPortGetFreeHeapSize(void);
void prvHeapInit(void);
void* pvPortMalloc(size_t xWantedSize);
void vPortFree(void* pv);

EXTERNC_END

#endif /* UNIT_TESTS_HEAP_H_ */
