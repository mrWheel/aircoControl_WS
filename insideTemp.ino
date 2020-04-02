/*
 * deze functie leest de temperatuur uit de DS18B20 sensor
 * needs:
 * #include <Wire.h> //I2C header file
 * #include <OneWire.h>
 * #include <DallasTemperature.h>
 * and:
 * // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
 *    OneWire oneWire(ONE_WIRE_PIN);
 * // Pass our oneWire reference to Dallas Temperature. 
 *    DallasTemperature DS18B20(&oneWire);
 *
*/
#define TEMPERATURE_PRECISION 12    // 9, 10, 11 or 12 bits
#define TEMPERATURE_ERROR     1.00  // measurements are to high..

float getInsideTemp(uint8_t DS18B20DevNr) {
    
  float DS18B20Temp, tmpTemp, corrTemp, insideTemp;
  char  fTmp[10], cMsg[20];
  
  if (DS18B20.getDeviceCount() > 0) {
  //for (int D=0; D < DS18B20.getDeviceCount(); D++) {
      sprintf(cMsg, "%X", DS18B20addr[DS18B20DevNr]);
      DS18B20.setResolution(DS18B20addr[DS18B20DevNr], TEMPERATURE_PRECISION);
      delay(50);
      DS18B20.requestTemperatures();
      delay(100);
      DS18B20Temp = DS18B20.getTempCByIndex(DS18B20DevNr);
      corrTemp    = DS18B20Temp * TEMPERATURE_ERROR;
      //dtostrf(DS18B20Temp, 6, 2, fTmp);  
      _dThis = true;
      Debugf("getInsideTemp(%d): DS18B20 (%s) \t[%s]\n", DS18B20DevNr, cMsg, String(DS18B20Temp, 1).c_str() );
      hasDS18B20sensor = true;    
  //}  
  } else {
    corrTemp = (inTemp0 + inTemp1) / 2.0;  
    hasDS18B20sensor = false;      
  }
  insideTemp = corrTemp;
  if (insideTemp <= -127 || insideTemp >= 126) {
    insideTemp = (inTemp0 + inTemp1) / 2.0;  // fallback to last known inTemp0
    _dThis = true;
    Debugln("getInsideTemp(): Error reading inside Temperature!");
//} else {
//  Debugf("getInsideTemp(%d): insideTemperature \t[%s]\n", DS18B20DevNr, String(insideTemp, 2).c_str());
  }
  
  return insideTemp;
    
}   // getInsideTemp()


// function to print a device address
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    // zero pad the address if necessary
    //if (deviceAddress[i] < 10) Debug("0");
    _dThis = true;
    Debug(deviceAddress[i]);
    Debug(":");
  }
}
