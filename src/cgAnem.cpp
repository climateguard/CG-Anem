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
    getFactory();	
//    getFactoryId1();
//    getFactoryId2();
//    getFactoryId3();
//    getFactoryId4();
    return true;
}

/*get new necessary data*/
bool CG_Anem::data_update()
{
    temperature = getAmbientTemperature();
    hottemperature = getHotendTemperature();
    airflowRate = getAirflowRate();
    airConsumption = calculateAirConsumption();
    heatPower = getheatPower();
    voltagesupply = getVoltageSupply();
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

/*Get factory.*/
long CG_Anem::getFactory()
{
        uint8_t raw[4];
        if (register_read_byte((uint8_t)i2c_reg_FACTORY_ID_1, &raw[0]))
        {
            if (register_read_byte((uint8_t)i2c_reg_FACTORY_ID_2, &raw[1]))
            {
		if (register_read_byte((uint8_t)i2c_reg_FACTORY_ID_3, &raw[2]))
                {
	  	    if (register_read_byte((uint8_t)i2c_reg_FACTORY_ID_4, &raw[3]))
                    {
                        return ((raw[0] << 24) | ((raw[1] << 16) | ((raw[2] << 8) | raw[3]);
                    }
		}
	    }
        }
	
//    register_read_byte(uint8_t(i2c_reg_FACTORY), &_factory);
//    return _factory;
}

/*/*Get factory_id_1.*/
uint8_t CG_Anem::getFactoryId1()
{
    register_read_byte(uint8_t(i2c_reg_FACTORY_ID_1), &_factoryid1);
    return _factoryid1;
}

/*Get factory_id_2.*/
uint8_t CG_Anem::getFactoryId2()
{
    register_read_byte(uint8_t(i2c_reg_FACTORY_ID_2), &_factoryid2);
    return _factoryid2;
}

/*Get factory_id_3.*/
uint8_t CG_Anem::getFactoryId3()
{
    register_read_byte(uint8_t(i2c_reg_FACTORY_ID_3), &_factoryid3);
    return _factoryid3;
}

/*Get factory_id_4.*/
uint8_t CG_Anem::getFactoryId4()
{
    register_read_byte(uint8_t(i2c_reg_FACTORY_ID_4), &_factoryid4);
    return _factoryid4;
}
*/
	
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

/*Get getsupplyV.*/
float CG_Anem::getVoltageSupply()
{
    register_read_byte(uint8_t(i2c_reg_SUPPLY_V), &_supply_v);
    float tmp = _supply_v / 10.0;
    return tmp;
}

/*Get getheatPower.*/
float CG_Anem::getheatPower()
{
    register_read_byte(uint8_t(i2c_reg_PWR_WT), &_heatpwr);
 /*   float tmp = _heatpwr / 10.0;  */
	float tmp = (_heatpwr * 1.36125) / 255;
    return tmp;
}

/*get current Ambient Temperature*/
float CG_Anem::getAmbientTemperature()
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

/*get current Hotend Temperature*/
float CG_Anem::getHotendTemperature()
{
    uint8_t raw[2];
    if (register_read_byte((uint8_t)i2c_reg_TEMP_HOT_H, &raw[0]))
    {
        if (register_read_byte((uint8_t)i2c_reg_TEMP_HOT_L, &raw[1]))
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

/*get data from status register stovBit
 *true - overvoltage detected
 *false - supply voltage is normal*/
bool CG_Anem::getSensorOVStatus()
{
    uint8_t statusReg;
    bool stovBit = 0;
    if (register_read_byte((uint8_t)i2c_reg_STATUS, &statusReg))
    {
        stovBit = statusReg & (1 << STOV);
    }
    return stovBit;
}

/*get data from status register stwdtBit
 *true - wdt enable
 *false - wdt disable*/
bool CG_Anem::getSensorWDTStatus()
{
    uint8_t statusReg;
    bool stwdtBit = 0;
    if (register_read_byte((uint8_t)i2c_reg_STATUS, &statusReg))
    {
        stwdtBit = statusReg & (1 << STWDT);
    }
    return stwdtBit;
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

/*Enable WatchDog Timer*/
bool CG_Anem::enableWDT()
{
    {
        if (register_write_byte(i2c_reg_STATUS, (uint8_t)0x10))
            return true;
    }
    return false;
}

/*Disable WatchDog Timer*/
bool CG_Anem::disableWDT()
{
    {
        if (register_write_byte(i2c_reg_STATUS, (uint8_t)0x0))
            return true;
    }
    return false;
}
