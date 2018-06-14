//#include "iostream"
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define PARTONE     446
#define PARTTYPE    4
#define PARTLBA     8

const char args[5][3]= { "/?", "-?", "-h", "/m", "/c" };
const char yesno[3][4]= { "No\0", "Yes", "???" };

unsigned int ReadMBR(const char* diskObject, unsigned char* buffer);
void HackString(char* name, unsigned char number);

int main(int argc, char* argv[])
{
    unsigned char firstsector[512];
    char diskname[20] = "\\\\.\\PhysicalDrive0"; // for up to 99 - should be enough

    int disknum, partnum, active, type;
    unsigned int lba, alignment = 64;

    printf("Diskcat alignment check v0.5\n");

    if (argc >= 2)
    {
        if ((strcmp((const char*)argv[1],args[0]) == 0) ||(strcmp((const char*)argv[1],args[1]) == 0) || (strcmp((const char*)argv[1],args[2]) == 0))
            {
                printf("released as Public Domain by whatevar (2018)\n\nCommandline arguments:\n    /m      Alignment to 1 Megabyte (2048)\n    /c 2048 Alignment to custom value (here: 2048 [* 512b])\n");
                return 0;
            }
        else if (strcmp((const char*)argv[1],args[3]) == 0)
            {
                alignment = 2048;
            }
        else if ((strcmp((const char*)argv[1],args[4]) == 0)&&(argc > 2))
            {
                alignment = (unsigned int) strtoul(argv[2],NULL,10);

            }
    }

    if (alignment == 0) alignment = 64;

    printf("Alignment set to: %d\n\n", alignment);

    printf("Object Path\t\tPart#\tActive\tType\tMisalignment\tLBA\n--------------------------------------------------------------------------------\n");

    for (disknum = 0; disknum < 10; disknum++)
    {
        HackString(diskname,disknum);

        if (ReadMBR(diskname,firstsector) == 0) continue;

        //Check the primary partitions

        for (partnum = 0; partnum < 4; partnum++)
        {
            if (firstsector[PARTONE + (partnum*16) + PARTTYPE] == 0) continue;

            active = firstsector[PARTONE + (partnum*16)];

            active = active > 127 ? 1 : active > 0 ? 2 : 0;

            type = firstsector[PARTONE + (partnum*16) + PARTTYPE];
            lba = *((unsigned int*)&firstsector[PARTONE + (partnum*16) + PARTLBA]);
            printf("%s\t%d\t%s\t%d\t%d\t\t%d\n",diskname,partnum,yesno[active],type,(alignment - (lba%alignment))%alignment,lba);
        }

    }
    return 0;
}

void HackString(char* name, unsigned char number)
{
    if (number < 10)
    {
        name[17] = '0' + number;
        name[18] = 0;
        name[19] = 0;
    } else if (number < 100)
    {
        name[17] = '0' + number /10;
        name[18] = '0' + number % 10;
        name[19] = 0;
    } else
    {
        name[0] = 0;
        printf("Not today, Sir!\n");
    }
}

unsigned int ReadMBR(const char* diskObject, unsigned char* buffer)
{
    DWORD count = 0;
    HANDLE disk = INVALID_HANDLE_VALUE;

    disk = CreateFile(diskObject, GENERIC_READ, FILE_SHARE_VALID_FLAGS, 0, OPEN_EXISTING, 0, 0);
    if (disk == INVALID_HANDLE_VALUE)
    {
       if (GetLastError() == 5) printf("%s\t%s\n",diskObject,"Access denied!");
       CloseHandle(disk);
       return 0;
    }

    ReadFile(disk, buffer, 512, &count, 0);  // read sector

    CloseHandle(disk);

    return (unsigned int)count;
}
