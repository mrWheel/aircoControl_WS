/*
 * helpers for various task's
 */

//=======================================================================
String dateToString(uint32_t dt) {
//=======================================================================
  char ds[30];

  sprintf(ds, "%04d-%02d-%02d %02d:%02d", year(dt), month(dt), day(dt)
                                        , hour(dt), minute(dt));

  return String(ds);                                      
  
} //  dateToString()


//=======================================================================
String uptime() {
//=======================================================================
  char calcUptime[20];

  sprintf(calcUptime, "%d(d) %02d:%02d", int((upTimeSeconds / (60 * 60 * 24)) % 365)
                                       , int((upTimeSeconds / (60 * 60)) % 24)
                                       , int((upTimeSeconds / (60)) % 60));

  return calcUptime;

} // uptime()


//=======================================================================
void splitString(String inStrng, char delimiter, String wOut[], uint8_t maxWords) {
//=======================================================================
  int8_t inxS = 0, inxE = 0, wordCount = 0;

    while(inxE < inStrng.length() && wordCount < maxWords) {
      inxE  = inStrng.indexOf(delimiter, inxS);             //finds location of first ,
      wOut[wordCount] = inStrng.substring(inxS, inxE);  //captures first data String
      wOut[wordCount].trim();
      inxS = inxE;
      inxS++;
      wordCount++;
    }
    if (inxS < inStrng.length()) {
      wOut[wordCount] = inStrng.substring(inxS, inStrng.length());  //captures first data String      
    }
    
} // splitString()


//=======================================================================
void handleAutonomous() {
//=======================================================================

  if (inTemp1 > (settingCoolThreshold * 1.0)) {
    if (aircoState == '0') {
      _dThis = true;
      Debugf("Temperature [%s] *C ==> switch On airco to Cool!\n", String(inTemp1, 1).c_str());
      noPulses = readRawData(settingPulsCool.c_str(), rawData);
      irsend.sendRaw(rawData, noPulses, 38);  // Send a raw data capture at 38kHz.
      delay(250);
      irsend.sendRaw(rawData, noPulses, 38);  // Send a raw data capture at 38kHz.
      _dThis = true;
      Debugln("Sending Cool26C3D captured from IRrecvDumpV2");
      aircoState         = '1';
      aircoSwitchOffTime = millis() + (AIRCOOFFTIME * 1000);
      aircoOnAt          = now();
      nextPlotTime       = millis();  // force to plot
    }
  }

  if (inTemp1 < (settingHeatThreshold * 1.0)) {
    if (aircoState == '0') {
      _dThis = true;
      Debugf("Temperature [%s] *C ==> switch On airco to Heat!\n", String(inTemp1, 1).c_str());
      noPulses = readRawData(settingPulsHeat.c_str(), rawData);
      irsend.sendRaw(rawData, noPulses, 38);  // Send a raw data capture at 38kHz.
      delay(250);
      irsend.sendRaw(rawData, noPulses, 38);  // Send a raw data capture at 38kHz.
      _dThis = true;
      Debugln("Sending Heat18C3D captured from IRrecvDumpV2");
      aircoState         = '2';
      aircoSwitchOffTime = millis() + (AIRCOOFFTIME * 1000);
      aircoOnAt          = now();
      nextPlotTime       = millis();  // force to plot
    }
  }

  if (aircoState != '0') {
    if (aircoSwitchOffTime != 0) {  // airco is on
      if (millis() > aircoSwitchOffTime) {
        if (inTemp1 > (settingCoolThreshold * 1.0)) {
          // temp still above or below treshhold
          _dThis = true;
          Debugf("Temperature [%s] *C ==> keep on cooling!\n", String(inTemp1, 1).c_str());
          noPulses = readRawData(settingPulsCool.c_str(), rawData);
          irsend.sendRaw(rawData, noPulses, 38);  // Send a raw data capture at 38kHz.
          delay(250);
          irsend.sendRaw(rawData, noPulses, 38);  // Send a raw data capture at 38kHz.
          aircoState         = '1';
          aircoSwitchOffTime = millis() + (AIRCOOFFTIME * 1000);

        } else if (inTemp1 < (settingHeatThreshold * 1.0)) {
          _dThis = true;
          Debugf("Temperature [%s] *C ==> keep on heating!\n", String(inTemp1, 1).c_str());
          noPulses = readRawData(settingPulsHeat.c_str(), rawData);
          irsend.sendRaw(rawData, noPulses, 38);  // Send a raw data capture at 38kHz.
          delay(250);
          irsend.sendRaw(rawData, noPulses, 38);  // Send a raw data capture at 38kHz.
          aircoState         = '2';
          aircoSwitchOffTime = millis() + (AIRCOOFFTIME * 1000);
        } else {
          _dThis = true;
          Debugln("handleAutonomous(): switching Off Airco");
          noPulses = readRawData(settingPulsOff.c_str(), rawData);
          irsend.sendRaw(rawData, noPulses, 38);  // Send a raw data capture at 38kHz.
          delay(250);
          irsend.sendRaw(rawData, noPulses, 38);  // Send a raw data capture at 38kHz.
          aircoState         = '0';
          aircoSwitchOffTime = 0;
          aircoOffAt         = now();
          _dThis = true;
          Debugln("loop(): Airo switched Off after timeout");
          nextPlotTime = millis();  // force to plot
        }
      }
    } // aircoSwitchOffTime > 0

  }   // aircoState != '0'

  if (hour() == 21 && minute() == 1 && hour() != last21Hour) {
    last21Hour         = hour();
     _dThis = true;
    Debugln("handleAutonomous(): switching Off Airco @21:00");
    noPulses           = readRawData(settingPulsOff.c_str(), rawData);
    irsend.sendRaw(rawData, noPulses, 38);  // Send a raw data capture at 38kHz.
    delay(250);
    irsend.sendRaw(rawData, noPulses, 38);  // Send a raw data capture at 38kHz.
    aircoState         = '0';
    aircoSwitchOffTime = 0;
    aircoOffAt         = now();
    Debugln("handleAutonomous(): Airo switched Off after 21:00");
  } 
  if (hour() != 21) {
    last21Hour         = hour();
  }
  
}  // handleAutonomous()


