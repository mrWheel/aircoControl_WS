
//=======================================================================
void showStatus() {
  char cMsg[150];
  
  Debugln("--------System Status-------------------------------------------");
  Debugf("#> airco Cool drempel >=[%d]*C\n", settingCoolThreshold);
  Debugf("#> airco Heat drempel <=[%d]*C\n", settingHeatThreshold);
  Debugf("#> Dempings Factor    <=[%d]*C\n", settingDemping);
  Debugln("----------------------------------------------------------------");
  
  Debugf("#> inside Temperature  [%s]*C\n", String(inTemp1, 2).c_str());
  Debugf("#> outside Temperature [%s]*C\n", String(outTemp, 2).c_str());

  if (aircoState == '1') {
      Debugf("#> Airco is turned On @ %s for Cooling \n", dateToString(aircoOnAt).c_str());
  } else if (aircoState == '2') {
      Debugf("#> Airco is turned On @ %s for Heating \n", dateToString(aircoOnAt).c_str());
  } else {
      Debugf("#> Airco is turned Off @ %s \n", dateToString(aircoOffAt).c_str());
  }
  if (aircoSwitchOffTime > millis()) {
    Debugf("#> Airco will switch Off in %ld minutes ", (((aircoSwitchOffTime - millis()) + 1) / 60000) + 1 );
    if (aircoState == '1') {
      Debugf("and if the temperature drops below %s*C \n", String(settingCoolThreshold - HYSTERESIS, 1).c_str());
    } else if (aircoState == '2') {
      Debugf("and if the temperature rises above %s*C \n", String(settingHeatThreshold + HYSTERESIS, 1).c_str());
    } else {
      Debugln(" ");
    }
  }

  Debugf("#> Number of dataPoints [%ld]\n", dataPointer);
  Debugf("#> Next Poll in %ld seconds\n", ((nextPollTime - millis()) + 1) / 1000);
  Debugf("#> Next Plot in %ld seconds\n", ((nextPlotTime - millis()) + 1) / 1000);
  Debugln("----------------------------------------------------------------");
  Debug("\r\n==================================================================\r");
  Debug(" \r\n               (c)2018 by [Willem Aandewiel");
  Debug("]\r\n         Firmware Version [");  Debug( _FW_VERSION );
  Debug("]\r\n                 Compiled [");  Debug( __DATE__ ); 
                                              Debug( "  " );
                                              Debug( __TIME__ );
  Debug("]\r\n        last Reset Reason [");  Debug( lastResetReason );
  Debug("]\r\n                 FreeHeap [");  Debug( String(ESP.getFreeHeap()).c_str() );

  sprintf(cMsg, "%08X", String( ESP.getChipId(), HEX ).c_str() );

  Debug("]\r\n                  Chip ID [");  Debug( cMsg );
  Debug("]\r\n             Core Version [");  Debug( String( ESP.getCoreVersion() ).c_str() );
  Debug("]\r\n              SDK Version [");  Debug( String( ESP.getSdkVersion() ).c_str() );
  Debug("]\r\n           CPU Freq (MHz) [");  Debug( String( ESP.getCpuFreqMHz() ).c_str() );
  Debug("]\r\n         Sketch Size (kB) [");  Debug( String( ESP.getSketchSize() / 1024.0 ).c_str() );
  Debug("]\r\n   Free Sketch Space (kB) [");  Debug( String( ESP.getFreeSketchSpace() / 1024.0 ).c_str() );

  if ((ESP.getFlashChipId() & 0x000000ff) == 0x85) 
        sprintf(cMsg, "%08X (PUYA)", ESP.getFlashChipId());
  else  sprintf(cMsg, "%08X", ESP.getFlashChipId());
  Debug("]\r\n            Flash Chip ID [");  Debug( cMsg );
  Debug("]\r\n     Flash Chip Size (kB) [");  Debug( String( ESP.getFlashChipSize() / 1024 ).c_str() );
  Debug("]\r\nFlash Chip Real Size (kB) [");  Debug( String( ESP.getFlashChipRealSize() / 1024 ).c_str() );
  Debug("]\r\n    Flash Chip Speed (MHz)[");  Debug( String( ESP.getFlashChipSpeed() / 1000 / 1000 ).c_str() );
  FlashMode_t ideMode = ESP.getFlashChipMode();
  Debug("]\r\n          Flash Chip Mode [");  Debug( flashMode[ideMode] );

  Debug("]\r\n");

  Debug("==================================================================\r\n");
  Debug(" \r\n               Board type [");
#ifdef ARDUINO_ESP8266_NODEMCU
    Debug("ESP8266_NODEMCU");
#endif
#ifdef ARDUINO_ESP8266_GENERIC
    Debug("ESP8266_GENERIC");
#endif
#ifdef ESP8266_ESP01
    Debug("ESP8266_ESP01");
#endif
#ifdef ESP8266_ESP12
    Debug("ESP8266_ESP12");
#endif
  Debug("]\r\n                     SSID [");  Debug( WiFi.SSID() );
  Debug("]\r\n                  PSK key [");  Debug( WiFi.psk() );
  Debug("]\r\n               IP Address [");  Debug( WiFi.localIP().toString() );
  Debug("]\r\n                 Hostname [");  Debug( _HOSTNAME );
//Debug("]\r\n                 OTA name [");  Debug( hostnameMAC );
  Debug("]\r\n   Firmware Update server [");  Debug( settingUpdateURL );
  Debug("]\r\n                     See: [https://www.grc.com/fingerprints.htm");
  Debug("]\r\n Update Server Fingeprint [");  Debug( settingFingerPrint ); 
  
  Debug("]\r\n          Switch Off Puls [");  Debug( settingPulsOff ); 
  Debug("]\r\n         Switch Cool puls [");  Debug( settingPulsCool ); 
  Debug("]\r\n     Switch Cool Max Puls [");  Debug( settingPulsCoolMax ); 
  Debug("]\r\n         Switch Heat Puls [");  Debug( settingPulsHeat ); 
  Debug("]\r\n     Switch Heat Max Puls [");  Debug( settingPulsHeatMax ); 
  
  Debug("]\r\n                   upTime [");  Debug( uptime() );
  Debug("]\r\n");
  Debug("==================================================================\r\n");
  Debug("number of DS18B20 sensors [");  Debug( String(DS18B20.getDeviceCount()) );
  Debug("]\r\n");
  for (int D=0; D < DS18B20.getDeviceCount(); D++) {
    //DS18B20.setResolution(DS18B20addr[D], TEMPERATURE_PRECISION);
    //delay(100);
      DS18B20.requestTemperatures();
      delay(100);
      float DS18B20Temp = DS18B20.getTempCByIndex(D);
      sprintf(cMsg, "%X", DS18B20addr[D]);
      Debugf("       Address Device (%d) [", D);  Debug( cMsg );
      Debug("]\r\n");
      Debugf("   Temperature Device (%d) [", D);  Debug( String(DS18B20Temp, 2) );
      Debug("]\r\n");
  }
  //Debugln("]\r");
  Debugln("==================================================================\r\n");
  TelnetStream.flush();
  
} // showStatus()


