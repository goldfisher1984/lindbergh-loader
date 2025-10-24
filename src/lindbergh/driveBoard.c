#include "string.h"

#include "driveBoard.h"
#include "jvs.h"
#include "ffb.h"

#define DRIVEBOARD_READY 0x00
#define DRIVEBOARD_NOT_INIT 0x11
#define DRIVEBOARD_BUSY 0x44

int wheelTest = 0;
unsigned char response = DRIVEBOARD_READY;

double steerValue = 0.5;
double force = 0.00;

int initDriveboard()
{
    wheelTest = 0;
    response = DRIVEBOARD_NOT_INIT;

    setAnalogue(ANALOGUE_1, steerValue);

    return 0;
}

ssize_t driveboardRead(int fd, void *buf, size_t count)
{
    memcpy(buf, &response, 1);
    return 1;
}

ssize_t driveboardWrite(int fd, const void *buf, size_t count)
{

    if (count != 4)
    {
        //printf("Error: Drive board count not what expected\n");
        return 1;
    }

    unsigned char *buffer = (unsigned char *)buf;
    printf("FFB send cmd: 0x%02hhX\n", buffer[0]);
    switch (buffer[0])
    {
    case 0xFF:
    {
        //printf("Driveboard: Drive board reset\n");
        response = DRIVEBOARD_READY;
    }
    break;

    case 0x81:
    {
        //printf("Driveboard: Drive board reset 2\n");
        response = DRIVEBOARD_NOT_INIT;
    }
    break;

    case 0xFC:
    {
        //printf("Driveboard: Start wheel bounds testing\n");
        wheelTest = 1;
    }
    break;

    case 0x80:
    {
        ffb_req_toggle(buffer);
    }
    break;

    case 0x9e:
    case 0x84:
    {

        ffb_req_constant_force(buffer);
        response = DRIVEBOARD_READY;
    }
    break;

    case 0xFA:
    case 0xFD:
    {
        //printf("Driveboard: auto turn wheel mode\n");

        if (wheelTest)
        {
            // printf("Increment wheel until 0.9 -> %d\n", (int)(steerValue * 255));
            steerValue += 0.09;
            setAnalogue(ANALOGUE_1, (int)(steerValue * 1024));
            response = DRIVEBOARD_BUSY;
            if (steerValue >= 0.9)
            {
                wheelTest = 0;
                response = DRIVEBOARD_READY;
            }
        }
    }
    break;
    case 0x85: // Rumble Command
    {
        ffb_req_rumble(buffer);
    }
    break;
    case 0x86:    
    {
        ffb_req_damper(buffer);
    }
    break;

    default:
        //printf("Driveboard: Unknown command received %X\n", buffer[0]);
        break;
    }

    return 0;
}
