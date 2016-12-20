#include <SymphonyLink.h>

SymphonyLink symlink;

uint8_t txData[256];
uint8_t rxData[128];
uint8_t rxDataLength;
uint8_t radioPath = 1;

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
  symlink.setAntenna(radioPath);

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
                txData[0]++;
                symlink.write(txData, sizeof(txData), true);

                Serial.print("\t... Outbound payload is ");
                symlink.printPayload(txData, sizeof(txData));
            }
            else
            {
                if (LL_TX_STATE_SUCCESS != symlink.getTransmitState())
                {
                    txData[0]--;
                }

                symlink.read(rxData, rxDataLength);
            }
            break;
    }

    lastSymphonyState = currentSymphonyState;
}
