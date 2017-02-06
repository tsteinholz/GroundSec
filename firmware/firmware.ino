#include <Scheduler.h>
#include <Wire.h>
#include <SymphonyLink.h>

#define D6T_ADDRESS  (0x0A)
#define D6T_GET_INFO (0x4C)

SymphonyLink symlink;

// Symphony Variables
static sym_module_state_t s_currentSymphonyState;
static sym_module_state_t s_lastSymphonyState;
static uint8_t s_txData[256];
static uint8_t s_rxData[128];
static uint8_t s_rxDataLength;

// D6T Sensor Variables
static int     s_dt6_rxBuf[35];
static float   s_dt6_ptat;
static float   s_dt6_tempData[16];

// Application Variables
static bool s_alert;

void setup()
{
    // Debug Prints
    Serial.begin(115200);

    uint32_t netToken = 0x4f50454e;
    uint8_t appToken[APP_TOKEN_LEN] = {0x0};

    // Initialize the SymphonyLink object and open UART communications with the module on Serial1.
    symlink.begin(netToken, appToken, LL_DL_MAILBOX, 15);

    //Set RF path
    symlink.setAntenna(1);

    // Update the state of the SymphonyLink module (aka Modem)
    lastSymphonyState = symlink.updateModemState();

    Scheduler.startLoop(symphony_loop);
    Scheduler.startLoop(sensor_loop);
}

void print_version(void)
{
    ll_version_t firmwareVersion = { 0, 0, 0 };
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

void update_sensor(void)
{
	Wire.beginTransmission(D6T_ADDRESS);
	Wire.write(byte(D6T_GET_INFO));
	Wire.endTransmission();

	delay(1);

	// TODO
	//Wire.requestFrom(adress, count);
	//for(i=0;i<count;i++){
	//in[i] = Wire.read();           // read char 0-18 (2*(8+1)+1=19)
  	//}
  	//Wire.endTransmission();     // stop
}

void loop()
{

}

void symphony_loop()
{
    s_currentSymphonyState = symlink.updateModemState();
    switch (currentSymphonyState)
    {
        case SYMPHONY_READY:
            if (SYMPHONY_TRANSMITTING != lastSymphonyState)
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
}

void sensor_loop()
{
    delay(250);

    delay(250);
}
