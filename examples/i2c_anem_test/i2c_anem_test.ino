/*Warning! After the power on the sensor need 10-15 seconds for heating. During that period, module doesn't' update data*/
#include <Arduino.h>
#include <Wire.h>
#include "cgAnem.h"

CG_Anem cgAnem(ANEM_I2C_ADDR);
float firmWareVer;
void setup()
{
  Serial.begin(115200);
  delay(1000);
  if (cgAnem.init()) // try to init the sensor module
    Serial.println("Sensor secsessfully found");
  else
    Serial.println("Sensor wiring error");

  uint8_t sensorChipId = cgAnem.getChipId(); /*Returns chip id, default value: 0x11.*/

  Serial.print("Chip id: 0x");
  Serial.println(sensorChipId, HEX);
  firmWareVer = cgAnem.getFirmwareVersion(); /*Returns firmware version.*/

  Serial.print("Firmware version: ");
  Serial.println(String(firmWareVer));

  Serial.println("-------------------------------------");
  Serial.println("try to set the duct area for flow consumption calculations");
  cgAnem.set_duct_area(100); // set here duct area for flow consumption calculation in sm^2. If duct area not seted cgAnem.airConsumption will be -255 (default value)
  Serial.println("Duct area setted as " + String(cgAnem.ductArea) + " sm^2");
  delay(3000);
}

/*Warning! After the power on the sensor need 10-15 seconds for heating. During that period, module doesn't' update data*/
void loop()
{
  if (cgAnem.data_update())
  {
    Serial.println("Air flow rate: " + String(cgAnem.airflowRate) + " m/s");
    Serial.println("Current temperature: " + String(cgAnem.temperature) + " C");
    Serial.println("Air flow consumption:" + String(cgAnem.airConsumption) + " m^3/hour");
    if (firmWareVer > 0.9)//all the methods below work with anemometers version 1.0 and higher
    { 
      Serial.println("Maximum measured value of the airflow rate after power up :" + String(cgAnem.getMaxAirFlowRate()) + " m/s");
      //cgAnem.resetMinMaxValues(); // to reset max min values uncoment it
    }
  }
  else
    Serial.println("transient process do not finished, measurements are not relevant");
  delay(1000);
}
