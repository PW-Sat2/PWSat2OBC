#include <FreeRTOS.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <task.h>

#include "obc.h"
#include "system.h"

using std::uint16_t;
using std::array;
using std::min;

static const char TaskStatuses[] = "RPBSD";

void TaskListCommand(uint16_t argc, char* argv[])
{
    UNUSED(argc, argv);

    array<TaskStatus_t, 20> tasks;

    auto tasksCount = min(static_cast<uint16_t>(uxTaskGetNumberOfTasks()), static_cast<uint16_t>(tasks.size()));

    uxTaskGetSystemState(tasks.data(), tasksCount, nullptr);

    TerminalPuts(&Main.terminal, "Status\tName      \tStack WM\n");

    for (auto i = 0; i < tasksCount; i++)
    {
        auto& t = tasks[i];

        TerminalPrintf(&Main.terminal, "%-6c\t%-10s\t%8d\n", TaskStatuses[t.eCurrentState], t.pcTaskName, t.usStackHighWaterMark);
    }
}
