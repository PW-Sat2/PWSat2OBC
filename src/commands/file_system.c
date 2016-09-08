#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include "obc.h"
#include "system.h"
#include "terminal.h"

void FSListFiles(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);

    const FSDirectoryOpenResult result = Main.fs.openDirectory(&Main.fs, argv[0]);

    if (OS_RESULT_FAILED(result.Status))
    {
        TerminalPuts("Error");
        TerminalSendNewLine();
        return;
    }

    char* entry;
    FSDirectoryHandle dir = result.Handle;
    while ((entry = Main.fs.readDirectory(&Main.fs, dir)) != NULL)
    {
        TerminalPuts(entry);
        TerminalSendNewLine();
    }

    Main.fs.closeDirectory(&Main.fs, dir);
}

void FSWriteFile(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);

    const FSFileOpenResult result = Main.fs.open(&Main.fs, argv[0], FsOpenCreateAlways, FsWriteOnly);
    if (OS_RESULT_FAILED(result.Status))
    {
        TerminalPuts("Error");
        TerminalSendNewLine();
        return;
    }

    const FSFileHandle file = result.Handle;
    Main.fs.ftruncate(&Main.fs, file, 0);
    Main.fs.write(&Main.fs, file, argv[1], strlen(argv[1]));
    Main.fs.close(&Main.fs, file);
}

void FSReadFile(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    const FSFileOpenResult result = Main.fs.open(&Main.fs, argv[0], FsOpenExisting, FsReadOnly);
    if (OS_RESULT_FAILED(result.Status))
    {
        TerminalPuts("Error");
        TerminalSendNewLine();
        return;
    }

    char buffer[100];
    memset(buffer, 0, sizeof(buffer));
    const FSFileHandle file = result.Handle;
    Main.fs.read(&Main.fs, file, buffer, sizeof(buffer));
    Main.fs.close(&Main.fs, file);

    buffer[99] = 0;

    TerminalPuts(buffer);
    TerminalSendNewLine();
}
