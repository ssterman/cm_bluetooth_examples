/*********************************************************************
  This is an example for our nRF51822 based Bluefruit LE modules

  Pick one up today in the adafruit shop!

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  MIT license, check LICENSE for more information
  All text above, and the splash screen below must be included in
  any redistribution
*********************************************************************/

/*
    Please note the long strings of data sent mean the *RTS* pin is
    required with UART to slow down data sent to the Bluefruit LE!
*/

#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

#if SOFTWARE_SERIAL_AVAILABLE
#include <SoftwareSerial.h>
#endif

// Create the bluefruit object, either software serial...uncomment these lines
/*
  SoftwareSerial bluefruitSS = SoftwareSerial(BLUEFRUIT_SWUART_TXD_PIN, BLUEFRUIT_SWUART_RXD_PIN);

  Adafruit_BluefruitLE_UART ble(bluefruitSS, BLUEFRUIT_UART_MODE_PIN,
                      BLUEFRUIT_UART_CTS_PIN, BLUEFRUIT_UART_RTS_PIN);
*/

/* ...or hardware serial, which does not need the RTS/CTS pins. Uncomment this line */
// Adafruit_BluefruitLE_UART ble(BLUEFRUIT_HWSERIAL_NAME, BLUEFRUIT_UART_MODE_PIN);

/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

/* ...software SPI, using SCK/MOSI/MISO user-defined SPI pins and then user selected CS/IRQ/RST */
//Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_SCK, BLUEFRUIT_SPI_MISO,
//                             BLUEFRUIT_SPI_MOSI, BLUEFRUIT_SPI_CS,
//                             BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);


// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

/* The service information */

int32_t tempServiceId;
int32_t tempMeasureCharId;
int32_t tempLocationCharId;
/**************************************************************************/
/*!
    @brief  Sets up the HW an the BLE module (this function is called
            automatically on startup)
*/
/**************************************************************************/
void setup(void)
{
  while (!Serial); // required for Flora & Micro
  delay(500);

  boolean success;

  Serial.begin(115200);
  Serial.println(F("Adafruit Bluefruit Heart Rate Monitor (HRM) Example"));
  Serial.println(F("---------------------------------------------------"));

  randomSeed(micros());

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  /* Perform a factory reset to make sure everything is in a known state */
  Serial.println(F("Performing a factory reset: "));
  if (! ble.factoryReset() ) {
    error(F("Couldn't factory reset"));
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  /* Change the device name to make it easier to find */
  Serial.println(F("Setting device name to 'Sarah's Feather': "));

  if (! ble.sendCommandCheckOK(F("AT+GAPDEVNAME=Sarah's Feather")) ) {
    error(F("Could not set device name?"));
  }

  /* Add the Weather Service definition */
  /* Service ID should be 1 */
  Serial.println(F("Adding the Weather definition (UUID = 0x180D): "));
  success = ble.sendCommandWithIntReply( F("AT+GATTADDSERVICE=UUID=0x180D"), &tempServiceId);
  if (! success) {
    error(F("Could not add Weather service"));
  }

  /* Add the Temperature Characteristic */
  /* Chars ID for Measurement should be 1 */
  Serial.println(F("Adding the Temperature characteristic (UUID = 0x2222): "));
  success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2222, PROPERTIES=0x0A, MIN_LEN=5, MAX_LEN=5"), &tempMeasureCharId);
  Serial.print("added at: ");
  Serial.print(tempMeasureCharId);
  if (! success) {
    error(F("Could not add Temperature characteristic"));
  }

  /* Reset the device for the new service setting changes to take effect */
  Serial.print(F("Performing a SW reset (service changes require a reset): "));
  ble.reset();

  Serial.println();

  /* Wait for connection */
  while (! ble.isConnected()) {
    delay(500);
  }
}

/** Send randomized heart rate data continuously **/
void loop(void)
{
  /* Command is sent when \n (\r) or println is called */
  /* AT+GATTCHAR=CharacteristicID,value */

  // Fun fact: this only gets the 1st byte... need to fetch the rest
  ble.println( F("AT+GATTCHAR=1") );
  if (strcmp(ble.buffer, "OK") == 0) {
    // no data
    return;
  }

    //convert the result
    char * buff={ble.buffer};
    char result[5];
    toHex(buff, result);

    //print out the temperature to serial monitor
    //you could also change the brightness of an LED, or do something else
    Serial.print("Temperature, first digit:   ");
    Serial.println(result);
   
   ble.waitForOK();

  /* Delay before next measurement update */
  delay(1000);
}


//https://forum.arduino.cc/index.php?topic=425663.0
void toHex(char *hex, char *result)
{
  char temp[3];
  int index = 0;

  temp[2] = '\0';
  while (hex[index]) {
    strncpy(temp, &hex[index], 2);
    *result = (char)strtol(temp, NULL, 16);
    result++;
    index += 2;
  }
  *result = '\0';
}