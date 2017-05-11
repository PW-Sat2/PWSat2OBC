#include <cstdint>
#include <em_device.h>
#include <em_dma.h>
#include "dmadrv.h"

#include "obc.h"
#include "system.h"

void DMAInfo(std::uint16_t argc, char* argv[])
{
    UNUSED(argc, argv);

    for (auto i = 0; i < DMA_CHAN_COUNT; i++)
    {
        auto enabled = DMA_ChannelEnabled(i);
        bool active = false;
        DMADRV_TransferActive(i, &active);

        int rem = 0;
        DMADRV_TransferRemainingCount(i, &rem);

        Main.terminal.Printf("%d\t%c\t%d\n", i, enabled ? 'E' : 'D', rem);
    }
}
