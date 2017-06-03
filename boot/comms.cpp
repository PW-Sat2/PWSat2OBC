/*
 * comms.c
 *
 *  Created on: 30 Jul 2013
 *      Author: pjbotma
 */

#include <algorithm>
#include <array>

#include "comms.h"

#include <em_burtc.h>
#include <em_cmu.h>
#include <em_emu.h>
#include <em_rmu.h>
#include "boot.h"
#include "bsp/bsp_boot.h"

#include "commands/commands.hpp"

using CommandHandler = void (*)();

struct Command
{
    constexpr Command(char commandId, const char* helpMesage, CommandHandler handler)
        : CommandId(commandId), HelpMessage(helpMesage), Handler(handler)
    {
    }

    const char CommandId;
    const char* HelpMessage;
    const CommandHandler Handler;
};

static void PrintHelp();

static std::array<Command, 16> Commands = {
    Command{'T', "Test", Test}, //
    Command{'S', "Test SRAM", TestSRAM},
    Command{'E', "Test EEPROM", TestEEPROM},
    Command{'u', "Boot to upper half", BootUpper},
    Command{'B', "Select index and boot", BootToSelected},
    Command{'s', "Set boot index", SetBootIndex},
    Command{'r', "Restart", NVIC_SystemReset},
    Command{'b', "Continue booting", ProceedWithBooting},
    Command{'x', "Upload application", UploadApplication},
    Command{'z', "Upload safe mode", UploadSafeMode},
    Command{'l', "Print boot table", PrintBootTable},
    Command{'?', "Print help", PrintHelp},
    Command{'t', "TMR boot", TMRBoot},
    Command{'R', "Runlevel", SetRunlevel},
    Command{'f', "Test FRAMs", TestFRAM},
    Command{'C', "Current boot settings", ShowBootSettings},
};

#define UPLOADBLOCKSIZE 256

static volatile uint8_t msgId;

volatile uint8_t uartReceived;

void COMMS_Init(void)
{
    BSP_UART_Init(BSP_UART_DEBUG);

    msgId = 0x00;

    uartReceived = 0;
}

void COMMS_processMsg(void)
{
    // Disable UART interrupts
    BSP_UART_DEBUG->IEN &= ~USART_IF_RXDATAV;

    auto x = msgId;

    auto command = std::find_if(Commands.begin(), Commands.end(), [x](Command& cmd) { return cmd.CommandId == x; });

    if (command != Commands.end())
    {
        uartReceived = 1;
        command->Handler();
    }

    if (msgId != 0)
    {
        BSP_UART_txByte(BSP_UART_DEBUG, '#');
    }

    msgId = 0;
    // Enable UART interrupts
    USART_IntClear(BSP_UART_DEBUG, USART_IF_RXDATAV);
    BSP_UART_DEBUG->IEN |= USART_IF_RXDATAV;
}

/**
 * UART interrupt handler
 */
void BSP_UART_DEBUG_IRQHandler(void)
{
    uint8_t temp;

    // disable interrupt
    BSP_UART_DEBUG->IEN &= ~USART_IEN_RXDATAV;

    // only save message if its been processed (i.e. 0x00)
    if (msgId == 0x00)
    {
        // save message id
        msgId = BSP_UART_DEBUG->RXDATA;
    }
    else
    {
        // store in dummy variable to clear flag
        temp = BSP_UART_DEBUG->RXDATA;
    }

    // enable interrupt
    BSP_UART_DEBUG->IEN |= USART_IEN_RXDATAV;
}

void PrintHelp()
{
    for (auto& c : Commands)
    {
        BSP_UART_Printf<60>(BSP_UART_DEBUG, "\n%c - %s", c.CommandId, c.HelpMessage);
    }

    BSP_UART_txByte(BSP_UART_DEBUG, '\n');
}
