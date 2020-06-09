/*
***************************************************************************  
**  Program  : settingsStuff, part of aircoControl
**  Version  : v0.2.0
**
**  Copyright (c) 2019 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/

//=======================================================================
void writeSettings() {
//=======================================================================

  _dThis = true;
  Debugf(" %s .. ", String(_SETTINGS_FILE).c_str());

  File file = SPIFFS.open(_SETTINGS_FILE, "w");

  file.print("CoolThreshold     = ");  file.println(settingCoolThreshold);
  file.print("HeatThreshold     = ");  file.println(settingHeatThreshold);
  file.print("Dempingsfactor    = ");  file.println(settingDemping);
  file.print("UpdateURL         = ");  file.println(settingUpdateURL);
  file.print("UpdateFingerprint = ");  file.println(settingFingerPrint);
  file.print("aircoOff          = ");  file.println(settingPulsOff);
  file.print("aircoCool         = ");  file.println(settingPulsCool);
  file.print("aircoCoolMax      = ");  file.println(settingPulsCoolMax);
  file.print("aircoHeat         = ");  file.println(settingPulsHeat);
  file.print("aircoHeatMax      = ");  file.println(settingPulsHeatMax);
  file.print("WeerLiveKey       = ");  file.println(settingWeerLiveKey);

  file.close();  
  
  Debugln(" .. done");

} // writeSettings()


//=======================================================================
void readSettings() {
//=======================================================================
  String sTmp;
  String words[10];
  int p = 0;

  _dThis = true;
  Debugf(" %s ..", String(_SETTINGS_FILE).c_str());

  if (!SPIFFS.exists(_SETTINGS_FILE)) {
    settingCoolThreshold    = 32;
    settingHeatThreshold    = 5;
    settingDemping          = 5;
    settingUpdateURL   = "https://willem.aandewiel.nl/wp-content/uploads/2019/01/aircoControl.bin";
    settingFingerPrint = "C2:10:43:58:12:74:4A:31:A6:19:E3:32:81:92:30:C3:40:C9:50:D7";
    settingWeerLiveKey = "";
    settingPulsOff     = "_rawOffData.puls";
    settingPulsCoolMax = "_Cool26Cauto.puls";
    settingPulsCool    = "_Cool26C3D.puls";
    settingPulsHeatMax = "_Heat18Cauto.puls";
    settingPulsHeat    = "_Heat18C3D.puls";

    _dThis = true;
    Debugln(" .. done (file not found!)");
    return;
  }

  File file = SPIFFS.open(_SETTINGS_FILE, "r");

  _dThis = true;
  Debugln();
  while(file.available()) {
    sTmp                = file.readStringUntil('\n');
    sTmp.replace("\r", "");
    _dThis = true;
    Debugf("[%s] (%d)\n", sTmp.c_str(), sTmp.length());
    splitString(sTmp, '=', words, 10);
    words[0].toLowerCase();
    if (words[0] == "coolthreshold")      settingCoolThreshold = words[1].toInt();  
    if (words[0] == "heatthreshold")      settingHeatThreshold = words[1].toInt();  
    if (words[0] == "dempingsfactor")     settingDemping       = words[1].toInt();  
    if (words[0] == "updateurl")          settingUpdateURL     = words[1];  
    if (words[0] == "updatefingerprint")  settingFingerPrint   = words[1];  
    if (words[0] == "weerlivekey")        settingWeerLiveKey   = words[1];  
    if (words[0] == "aircooff")           settingPulsOff       = words[1];  
    if (words[0] == "aircocool")          settingPulsCool      = words[1];  
    if (words[0] == "aircocoolmax")       settingPulsCoolMax   = words[1];  
    if (words[0] == "aircoheat")          settingPulsHeat      = words[1];  
    if (words[0] == "aircoheatmax")       settingPulsHeatMax   = words[1];  
    
  } // while available()

  Debugln();
  _dThis = true; Debugf("Cool Threshold   : %d *C\n", settingCoolThreshold);
  _dThis = true; Debugf("Heat Threshold   : %d *C\n", settingHeatThreshold);
  _dThis = true; Debugf("Dempings Factor  : %d\n", settingDemping);
  _dThis = true; Debugf("updateURL        : %s\n", settingUpdateURL.c_str());
  _dThis = true; Debugf("FingerPrint      : %s\n", settingFingerPrint.c_str());
  _dThis = true; Debugf("Weerlive.nl key  : %s\n", settingWeerLiveKey.c_str());
  _dThis = true; Debugf("Puls Off         : %s\n", settingPulsOff.c_str());
  _dThis = true; Debugf("Puls Cool        : %s\n", settingPulsCool.c_str());
  _dThis = true; Debugf("Puls Cool Max    : %s\n", settingPulsCoolMax.c_str());
  _dThis = true; Debugf("Puls Heat        : %s\n", settingPulsHeat.c_str());
  _dThis = true; Debugf("Puls Heat Max    : %s\n", settingPulsHeatMax.c_str());
  
  file.close();  
  _dThis = true;
  Debugln(" .. done");
  DebugFlush();

} // readSettings()

/***************************************************************************
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to permit
* persons to whom the Software is furnished to do so, subject to the
* following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
* OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
* THE USE OR OTHER DEALINGS IN THE SOFTWARE.
* 
***************************************************************************/
