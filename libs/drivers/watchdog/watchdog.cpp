#include "watchdog.hpp"
#include <em_cmu.h>
#include <em_wdog.h>
#include "io_map.h"

namespace drivers
{
    namespace watchdog
    {
        void InternalWatchdog::Enable()
        {
            WDOG_Init_TypeDef init = WDOG_INIT_DEFAULT;
            init.debugRun = false;
            init.enable = true;
            init.perSel = io_map::Watchdog::Period;

            WDOGn_Init(WDOG, &init);
        }

        void InternalWatchdog::Disable()
        {
            WDOGn_Enable(WDOG, false);
        }

        void InternalWatchdog::Kick()
        {
            WDOGn_Feed(WDOG);
        }
    }
}
