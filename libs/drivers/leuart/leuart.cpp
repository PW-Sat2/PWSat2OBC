#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include <task.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <array>

#include <em_cmu.h>
#include <em_gpio.h>
#include <em_leuart.h>
#include "base/os.h"

#include "dmadrv.h"
#include "io_map.h"
#include "leuart.h"
#include "line_io.h"
#include "system.h"

static SemaphoreHandle_t transferFinished;
static unsigned int dmaChannel;

void LEUART0_IRQHandler(void)
{
    uint32_t flags = LEUART_IntGet(LEUART0);
    LEUART_IntClear(LEUART0, flags);

    if (flags & LEUART_IF_SIGF)
    {
        xSemaphoreGiveFromISR(transferFinished, NULL);
    }

    portEND_SWITCHING_ISR(NULL);
}

static void leuartInit(void)
{
    LEUART_Init_TypeDef leuart0Init;
    leuart0Init.enable = leuartEnable;               /* Activate data reception on LEUn_TX pin. */
    leuart0Init.refFreq = 0;                         /* Inherit the clock frequenzy from the LEUART clock source */
    leuart0Init.baudrate = io_map::LEUART::Baudrate; /* Baudrate = 9600 bps */
    leuart0Init.databits = leuartDatabits8;          /* Each LEUART frame containes 8 databits */
    leuart0Init.parity = leuartNoParity;             /* No parity bits in use */
    leuart0Init.stopbits = leuartStopbits2;          /* Setting the number of stop bits in a frame to 2 bitperiods */

    CMU_ClockEnable(cmuClock_CORELE, true);
    CMU_ClockEnable(cmuClock_LEUART0, true);

    LEUART_Reset(LEUART0);
    LEUART_Init(LEUART0, &leuart0Init);

    LEUART0->SIGFRAME = '\n';

    /* Enable LEUART0 interrupt vector */
    NVIC_SetPriority(LEUART0_IRQn, io_map::LEUART::InterruptPriority);

    LEUART0->ROUTE = LEUART_ROUTE_RXPEN | LEUART_ROUTE_TXPEN | io_map::LEUART::Location;

    transferFinished = xSemaphoreCreateBinary();

    DMADRV_AllocateChannel(&dmaChannel, NULL);
}

static void leuartPuts(LineIO* io, const char* buffer)
{
    UNREFERENCED_PARAMETER(io);

    const size_t len = strlen(buffer);

    for (size_t i = 0; i < len; i++)
    {
        LEUART_Tx(LEUART0, buffer[i]);
    }
}

static void leuartvPrintf(LineIO* io, const char* text, va_list args)
{
    char buf[255] = {0};

    vsniprintf(buf, sizeof(buf), text, args);

    leuartPuts(io, buf);
}

static void leuartPrintBuffer(gsl::span<const std::uint8_t> buffer)
{
    for (auto c : buffer)
    {
        LEUART_Tx(LEUART0, c);
    }
}

static size_t leuartReadline(LineIO* io, char* buffer, size_t bufferLength)
{
    UNREFERENCED_PARAMETER(io);

    LEUART_IntEnable(LEUART0, LEUART_IEN_SIGF);

    DMADRV_PeripheralMemory(dmaChannel,
        dmadrvPeripheralSignal_LEUART0_RXDATAV,
        buffer,
        (void*)&LEUART0->RXDATA,
        true,
        bufferLength,
        dmadrvDataSize1,
        NULL,
        NULL);

    NVIC_EnableIRQ(LEUART0_IRQn);

    xSemaphoreTake(transferFinished, portMAX_DELAY);

    DMADRV_StopTransfer(dmaChannel);

    NVIC_DisableIRQ(LEUART0_IRQn);

    LEUART_IntDisable(LEUART0, LEUART_IEN_SIGF);

    int remaining = 0;

    DMADRV_TransferRemainingCount(dmaChannel, &remaining);

    buffer[bufferLength - remaining - 1] = '\0';

    return bufferLength - remaining;
}

static bool BufferFilled(unsigned int channel, unsigned int sequenceNo, void* userParam)
{
    UNUSED(channel, sequenceNo, userParam);

    System::GiveSemaphoreISR(transferFinished);

    return true;
}

static void leuartExchangeBuffers(LineIO* io, gsl::span<const std::uint8_t> outputBuffer, gsl::span<uint8_t> inputBuffer)
{
    UNREFERENCED_PARAMETER(io);

    DMADRV_PeripheralMemory(dmaChannel,
        dmadrvPeripheralSignal_LEUART0_RXDATAV,
        inputBuffer.data(),
        (void*)&LEUART0->RXDATA,
        true,
        inputBuffer.size(),
        dmadrvDataSize1,
        BufferFilled,
        NULL);

    leuartPrintBuffer(outputBuffer);

    System::TakeSemaphore(transferFinished, InfiniteTimeout);

    uint32_t flags = LEUART_IntGet(LEUART0);
    LEUART_IntClear(LEUART0, flags);
}

void LeuartLineIOInit(LineIO* io)
{
    leuartInit();

    io->extra = NULL;
    io->Puts = leuartPuts;
    io->VPrintf = leuartvPrintf;
    io->Readline = leuartReadline;
    io->PrintBuffer = leuartPrintBuffer;
    io->ExchangeBuffers = leuartExchangeBuffers;
}
