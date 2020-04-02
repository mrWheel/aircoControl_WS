

//=======================================================================
void saveRawData(String fName, uint16_t inData[], uint16_t len) {
//=======================================================================

  _dThis = true;
  Debugf("saveRawData(%s): aantal pulsen [%ld]\n", fName.c_str(), len);

  File file = SPIFFS.open(fName, "w");
  uint16_t  pulsNr = 0;
    
  for (int16_t r=0; r < len; r++) {
    yield();
    file.print((int)inData[r]);
    file.print(";");
    Debugf("%ld, ", inData[r]);
    if (r%20 == 0) Debugln(" ");
    pulsNr++;
  }

  file.close();  
  Debugf("\nsaveRawData(): saved [%ld] pulses\n", pulsNr);

} // saveRawData()

//=======================================================================
uint16_t readRawData(String fName, uint16_t inData[]) {
//=======================================================================

  uint16_t  pulsNr = 0;

  _dThis = true;
  Debugf("readRawData(%s): \n", fName.c_str());
  
  memset(inData, 0, sizeof(inData));
  File file = SPIFFS.open("/" + fName, "r");
    
  while(file.available()) {
    yield();
    inData[pulsNr] = (uint16_t)file.readStringUntil(';').toInt();
    //Debugf("[%ld]%ld, ", pulsNr,inData[pulsNr]);
    //if (pulsNr%20 == 0) Debugln(" ");
    pulsNr++;
  }
  //Debugln("");
  
  file.close();  
  _dThis = true;
  Debugf("readRawData(): read [%ld] pulses\n", pulsNr);
  
  return pulsNr;

} // readRawData()

//===========================================================================================
int32_t freeSpace() {
//===========================================================================================
  int32_t space;
  
  SPIFFS.info(SPIFFSinfo);

  space = (int32_t)(SPIFFSinfo.totalBytes - SPIFFSinfo.usedBytes);

  return space;
  
} // freeSpace()

//===========================================================================================
void listSPIFFS() {
//===========================================================================================
  Dir dir = SPIFFS.openDir("/");

  TelnetStream.println();
  while (dir.next()) {
    File f = dir.openFile("r");
    TelnetStream.printf("%-15s %ld \r\n", dir.fileName().c_str(), f.size());
    yield();
  }
  TelnetStream.flush();

  SPIFFS.info(SPIFFSinfo);

  int32_t space = (int32_t)(SPIFFSinfo.totalBytes - SPIFFSinfo.usedBytes);
  TelnetStream.println("\r");
  TelnetStream.printf("Available SPIFFS space [%ld]bytes\r\n", freeSpace());
  TelnetStream.printf("           SPIFFS Size [%ld]kB\r\n", (SPIFFSinfo.totalBytes / 1024));
  TelnetStream.printf("     SPIFFS block Size [%ld]bytes\r\n", SPIFFSinfo.blockSize);
  TelnetStream.printf("      SPIFFS page Size [%ld]bytes\r\n", SPIFFSinfo.pageSize);
  TelnetStream.printf(" SPIFFS max.Open Files [%ld]\r\n\n", SPIFFSinfo.maxOpenFiles);


} // listSPIFFS()

//===========================================================================================
String listPulsFiles(String checkedFile) {
//===========================================================================================
  String selHTML = "<select>";
  Dir dir = SPIFFS.openDir("/");

  while (dir.next()) {
    File f = dir.openFile("r");
    if (dir.fileName().indexOf(".pls") > -1) {
      selHTML += "<option value='" + dir.fileName() + "'";
      if (dir.fileName().indexOf(checkedFile) > -1) selHTML += " selected ";
      selHTML += ">" + dir.fileName() + "</option>";
    }
    yield();
  }

  selHTML += "</select>";

  return selHTML;

} // listPulsFiles()


