#include <FreeRTOS.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <task.h>

#include "obc_access.hpp"
#include "system.h"
#include "terminal/terminal.h"

using std::uint16_t;
using std::array;
using std::min;

// static const char TaskStatuses[] = "RPBSD";

void TaskListCommand(uint16_t argc, char* argv[])
{
    UNUSED(argc, argv);

    // array<TaskStatus_t, 20> tasks;

    // auto tasksCount = min(static_cast<uint16_t>(uxTaskGetNumberOfTasks()), static_cast<uint16_t>(tasks.size()));

    // uxTaskGetSystemState(tasks.data(), tasksCount, nullptr);

    // GetTerminal().Puts("Status\tName      \tStack WM\tCur. Pri\tBase Pri\n");

    // for (auto i = 0; i < tasksCount; i++)
    // {
    //     auto& t = tasks[i];

    //     GetTerminal().Printf("%-6c\t%-10s\t%8d\t%8ld\t%8ld\n",
    //         TaskStatuses[t.eCurrentState],
    //         t.pcTaskName,
    //         t.usStackHighWaterMark * sizeof(StackType_t),
    //         t.uxCurrentPriority,
    //         t.uxBasePriority);
    // }
}
