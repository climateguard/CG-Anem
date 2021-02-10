#include "cgAnem.h"

ClimateGuard_Anem::ClimateGuard_Anem(uint8_t sensor_address)
{
    _sensor_address = sensor_address;
}

ClimateGuard_Anem::~ClimateGuard_Anem()
{
}

/*Initialization function and sensor connection. Returns false if the sensor is not connected to the I2C bus.*/
bool ClimateGuard_Anem::init()
{
    Wire.begin();
    Wire.beginTransmission(_sensor_address); //safety check, make sure the sensor is connected
    Wire.write(i2c_reg_WHO_I_AM);
    if (Wire.endTransmission(true) != 0)
        return false;
    return true;
}

/*get new necessary data*/
bool ClimateGuard_Anem::data_update()
{
    bool status = getSensorStatus();
    if (overVcc || !status)
        return false;
    temperature = getTemperuture();
    airflowRate = getAirflowRate();
    airConsumption = calculateAirConsumption();
    return true;
}

/*read 1 byte from register*/
bool ClimateGuard_Anem::register_read_byte(uint8_t regAddr, uint8_t *retrieveData)
{
    Wire.beginTransmission(_sensor_address);
    Wire.write(regAddr);
    Wire.endTransmission(false);

    Wire.requestFrom(_sensor_address, 1);
    if (!Wire.readBytes(retrieveData, 1))
    {
        return false;
    }
    return true;
}

/*Get chip id, default value: 0x11.*/
uint8_t ClimateGuard_Anem::getChipId()
{
    register_read_byte(uint8_t(i2c_reg_WHO_I_AM), &_chip_id);
    return _chip_id;
}

/*Get firmware version.*/
uint8_t ClimateGuard_Anem::getFirmwareVersion()
{
    register_read_byte(uint8_t(i2c_reg_VERSION), &_firmware_ver);
    return _firmware_ver;
}

/*get current temperature*/
float ClimateGuard_Anem::getTemperuture()
{
    uint8_t raw[2];
    if (register_read_byte((uint8_t)i2c_reg_TEMP_COLD_H, &raw[0]))
    {
        if (register_read_byte((uint8_t)i2c_reg_TEMP_COLD_L, &raw[1]))
        {
            int16_t tmp = (raw[0] << 8) | raw[1];
            return tmp / 10.0;
        }
    }
    return -255;
}

/*get current flow rate*/
float ClimateGuard_Anem::getAirflowRate()
{
    uint8_t raw[2];
    if (register_read_byte((uint8_t)i2c_reg_WIND_H, &raw[0]))
    {
        if (register_read_byte((uint8_t)i2c_reg_WIND_L, &raw[1]))
        {
            return ((raw[0] << 8) | raw[1]) / 10.0;
        }
    }
    return -255;
}

/*set duct area for rate consumption calculation, if not setted, the airFlowConsumption  variable will be -255*/
void ClimateGuard_Anem::set_duct_area(float area)
{
    ductArea = area;
}

/*calculate flow consumption*/
float ClimateGuard_Anem::calculateAirConsumption()
{
    if (ductArea > -0.01 && airflowRate != -255)
    {
        return 6 * airflowRate * ductArea * 0.06;
    }
    return -255;
}

/*get data from status register*/
bool ClimateGuard_Anem::getSensorStatus()
{
    uint8_t statusReg;
    if (register_read_byte((uint8_t)i2c_reg_STATUS, &statusReg))
    {
        unsteadyProcess = statusReg & (1 << STUP);
        overVcc = statusReg & (1 << STOV);
        taringError = (statusReg & (1 << STITR)) | (statusReg & (1 << STIT));
        return true;
    }
    unsteadyProcess = false;
    overVcc = false;
    taringError = false;
    return false;
}