//=======================================================================
void loadHistory() {
  loadHistory(DATASTORE);
}
void loadHistory(String dataFile) {
//=======================================================================
  String    s;
  char      pointType;
  uint32_t  timeStamp = 0;
  
  _dThis = true;
  Debugf("(%s): -> dataStore\n", dataFile.c_str());
  for (int16_t r=0; r < MAXDATA; r++) { // erase dataStore;
    dataStore[r].timestamp    = 0;
    dataStore[r].outsideTemp  = 0.0;
    dataStore[r].insideTemp0  = 0.0;
    dataStore[r].insideTemp1  = 0.0;
  }

  File file = SPIFFS.open(dataFile, "r");
  if (!file) {
    _dThis = true;
    Debugf("No dataStore [%s] Exist\n", dataFile.c_str());
  } else {
    size_t size = file.size();
    if ( size == 0 ) {
      _dThis = true;
      Debugf("dataStore [%s] empty !\n", dataFile.c_str());
    } else {
      dataPointer = 0;
      while (file.available() && dataPointer < MAXDATA) {
        yield();
        s         = file.readStringUntil(';');
        timeStamp = s.toInt();
        s         = file.readStringUntil(';');
        pointType = s[0];
        s         = file.readStringUntil(';');
        outTemp   = s.toFloat();
        s         = file.readStringUntil(';');
        inTemp0   = s.toFloat();
        s         = file.readStringUntil(';');
        inTemp1   = s.toFloat();
        s         = file.readStringUntil('\n');
        if (dataPointer > 0) {
          if (dataStore[(dataPointer - 1)].timestamp == 0 || timeStamp < dataStore[(dataPointer - 1)].timestamp 
                     || inTemp0 < -125 || inTemp0 > 90 
                     || inTemp1 < -125 || inTemp1 > 90) {
            _dThis = true;
            Debugf("found some illigal values @[%d] .. break!", dataPointer);
            break;
          }
        }
        if (externalNtpSync() && (timeStamp < now())) {
          dataStore[dataPointer].timestamp    = timeStamp;
          dataStore[dataPointer].pointType    = pointType;
          if (   (outTemp > -50 && outTemp < 60) 
              && (inTemp0 > -50 && inTemp0 < 60)
              && (inTemp1 > -50 && inTemp1 < 60) ) {
            dataStore[dataPointer].outsideTemp  = outTemp;
            dataStore[dataPointer].insideTemp0  = inTemp0;
            dataStore[dataPointer].insideTemp1  = inTemp1;
            dataPointer++;
          } else {
            _dThis = true;
            Debugf("read invalid data @[%d] .. SKIP!\n", dataPointer);
          }
        } else {
          _dThis = true;
          Debugln("read invalid timeStamp .. SKIP!");
        }
      } // while records ..
      _dThis = true;
      Debugf("(%s): loaded with [%s] points\n", dataFile.c_str(), String(dataPointer).c_str());
    }
    file.close();
  }
  if (!externalNtpSync()) {
    setTime(dataStore[(dataPointer - 1)].timestamp);
    _dThis = true;
    Debugf("last data point time [%04d-%02d-%02d @ %02d:%02d:%02d]\n", year(), month(), day(), hour(), minute(), second());
    DebugFlush();
  }

} // loadHistory()


//=======================================================================
void saveHistory() {
  saveHistory(false);
}
void saveHistory(bool force) {
//=======================================================================
  uint32_t  prevTimestamp = 0;
  uint16_t  actDataPoint  = 0;
  uint16_t  prevDataPoint;
  uint32_t  dt;

  _dThis = true;
  Debugf("[%s] dataPoints ", String((dataPointer - 1)).c_str()); 
  if (SAVEHOUR && !force) {
    if (lastSaveHour == hour()) {
      Debugln(" ==> skip!"); 
      return;
    }
    lastSaveHour = hour();
  }
  Debugln(" ");
  //if (dataPointer > 0) {
  //  prevTimestamp = dataStore[(dataPointer - 1)].timestamp - 10;
  //}
  File file = SPIFFS.open(DATASTORE, "w");
  for (int16_t r=0; (r < dataPointer && r < MAXDATA); r++) {
    if (dataStore[r].timestamp >= prevTimestamp) { // else, something is wrong
      file.print(dataStore[r].timestamp);
      file.print(";");
      file.print(dataStore[r].pointType);
      file.print(";");
      file.print(String(dataStore[r].outsideTemp, 1));
      file.print(";");
      file.print(String(dataStore[r].insideTemp0, 1));
      file.print(";");
      file.print(String(dataStore[r].insideTemp1, 1));
      file.println(";");
      prevDataPoint = r;
      prevTimestamp = dataStore[r].timestamp;
      actDataPoint++;
      Debug(".");
    } else {
      _dThis = true;
      Debugf("timestamp error entry[%d]/[%d] => prev[%d], new[%d]", prevDataPoint, r
                                                                    , prevTimestamp 
                                                                    , dataStore[r].timestamp
           );
    }
  }
  Debugln();
  
  file.close();  
  
  dataPointer = actDataPoint++;
  _dThis = true;
  Debugf("Saved [%d] dataPoints ..\n", dataPointer);
  for (int16_t r=dataPointer; r < MAXDATA; r++) {
    dataStore[r].timestamp    = 0;
    dataStore[r].pointType    = '-';
    dataStore[r].outsideTemp  = 0.0;
    dataStore[r].insideTemp0  = 0.0;
    dataStore[r].insideTemp1  = 0.0;
  }
  
} // saveHistory()
