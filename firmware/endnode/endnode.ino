//----------------------------------------------------------------------------//
//                   ______________________________________                   //
//         ________|                                      |_______            //
//         \       |       This file is a part of         |      /            //
//          \      |              GroundSec               |     /             //
//          /      |______________________________________|     \             //
//         /__________)                                (_________\            //
//                                                                            //
//                             GroundSec(c) 2016                              //
//                                                                            //
//                        GNU GENERAL PUBLIC LICENSE                          //
//                         Version 3, 29 June 2007                            //
//                        													  //
//     Copyright (C) 2007 Free Software Foundation, Inc. <http://fsf.org/>    //
//       Everyone is permitted to copy and distribute verbatim copies	      //
//         of this license document, but changing it is not allowed.          //
//                                                                            //
//----------------------------------------------------------------------------//

#include <Scheduler.h>
#include <SymphonyLink.h>
#include <Wire.h>

#define D6T_ADDRESS (0x0A)
#define D6T_GET_INFO (0x4C)
#define HUMAN_TEMP (37)

// Symphony Variables
SymphonyLink symlink;
static sym_module_state_t s_currentSymphonyState;
static sym_module_state_t s_lastSymphonyState;
static byte s_txData[256];
static byte s_rxData[128];
static byte s_rxDataLength;

// D6T Sensor Variables
static byte s_d6t_rxBuf[35];
static float s_d6t_ptat;
static uint16_t s_d6t_tempData[16];

void setup()
{
    // Debug Prints
    Serial.begin(115200);

    // Initialize the SymphonyLink object and open UART communications with the
    // module on Serial1.
    symlink.begin(0x4f50454e, {0x0}, LL_DL_MAILBOX, 15);

    // Set RF path
    symlink.setAntenna(1);

    // Update the state of the SymphonyLink module (aka Modem)
    s_lastSymphonyState = symlink.updateModemState();

    // Start the symphony and sensor tasks
    Scheduler.startLoop(symphony_loop);
    Scheduler.startLoop(sensor_loop);
}

// Print the version of the symphony module
void print_module_version(void)
{
    ll_version_t firmwareVersion = {0, 0, 0};
    int32_t ret = symlink.getModuleFirmware(&firmwareVersion);
    Serial.print("Firmware: ");
    Serial.print(firmwareVersion.major);
    Serial.print(".");
    Serial.print(firmwareVersion.minor);
    Serial.print(".");
    Serial.print(firmwareVersion.tag);
    Serial.print(" | ret: ");
    Serial.print(ret);
    Serial.println();
}

// Handle alert
void alert()
{
    Serial.println("Alert! Sending Alert Data!");

    // TODO : Populate the transmit buffer
    memcpy(s_txData, s_d6t_tempData, sizeof(uint16_t) * 16);
}

// CRC Calculation
byte calc_crc(byte data)
{
    for (byte index = 0; index < 8; index++)
    {
        byte temp = data;
        data <<= 1;

        if (temp & 0x80)
        {
            data ^= 0x07;
        }
    }

    return data;
}

// Check if the data from the d6t is valid or not
int D6T_checkPEC(byte buf[35], int pPEC)
{
    byte crc = calc_crc(0x15);

    for (int i = 0; i < pPEC; i++)
    {
        crc = calc_crc(buf[i] ^ crc);
    }

    return (crc == buf[pPEC]);
}

// Get the sensor data from the d6t
bool update_sensor(void)
{
    Wire.beginTransmission(D6T_ADDRESS);
    Wire.write(byte(D6T_GET_INFO));
    Wire.endTransmission();

    delay(1);

    Wire.requestFrom(D6T_ADDRESS, 35);
    for (byte i = 0; i < 35; i++)
    {
        s_d6t_rxBuf[i] = Wire.read();
    }
    Wire.endTransmission();

    s_d6t_ptat = (s_d6t_rxBuf[1] << 8) | s_d6t_rxBuf[0];

    for (byte i = 1; i < 17; i++)
    {
        s_d6t_tempData[i - 1] = ((uint16_t)s_d6t_rxBuf[i * 2 + 1] << 8) | s_d6t_rxBuf[i * 2];
    }

    return D6T_checkPEC(s_d6t_rxBuf, s_d6t_rxBuf[34]);
}

void loop()
{
    // TODO
    symphony_loop();
}

// Symphony Module State Machine
void symphony_loop()
{
    s_currentSymphonyState = symlink.updateModemState();
    switch (s_currentSymphonyState)
    {
    case SYMPHONY_READY:
        if (SYMPHONY_TRANSMITTING != s_lastSymphonyState)
        {
            s_txData[0]++;
            symlink.write(s_txData, sizeof(s_txData), true);

            Serial.print("\t... Outbound payload is ");
            symlink.printPayload(s_txData, sizeof(s_txData));
        }
        else
        {
            if (LL_TX_STATE_SUCCESS != symlink.getTransmitState())
            {
                s_txData[0]--;
            }

            symlink.read(s_rxData, s_rxDataLength);
        }
        break;
    }

    s_lastSymphonyState = s_currentSymphonyState;
    //yield();
    delay(250);
}

// D6T Driver + Alert Detection
void sensor_loop()
{
    if(update_sensor())
    {
        Serial.print("Transfered valid data from the D6T.\n");

        // Detect if a person is present by pasrsing the sensor data
        for (int i = 0; i < 35; i++)
        {
            if (s_d6t_tempData[i] > HUMAN_TEMP)
            {
                alert();
            }
        }
    }
    else
    {
        Serial.print("D6T Data was invalid! CRC check failed!\n");

        // TODO: Report error to symphony? Keep embeded?
    }

    delay(250);
}
