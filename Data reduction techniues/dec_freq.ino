#include "LoRaWan_APP.h"
#include "Arduino.h"

// LoRaWAN Configuration Constants
#define RF_FREQUENCY                                915000000 // Hz US915
#define TX_OUTPUT_POWER                             5        // dBm
#define LORA_BANDWIDTH                              0         // 125 kHz
#define LORA_SPREADING_FACTOR                       7         // SF7
#define LORA_CODINGRATE                             1         // 4/5
#define LORA_PREAMBLE_LENGTH                        8         // 8 symbols
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false

#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 50 // Define the payload size here

// Variables for LoRaWAN message and state
char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];
double batteryVoltage;
bool lora_idle = true;

// Radio events and callback functions
static RadioEvents_t RadioEvents;
void OnTxDone(void);
void OnTxTimeout(void);

// Analog pin for battery voltage measurement
const int batteryPin = A0;

void setup() {
    Serial.begin(115200);
    Mcu.begin();
  
    // Set up LoRaWAN event callbacks
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    
    // Initialize the LoRa radio
    Radio.Init(&RadioEvents);
    Radio.SetChannel(RF_FREQUENCY);
    Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                      LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                      LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                      true, 0, 0, LORA_IQ_INVERSION_ON, 3000);
}

void loop() {
    if (lora_idle == true) {
        delay(1000);
        
        // Read battery voltage from analog pin
        int batteryValue = analogRead(batteryPin);
        batteryVoltage = (batteryValue * 3.3) / 4095; // Assuming 12-bit ADC and 3.3V reference
        
        // Create the LoRaWAN message with only battery voltage
        sprintf(txpacket, "Battery Voltage: %.2f V", batteryVoltage);
        
        Serial.printf("\r\nSending packet \"%s\", length %d\r\n", txpacket, strlen(txpacket));

        // Send the LoRaWAN message
        Radio.Send((uint8_t *)txpacket, strlen(txpacket));
        lora_idle = false;
    }
    // Process LoRa radio interrupts
    Radio.IrqProcess();
}

// Callback function for LoRaWAN transmission complete event
void OnTxDone(void) {
    Serial.println("TX done......");
    lora_idle = true;
}

// Callback function for LoRaWAN transmission timeout event
void OnTxTimeout(void) {
    Radio.Sleep();
    Serial.println("TX Timeout......");
    lora_idle = true;
}
