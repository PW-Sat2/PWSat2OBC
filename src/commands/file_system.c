#include <stdint.h>
#include <fcntl.h>
#include "system.h"
#include "terminal.h"
#include "yaffsfs.h"

void FSListFiles(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);

    yaffs_DIR* dir = yaffs_opendir(argv[0]);

    struct yaffs_dirent* entry;
    while ((entry = yaffs_readdir(dir)) != NULL)
    {
        TerminalPuts(entry->d_name);
        TerminalSendNewLine();
    }

    yaffs_closedir(dir);
}

void FSWriteFile(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);

    int file = yaffs_open(argv[0], O_WRONLY | O_CREAT, S_IRWXU);
    yaffs_write(file, argv[1], strlen(argv[1]));
    yaffs_close(file);
}

void FSReadFile(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    int file = yaffs_open(argv[0], O_RDONLY, S_IRWXU);

    char buffer[100];

    memset(buffer, 0, sizeof(buffer));

    yaffs_read(file, buffer, sizeof(buffer));
    yaffs_close(file);

    buffer[99] = 0;

    TerminalPuts(buffer);
    TerminalSendNewLine();
}
