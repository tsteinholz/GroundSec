#include <Scheduler.h>
#include <Wire.h>
#include <SymphonyLink.h>

#define D6T_ID (0x0A)
#define D6T_GET_INFO (0x4C)

SymphonyLink symlink;

// Symphony Variables
static uint8_t s_txData[256];
static uint8_t s_rxData[128];
static uint8_t s_rxDataLength;

// D6T Sensor Variables
static int     s_dt6_rxBuf[35];
static float   s_dt6_ptat;
static float   s_dt6_tempData[16];

sym_module_state_t currentSymphonyState;
sym_module_state_t lastSymphonyState;

void setup()
{
  //Arduino Due allows debug and status signals out of the Serial port. UART communications with the SymphonyLink module are done using Serial1.
  Serial.begin(115200);

  uint32_t netToken = 0x4f50454e;
  uint8_t appToken[APP_TOKEN_LEN] = {0x0};

  //Initialize the SymphonyLink object and open UART communications with the module on Serial1.
  symlink.begin(netToken, appToken, LL_DL_MAILBOX, 15);

  //Set RF path
  symlink.setAntenna(1);

  //Update the state of the SymphonyLink module (aka Modem)
  lastSymphonyState = symlink.updateModemState();
}

void loop()
{
    currentSymphonyState = symlink.updateModemState();
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

    lastSymphonyState = currentSymphonyState;
}