//=======================================================================
void updateFirmware() {

    _dThis = true;
    Debugf("URL         [%s] \n", settingUpdateURL.c_str());
    _dThis = true;
    Debugf("Fingerprint [%s]\n\n", settingFingerPrint.c_str());

    _dThis = true;
    Debugln("Sorry, server-update crashes all the time ... Skipping!");
    /****
    //t_httpUpdate_return ret = ESPhttpUpdate.update(settingUpdateURL.c_str());
    t_httpUpdate_return  ret = ESPhttpUpdate.update(settingUpdateURL.c_str(), "", settingFingerPrint.c_str());

    _dThis = true;
    Debugln("Done Updating Firmware ..");

    _dThis = true;
    switch (ret) {
      case HTTP_UPDATE_FAILED:
        Debugf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        break;

      case HTTP_UPDATE_NO_UPDATES:
        Debugln("HTTP_UPDATE_NO_UPDATES");
        break;

      case HTTP_UPDATE_OK:
        Debugln("HTTP_UPDATE_OK");
        break;
    }
    ***/
    Debugln();

} // updateFirmware()


//=======================================================================
void handleKeyInput() {
  
  while (TelnetStream.available() > 0) {
    yield();
    inChar = (char)TelnetStream.read();
    while (TelnetStream.available() > 0) {
       yield();
       (char)TelnetStream.read();
    }

    switch(inChar) {
      case 'b':
      case 'B':     showStatus();
                    break;
      case 'h':
      case 'H':     displayHistory("displayHistory()");
                    break;
      case 'i':
      case 'I':     inTemp1 = getInsideTemp(1);
                    Debugf("Insite temperature in Office is %s*C\n", String(inTemp1, 1).c_str());
                    break;
      case 'L':     loadHistory();
                    break;
      case 'o':
      case 'O':     outsiteTemp = getOutsideTemp(true);
                    Debugf("Outsite temperature in Baarn is %s*C\n", String(outsiteTemp, 1).c_str());
                    break;
      case 'R':     ESP.reset();
                    break;
      case 's':
      case 'S':     listSPIFFS();
                    break;
      case 'T':     Debugln("buildHistory() - commented");
                    buildHistory();
                    break;
      case 'U':     updateFirmware();
                    break;
      case 'W':     saveHistory(true);
                    break;
      default:      TelnetStream.println("\nCommandos are:\n");
                    TelnetStream.println("   B - Board, Build info & System Status");
                    TelnetStream.println("   H - Display History (memory)");
                    TelnetStream.println("   I - Inside Temperature");
                    TelnetStream.println("  *L - Load historie from SPIFFS");
                    TelnetStream.println("   O - Outside Temperature");
                    TelnetStream.println("  *R - Reboot");
                    TelnetStream.println("   S - list SPIFFS");
                    TelnetStream.println("  *T - Build dummy data");
                    TelnetStream.println("  *U - update Firmware");
                    TelnetStream.println("  *W - Write historie to SPIFFS");
                    TelnetStream.println(" ");

    } // switch()
  }
  
} // handleKeyInput()
