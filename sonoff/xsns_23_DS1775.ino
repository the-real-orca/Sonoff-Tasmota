/*
  xsns_23_DS1775.ino - DS1775 temperature sensor support for Sonoff-Tasmota

  Copyright (C) 2018  Stephan Veigl

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "user_config.h"

#ifdef USE_I2C
#ifdef USE_DS1775
/*********************************************************************************************\
 * DS1775 - Temperature sensor / Thermostat
 *
 * I2C Address: 0x48 to 0x4F
\*********************************************************************************************/

#ifndef DS1775_ADDRS
  #define DS1775_ADDRS      { 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F}
#endif

uint8_t ds1775_addressSpace[] = DS1775_ADDRS;
uint8_t ds1775_devices[ sizeof(ds1775_addressSpace) ];
uint8_t ds1775_deviceCount = 0;


float Ds1775ReadTemp(uint8_t address)
{
  if ( Wire.requestFrom( address, 2 ) != 2 )
    return NAN;
  
  // Read the MSB and LSB
  int16_t rawValue;
  rawValue = Wire.read() << 8;
  rawValue |= Wire.read();

  // Convert to the floating point number that it represents
  return ((float)rawValue) / 256.0;
}

/********************************************************************************************/

void Ds1775Init()
{
  // scan for DS1775 devices
  ds1775_deviceCount = 0;
  for (byte i = 0; i < sizeof(ds1775_addressSpace); i++) {
    uint8_t address = ds1775_addressSpace[i];
    Wire.beginTransmission(address);
    uint8_t error = Wire.endTransmission();
    if (0 == error) {
      ds1775_devices[ds1775_deviceCount] = address;
      ds1775_deviceCount++;
      snprintf_P(log_data, sizeof(log_data), S_LOG_I2C_FOUND_AT, "DS1775", address);
      AddLog(LOG_LEVEL_DEBUG);
    }
  }
}

void Ds1775Show(boolean json)
{
  float t;
  char temperature[10];
  char name[16];

  for(byte i = 0; i < ds1775_deviceCount; i++ ) {
    uint8_t address = ds1775_devices[i];
    t = Ds1775ReadTemp(address);
    if ( !isnan(t) ) {

      dtostrfd( ConvertTemp(t), Settings.flag2.temperature_resolution, temperature);
      sprintf(name, "DS1775 (0x%02X)", address);
      if(json) {
        snprintf_P(mqtt_data, sizeof(mqtt_data), PSTR("%s,\"%s\":{\"" D_JSON_TEMPERATURE "\":%s}"), mqtt_data, name, temperature);
#ifdef USE_DOMOTICZ
        if (0 == tele_period) DomoticzSensor(DZ_TEMP, temperature);
#endif  // USE_DOMOTICZ
#ifdef USE_WEBSERVER
      } else {
        snprintf_P(mqtt_data, sizeof(mqtt_data), HTTP_SNS_TEMP, mqtt_data, name, temperature, TempUnit());
#endif  // USE_WEBSERVER
      }
    }
  }
}

/*********************************************************************************************\
 * Interface
\*********************************************************************************************/

#define XSNS_23

boolean Xsns23(byte function)
{
  boolean result = false;

  if (i2c_flg) {
    switch (function) {
      case FUNC_INIT:
        Ds1775Init();
        break;
      case FUNC_JSON_APPEND:
        Ds1775Show(1);
        break;
#ifdef USE_WEBSERVER
      case FUNC_WEB_APPEND:
        Ds1775Show(0);
        break;
#endif  // USE_WEBSERVER
    }
  }
  return result;
}

#endif  // USE_DS1775
#endif  // USE_I2C
