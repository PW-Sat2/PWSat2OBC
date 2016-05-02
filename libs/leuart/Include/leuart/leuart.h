#ifndef SRC_LEUART_H_
#define SRC_LEUART_H_

void leuartInit(xQueueHandle sink);
void leuartPuts(const char* buffer);
void leuartPrintf(const char * text, ...);
void leuartPutc(const char c);

#endif