//=======================================================================
void handleLed() {
//=======================================================================
  switch(aircoState) {
    case '0':     ledOnIntrvl   = 1000;
                  ledOffIntrvl  = 1000;
                  break;
    case '1':     ledOnIntrvl   = 1000;
                  ledOffIntrvl  = 3000;
                  break;
    case '2':     ledOnIntrvl   = 3000;
                  ledOffIntrvl  = 1000;
                  break;
    default :     ledOnIntrvl   = 500;
                  ledOffIntrvl  = 500;
                  
  } // switch(aircoState)

  if ((ledOnTime > 0) && (millis() > ledOnTime)) {
    ledOffTime  = millis() + ledOffIntrvl;
    ledOnTime   = 0;
    digitalWrite(LEDPIN, HIGH);
    ledOnAt     = now();
    ledOnState  = true;
  }
  if ((ledOffTime > 0) && (millis() > ledOffTime)) {
    ledOnTime   = millis() + ledOnIntrvl;  // every five minutes
    ledOffTime  = 0;
    digitalWrite(LEDPIN, LOW);
    ledOffAt    = now();
    ledOnState  = false;
  }

} // handleLed()


//=======================================================================
void handlePlotPoint() {
//=======================================================================
  if (millis() > nextPlotTime) {
    addPointToHistory();
    sendHistory();
  }

} // handlePlotPoints()


//=======================================================================
void handleSensor() {
//=======================================================================
  
  if (millis() > nextPollTime) {

    if (!hasDS18B20sensor) {
      //randomSeed(minute());
      // ===== verzin wat data =====
      tn0 = inTemp0;
      tn0 = tn0 + ((float)(random(-2,84) / 75.1) * increment0);
      if (tn0 > 21.8)       { increment0 = -1.0; }
      else if (tn0 < -9.0)  { increment0 =  1.0; }
      tn1 = inTemp1;
      tn1 = tn1 + ((float)(random(-3,92) / 70.1) * increment1);
      if (tn1 > 24.0)       { increment1 = -1.0; }
      else if (tn1 < -10.0) { increment1 =  1.0; }
      //tn0 = inTemp0;
      //tn1 = inTemp1;
    } else {
      tn0 = getInsideTemp(0);   
      tn1 = getInsideTemp(1);   
    }
          
    _dThis = true;
    Debugf("loop(): Sensor: T=[%s/%s], newT=[%s/%s] => ", String(inTemp0).c_str(), String(inTemp1).c_str()
                                                        , String(tn0).c_str(), String(tn1).c_str());
    inTemp0 = ((inTemp0 * settingDemping) + tn0) / (settingDemping + 1);
    inTemp1 = ((inTemp1 * settingDemping) + tn1) / (settingDemping + 1);
    Debugf(" smoothT=[%s/%s]\n", String(inTemp0).c_str(), String(inTemp1).c_str());
    ////addPointToHistory();

    if (!hasDS18B20sensor) {
      nextPollTime = millis() + (10 * 1000);  // 10 seconden interval
    } else {
      nextPollTime = millis() + (POLLINTERVAL * 1000);  // 30 seconden interval
    }
    _dThis = true;
    Debugf("loop(): Next poll in [%02d] seconden\n", ((nextPollTime - millis()) / 1000));
  }
  
} // handleSensor()
