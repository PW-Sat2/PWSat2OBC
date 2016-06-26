#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include "obc.h"
#include "system.h"
#include "terminal.h"

void FSListFiles(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);

    FSDirectoryHandle dir = Main.fs.openDirectory(argv[0]);

    char* entry;
    while ((entry = Main.fs.readDirectory(dir)) != NULL)
    {
        TerminalPuts(entry);
        TerminalSendNewLine();
    }

    Main.fs.closeDirectory(dir);
}

void FSWriteFile(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);

    FSFileHandle file = Main.fs.open(argv[0], O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
    Main.fs.ftruncate(file, 0);
    Main.fs.write(file, argv[1], strlen(argv[1]));
    Main.fs.close(file);
}

void FSReadFile(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    FSFileHandle file = Main.fs.open(argv[0], O_RDONLY, S_IRWXU);

    char buffer[100];

    memset(buffer, 0, sizeof(buffer));

    Main.fs.read(file, buffer, sizeof(buffer));

    Main.fs.close(file);

    buffer[99] = 0;

    TerminalPuts(buffer);
    TerminalSendNewLine();
}
