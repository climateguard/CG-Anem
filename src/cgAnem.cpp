#include "cgAnem.h"

CG_Anem::CG_Anem(uint8_t sensor_address)
{
    _sensor_address = sensor_address;
}

CG_Anem::~CG_Anem()
{
}

/*Initialization function and sensor connection. Returns false if the sensor is not connected to the I2C bus.*/
bool CG_Anem::init()
{
    Wire.begin();
    Wire.beginTransmission(_sensor_address); // safety check, make sure the sensor is connected
    Wire.write(i2c_reg_WHO_I_AM);
    if (Wire.endTransmission(true) != 0)
        return false;
    getFirmwareVersion();
    return true;
}

/*get new necessary data*/
bool CG_Anem::data_update()
{
    temperature = getTemperature();
    airflowRate = getAirflowRate();
    airConsumption = calculateAirConsumption();
    return getSensorStatus() ? false : true; // check data relevance
}

/*read 1 byte from register*/
bool CG_Anem::register_read_byte(uint8_t regAddr, uint8_t *retrieveData)
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

/*write 1 byte to register*/
bool CG_Anem::register_write_byte(uint8_t regAddr, uint8_t regData)
{
    Wire.beginTransmission(_sensor_address);
    Wire.write(regAddr);
    Wire.write(regData);
    if (Wire.endTransmission())
    {
        return false;
    }
    return true;
}
/*Get chip id, default value: 0x11.*/
uint8_t CG_Anem::getChipId()
{
    register_read_byte(uint8_t(i2c_reg_WHO_I_AM), &_chip_id);
    return _chip_id;
}

/*Get firmware version.*/
float CG_Anem::getFirmwareVersion()
{
    register_read_byte(uint8_t(i2c_reg_VERSION), &_firmware_ver);
    float ver = _firmware_ver / 10.0;
    return ver;
}

/*get current temperature*/
float CG_Anem::getTemperature()
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
float CG_Anem::getAirflowRate()
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
void CG_Anem::set_duct_area(float area)
{
    ductArea = area;
}

/*calculate flow consumption*/
float CG_Anem::calculateAirConsumption()
{
    if (ductArea > -0.01 && airflowRate != -255)
    {
        return 6 * airflowRate * ductArea * 0.06;
    }
    return -255;
}

/*get data from status register
 *true - unsteady process, measurements not prepared
 *false - transient process finished, measurements are relevant*/
bool CG_Anem::getSensorStatus()
{
    uint8_t statusReg;
    bool stupBit = 0;
    if (register_read_byte((uint8_t)i2c_reg_STATUS, &statusReg))
    {
        stupBit = statusReg & (1 << STUP);
    }
    return stupBit;
}

/*change i2c address of sensor
 *newAddr - i2c address to set
 */
bool CG_Anem::setI2Caddr(uint8_t newAddr)
{
    if (newAddr > 0x04 && newAddr <= 0x75)
    {
        if (register_write_byte(i2c_reg_ADDRESS, newAddr))
            return true;
    }
    return false;
}

/*get the minimum measured value of the airflow rate after power up or last reset function calling
 *Method available since firmware version 1.0
 */
float CG_Anem::getMinAirFlowRate()
{
    if (_firmware_ver >= 10) // method available since version 1.0
    {
        uint8_t raw[2];
        if (register_read_byte((uint8_t)i2c_reg_WIND_MIN_H, &raw[0]))
        {
            if (register_read_byte((uint8_t)i2c_reg_WIND_MIN_L, &raw[1]))
            {
                return ((raw[0] << 8) | raw[1]) / 10.0;
            }
        }
    }
    return -255;
}

/*get the maximum measured value of the airflow rate after power up or last reset function calling
 *Method available since firmware version 1.0
 */
float CG_Anem::getMaxAirFlowRate()
{
    if (_firmware_ver >= 10) // method available since version 1.0
    {
        uint8_t raw[2];
        if (register_read_byte((uint8_t)i2c_reg_WIND_MAX_H, &raw[0]))
        {
            if (register_read_byte((uint8_t)i2c_reg_WIND_MAX_L, &raw[1]))
            {
                return ((raw[0] << 8) | raw[1]) / 10.0;
            }
        }
    }
    return -255;
}

/*reset values of Min and Max air flow rate registers
*Method available since firmware version 1.0 */
bool CG_Anem::resetMinMaxValues()
{
    if (_firmware_ver >= 10) // method available since version 1.0
    {
        if (register_write_byte(i2c_reg_RESET_WIND, (uint8_t)0x1))
            return true;
    }
    return false;
}
