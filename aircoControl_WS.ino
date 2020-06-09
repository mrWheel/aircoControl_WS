/*
 * aircoControl_WS 
*/
#define _FW_VERSION "v0.4.0 WS (09-06-2020)"
/* 

    Arduino-IDE settings for ESP-12E:

    - Board: "NodeMCU 1.0 (ESP-12E Module)"
    - Flash mode: "DIO" / "DOUT"
    - Flash size: "4M (1M SPIFFS)"
    - CPU Frequency: "80 MHz"
    - Debug port: "Disabled"
    - Debug Level: "None"
    - IwIP Variant: "v2 Lower Memory"
    - VTables: "Flash"
    - Reset Method: "nodemcu"
    - CPU Frequency: "80 MHz"
    - Buildin Led: "1"
    - Upload Speed: "115200"
    - Erase Flash: "Only Sketch"
    - Port: "aircoControl at <-- IP address -->"

    Arduino ESP8266 core v2.7.1
*/

#include <IRremoteESP8266.h>    // v2.5.2
#include <IRsend.h>
#include "rawDataIR.h"  // raw date from IRrecvDumpV2.ino
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ezTime.h>   // https://github.com/ropg/ezTime

#include "Debug.h"
#define _HOSTNAME     "aircoControl"
#include "networkStuff.h"

#define ONE_WIRE_PIN  2                 // GPIO02 - GPIO pin waar DS18B20 op aangesloten is
#define IR_LED        4                 // ESP8266 GPIO pin to use. Recommended: 4 (D2).
#define LEDPIN        5                 // what digital pin we're connected to
#define DATASTORE     "/dataStore.dat"
#define MAXDATA       250               // 144 is one day
#define POLLINTERVAL  20                // in seconds
//#define PLOTINTERVAL  600               // in seconds (= 10 minuten)
#define PLOTINTERVAL  300               // in seconds (= 5 minuten)
#define SAVEHOUR      true              // when "true" save on the hour
#define AIRCOOFFTIME  600               // Time for auto switch-off
#define HYSTERESIS    2.0
#define _SETTINGS_FILE  "/settings.ini"

struct dataType {
  uint32_t  timestamp;
  char      pointType;
  float     outsideTemp;
  float     insideTemp0;
  float     insideTemp1;
} dataStore[MAXDATA + 3];

static char *flashMode[]    { "QIO", "QOUT", "DIO", "DOUT", "UnKnown" };
enum    { TAB_CONTROL, TAB_GRAFIEK, TAB_HISTORIE, TAB_UNKNOWN };

uint16_t  dataPointer;
char     *dummyspace = {"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"};

float     tn0, tn1, outsiteTemp, outTemp, inTemp0 = 0.0, inTemp1 = 0.0;
float     increment0 = 0.1, increment1 = 0.14;
bool      hasDS18B20sensor, ledOnState; //--, isConnected;
uint32_t  nextSecond, nextStateSend, nextPollTime, nextPlotTime;
uint32_t  ledOnTime, ledOffTime, aircoSwitchOffTime;
uint32_t  ledOnAt, ledOffAt, aircoOnAt, aircoOffAt; // timestamps
uint64_t  upTimeSeconds;
uint8_t   last21Hour, lastSaveHour; // if this is not the same as "hour()" -> save data
String    hostnameMAC, jsonString, lastResetReason;
char      inChar;       // Console input
uint16_t  noPulses;
char      aircoState   = '0';
char      switch1State = '0';
char      switch2State = '1';
char      wsSend[100];          // String to send via WebSocket
uint32_t  ledOnIntrvl, ledOffIntrvl;
bool      newTab  = true;
int8_t    clientActiveTab;
float     clientInTemp0, clientInTemp1, clientOutTemp;
int16_t   clientCoolTemp, clientHeatTemp, clientDemping;
uint32_t  clientPollTime, clientPlotTime;
String    clientUpdateURL, clientFingerPrint, clientWeerLiveKey;
String    clientPulsOff, clientPulsCool, clientPulsCoolMax, clientPulsHeat, clientPulsHeatMax;
uint16_t  clientDataPointer;
char      clientState = '-';
int16_t   settingCoolThreshold;
int16_t   settingHeatThreshold;
String    settingPulsOff     = "_rawOffData.puls";
String    settingPulsCoolMax = "_Cool26Cauto.puls";
String    settingPulsCool    = "_Cool26C3D.puls";
String    settingPulsHeatMax = "_Heat18Cauto.puls";
String    settingPulsHeat    = "_Heat18C3D.puls";
int8_t    settingDemping     = 5;
String    settingUpdateURL   = "https://willem.aandewiel.nl/wp-content/uploads/2019/01/aircoControl.bin";
          // find out fingerprint: https://www.grc.com/fingerprints.htm
String    settingFingerPrint = "C2:10:43:58:12:74:4A:31:A6:19:E3:32:81:92:30:C3:40:C9:50:D7";
String    settingWeerLiveKey = "";

Timezone  CET;

const int         timeZone = 1;       // Central European (Winter) Time
unsigned int      localPort = 8888;   // local port to listen for UDP packets

time_t            prevDisplay = 0; // when the digital clock was displayed
                  // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire           oneWire(ONE_WIRE_PIN);
                  // Pass our oneWire reference to Dallas Temperature. 
DallasTemperature DS18B20(&oneWire);
// arrays to hold device address
DeviceAddress     DS18B20_0, DS18B20_1, DS18B20_2;
DeviceAddress     DS18B20addr[8];
IRsend            irsend(IR_LED);  // Set the GPIO to be used to sending the message.


/***************************************************************************/
//===============PROTOTYPES==============================================
float   getInsideTemp(uint8_t); 
float   getOutsideTemp(bool);  
time_t  getNtpTime();           
void    loadHistory(String);
void    saveHistory(bool);
//=======================================================================

//=======================================================================
void updateAirco(int aircoStatus) {
  //String Airco = HttpServer.arg("id");
  //String aircoStatus = HttpServer.arg("aircoStatus");
  //String success = "1";

  _dThis = true;
  Debugf("updateAirco(): ==> aircoStatus[%d]\n", aircoStatus);
  _dThis = true;
  if ( aircoStatus == 1 ) {
    Debugln("updateAirco(): updateAirco(): Airco switched to Cool");
    Debugln("updateAirco(): Sending Cool26Cauto captured from IRrecvDumpV2");
    noPulses = readRawData(settingPulsCoolMax.c_str(), rawData);
    irsend.sendRaw(rawData, noPulses, 38);  // Send a raw data capture at 38kHz.
    delay(250);
    irsend.sendRaw(rawData, noPulses, 38);  // Send a raw data capture at 38kHz.
    aircoState         = '1';
    aircoSwitchOffTime = 0;
    aircoOnAt          = now();
  } else if ( aircoStatus == 2 ) {
    Debugln("updateAirco(): updateAirco(): Airco switched to Heat");
    Debugln("updateAirco(): Sending Heat18Cauto captured from IRrecvDumpV2");
    noPulses = readRawData(settingPulsHeatMax.c_str(), rawData);
    irsend.sendRaw(rawData, noPulses, 38);  // Send a raw data capture at 38kHz.
    delay(250);
    irsend.sendRaw(rawData, noPulses, 38);  // Send a raw data capture at 38kHz.
    aircoState         = '2';
    aircoSwitchOffTime = 0;
    aircoOnAt          = now();
  } else {
    Debugln("updateAirco(): updateAirco(): Airco switched OFF");
    Debugln("updateAirco(): Sending rawOffData captured from IRrecvDumpV2");
    noPulses = readRawData(settingPulsOff.c_str(), rawData);
    irsend.sendRaw(rawData, noPulses, 38);  // Send a raw data capture at 38kHz.
    delay(250);
    irsend.sendRaw(rawData, noPulses, 38);  // Send a raw data capture at 38kHz.
    aircoState         = '0';
    aircoSwitchOffTime = 0;
    aircoOffAt         = now();
  }

  sprintf(wsSend, "aircoStatus, %c", aircoState);
  _dThis = true;
  Debugf("websocket.send(%s)\n", wsSend);
  webSocket.broadcastTXT(wsSend);
    
} // updateAirco()


//=======================================================================
void apiAircoState() {
  uint32_t dt = now();
  char     dateTime[20];
  String   statusLong;

  _dThis = true;
  Debugf("HttpServer.uri() [%s]\n", HttpServer.uri().c_str());

  if (aircoState == '1') {
      statusLong ="Airco is turned On @ ";
      statusLong += dateToString(aircoOnAt);
      statusLong += " for Cooling";
  } else if (aircoState == '2') {
      statusLong  = "Airco is turned On @ ";
      statusLong += dateToString(aircoOnAt);
      statusLong += " for Heating ";
  } else {
      statusLong  = "Airco is turned Off @ ";
      statusLong += dateToString(aircoOffAt);
  }
  statusLong += "<br>";
  if (aircoSwitchOffTime > millis()) {
    statusLong += "Airco will switch Off in ";
    statusLong += String(((aircoSwitchOffTime - millis()) + 1) / 60000);
    statusLong += " minutes ";
    if (aircoState == '1') {
      statusLong += "and the temperature drops below ";
      statusLong += String(settingCoolThreshold - HYSTERESIS, 1);
      statusLong += "&deg;C";
    } else if (aircoState == '2') {
      statusLong += "and the temperature rises above ";
      statusLong += String(settingHeatThreshold + HYSTERESIS, 1);
      statusLong += "&deg;C";
    }
  }

  sprintf(dateTime, "%02d-%02d-%04d %02d:%02d", day(dt), month(dt), year(dt), hour(dt),minute(dt));

  statusLong.replace("<br>", "");

  if (aircoState == '1') {   
    jsonString  = "{\"aircoStatus\":\"1\",";
  } else if (aircoState == '2') {   
    jsonString  = "{\"aircoStatus\":\"2\",";
  } else {
    jsonString  = "{\"aircoStatus\":\"0\",";
  }   
  jsonString += "\"aircoInTemp0\":\""+String(inTemp0)+"\",";
  jsonString += "\"aircoInTemp1\":\""+String(inTemp1)+"\",";
  jsonString += "\"aircoOutTemp\":\""+String(outTemp)+"\",";
  jsonString += "\"aircoTime\":\""+String(dateTime)+"\",";
  jsonString += "\"upTime\":\""+uptime()+"\",";
  jsonString += "\"aircoCoolTemp\":\"" + String(settingCoolThreshold) + "\",";
  jsonString += "\"aircoHeatTemp\":\"" + String(settingHeatThreshold) + "\",";
  jsonString += "\"tempScale\":\" &deg;C\",";
  jsonString += "\"aircoDataPoints\":\"" + String(dataPointer) + "\",";
  jsonString += "\"aircoNextPoll\":\"" + String(((nextPollTime - millis()) + 1) / 1000) + "\",";
  jsonString += "\"aircoNextPlot\":\"" + String(((nextPlotTime - millis()) + 1) / 1000) + "\",";
  jsonString += "\"aircoStatusLong\":\"" + statusLong + "\",";
  jsonString += "\"success\":\"1\"}";

  _dThis = true;
  Debugln("apiAircoState(): Send json to requesting party ..");
  HttpServer.send(200, "text/json", jsonString); // Send stateType value only to client ajax request
  dt++;
  
} // apiAircoState()


//=======================================================================
void getAircoState() {
  uint32_t dt = now();
  char     dateTime[20];
  String   statusLong;
  static bool   firstTime;
  static int8_t savMinute;

  if (newTab) {
    newTab    = false;
    firstTime = true;
  }
  if (savMinute != minute(dt)) {
    savMinute = minute(dt);
    firstTime = true;
  }

  sprintf(dateTime, "%02d-%02d-%04d %02d:%02d", day(dt), month(dt), year(dt), hour(dt), minute(dt));

  if (aircoState != clientState) 
  {
    clientState = aircoState;
    sprintf(wsSend, "aircoStatus, %c", clientState);
    _dThis = true;
    Debugf("websocket.send(%s)\n", wsSend);
    webSocket.broadcastTXT(wsSend);
  }   

  if (inTemp1 != clientInTemp1) 
  {
    clientInTemp1 = inTemp1;
    sprintf(wsSend, "inTemp1, %.1f", clientInTemp1);
    _dThis = true;
    Debugf("websocket.send(%s)\n", wsSend);
    webSocket.broadcastTXT(wsSend);
  }   

  if (outTemp != clientOutTemp) 
  {
    clientOutTemp = outTemp;
    sprintf(wsSend, "outTemp, %.1f", clientOutTemp);
    _dThis = true;
    Debugf("websocket.send(%s)\n", wsSend);
    webSocket.broadcastTXT(wsSend);
  }   
  if (settingCoolThreshold != clientCoolTemp) 
  {
    clientCoolTemp = settingCoolThreshold;
    sprintf(wsSend, "aircoCoolTemp, %d", clientCoolTemp);
    _dThis = true;
    Debugf("websocket.send(%s)\n", wsSend);
    webSocket.broadcastTXT(wsSend);
  }
  if (settingHeatThreshold != clientHeatTemp) 
  {
    clientHeatTemp = settingHeatThreshold;
    sprintf(wsSend, "aircoHeatTemp, %d", clientHeatTemp);
    _dThis = true;
    Debugf("websocket.send(%s)\n", wsSend);
    webSocket.broadcastTXT(wsSend);
  }
  if (settingDemping != clientDemping) 
  {
    clientDemping = settingDemping;
    sprintf(wsSend, "aircoDemping, %d", clientDemping);
    _dThis = true;
    Debugf("websocket.send(%s)\n", wsSend);
    webSocket.broadcastTXT(wsSend);
  }
  if (dataPointer != clientDataPointer) 
  {
    clientDataPointer = dataPointer;
    sprintf(wsSend, "aircoDataPoints, %d", clientDataPointer);
    _dThis = true;
    Debugf("websocket.send(%s)\n", wsSend);
    webSocket.broadcastTXT(wsSend);
  }
  if (nextPlotTime != clientPlotTime) 
  {
    clientPlotTime = nextPlotTime;
    sprintf(wsSend, "aircoNextPlot, %s", String(((clientPlotTime - millis()) + 1) / 1000).c_str());
    _dThis = true;
    Debugf("websocket.send(%s)\n", wsSend);
    webSocket.broadcastTXT(wsSend);
  }
  if (settingUpdateURL != clientUpdateURL) 
  {
    clientUpdateURL = settingUpdateURL;
    sprintf(wsSend, "setUpdateURL, %s", clientUpdateURL.c_str());
    _dThis = true;
    Debugf("websocket.send(%s)\n", wsSend);
    webSocket.broadcastTXT(wsSend);
  }
  if (settingFingerPrint != clientFingerPrint) 
  {
    clientFingerPrint = settingFingerPrint;
    sprintf(wsSend, "setFingerPrint, %s", clientFingerPrint.c_str());
    _dThis = true;
    Debugf("websocket.send(%s)\n", wsSend);
    webSocket.broadcastTXT(wsSend);
  }
  if (settingWeerLiveKey != clientWeerLiveKey) 
  {
    clientWeerLiveKey = settingWeerLiveKey;
    sprintf(wsSend, "setWeerLiveKey, %s", clientWeerLiveKey.c_str());
    _dThis = true;
    Debugf("websocket.send(%s)\n", wsSend);
    webSocket.broadcastTXT(wsSend);
  }
  if (settingPulsOff != clientPulsOff) 
  {
    clientPulsOff = settingPulsOff;
    sprintf(wsSend, "setPulsOff, %s", clientPulsOff.c_str());
    _dThis = true;
    Debugf("websocket.send(%s)\n", wsSend);
    webSocket.broadcastTXT(wsSend);
  }
  if (settingPulsCool != clientPulsCool) 
  {
    clientPulsCool = settingPulsCool;
    sprintf(wsSend, "setPulsCool, %s", clientPulsCool.c_str());
    _dThis = true;
    Debugf("websocket.send(%s)\n", wsSend);
    webSocket.broadcastTXT(wsSend);
  }
  if (settingPulsCoolMax != clientPulsCoolMax) 
  {
    clientPulsCoolMax = settingPulsCoolMax;
    sprintf(wsSend, "setPulsCoolMax, %s", clientPulsCoolMax.c_str());
    _dThis = true;
    Debugf("websocket.send(%s)\n", wsSend);
    webSocket.broadcastTXT(wsSend);
/*
    String selHtml = listPulsFiles(clientPulsCoolMax);
    sprintf(wsSend, "selectPulsCoolMax, %s", selHtml.c_str());
    _dThis = true;
    Debugf("websocket.send(%s)\n", wsSend);
    webSocket.broadcastTXT(wsSend);
*/
  }
  if (settingPulsHeat != clientPulsHeat) 
  {
    clientPulsHeat = settingPulsHeat;
    sprintf(wsSend, "setPulsHeat, %s", clientPulsHeat.c_str());
    _dThis = true;
    Debugf("websocket.send(%s)\n", wsSend);
    webSocket.broadcastTXT(wsSend);
  }
  if (settingPulsHeatMax != clientPulsHeatMax) 
  {
    clientPulsHeatMax = settingPulsHeatMax;
    sprintf(wsSend, "setPulsHeatMax, %s", clientPulsHeatMax.c_str());
    _dThis = true;
    Debugf("websocket.send(%s)\n", wsSend);
    webSocket.broadcastTXT(wsSend);
  }
  
  if (firstTime) 
  {
    firstTime = false;
    sprintf(wsSend, "upTime, %s", uptime().c_str());
    _dThis = true;
    Debugf("websocket.send(%s)\n", wsSend);
    webSocket.broadcastTXT(wsSend);

    sprintf(wsSend, "aircoTime, %s", dateTime);
    _dThis = true;
    Debugf("websocket.send(%s)\n", wsSend);
    webSocket.broadcastTXT(wsSend);
    
    clientPollTime = (((nextPollTime - millis()) + 1) / 1000);
    sprintf(wsSend, "aircoNextPoll, %d", clientPollTime);
    _dThis = true;
    Debugf("websocket.send(%s)\n", wsSend);
    webSocket.broadcastTXT(wsSend);
  }

} // getAircoState()


//=======================================================================
void sendTableHistory() 
{
  float     prevTemp, temp = 0;
  String    jsonTimestamp, jsonInTemp0, sKomma;
  uint32_t  dt;
  char      dateTime[20];
  uint8_t   savHour, countHours, stopMark;

  jsonString  = "[";
  sKomma      = "";
  savHour     = 99;
  prevTemp    = 0;
  countHours  = 0;

//temp  = getInsideTemp(0);
  dataStore[dataPointer].timestamp    = now() - (timeZone * SECS_PER_HOUR);
  dataStore[dataPointer].pointType    = 'm';
  dataStore[dataPointer].outsideTemp  = dataStore[(dataPointer - 1)].outsideTemp;
  dataStore[dataPointer].insideTemp0  = getInsideTemp(0);
  dataStore[dataPointer].insideTemp1  = getInsideTemp(1);
  
  for ( int16_t r = (dataPointer - 0) ; ((r > 0) && (countHours <= 10)) ; r-- ) 
  {
    dt = dataStore[r].timestamp + (timeZone * SECS_PER_HOUR);
    sprintf(dateTime, "\"%02d-%02d-%04d %02d:%02d\"", day(dt), month(dt), year(dt), hour(dt),minute(dt));
    if ((savHour == hour(dt)) && (r < (dataPointer - 0))) continue;
    savHour = hour(dt);
    countHours++;
    jsonString +=  sKomma + "{\"meting\":" + String(dateTime); 
    jsonString +=  ",\"waardeOut\":\"" + String(dataStore[r].outsideTemp, 1) + "\"";
    jsonString +=  ",\"waardeIn0\":\"" + String(dataStore[r].insideTemp0, 1) + "\"";
    jsonString +=  ",\"waardeIn1\":\"" + String(dataStore[r].insideTemp1, 1) + "\"";
    jsonString +=  ",\"units\":\"&nbsp;°C\"";
    jsonString +=  ",\"vorige\":\"" + String(prevTemp, 1) + "\"";
    jsonString += "}";
    sKomma      = ",";
    prevTemp    = dataStore[r].insideTemp0;
  }
  jsonString   += "]";
  //Debugln(String(jsonString));
  //HttpServer.send(200, "application/json", jsonString);
  _dThis = true;
  Debugln("sendTableHistory(): Send data");

} // sendTableHistory()


//=======================================================================
void sendHistory() 
{  
  static bool firstTime;
  String    jsonString, jsonTimestamp, jsonOutTemp, jsonInTemp0, jsonInTemp1, sKomma;
  uint16_t  waitCount = 0;  
  int16_t   startP;
  
  _dThis = true;
  Debugf("sendHistory(): FreeHeap @start [%d], dataPointer is [%ld]\n", ESP.getFreeHeap(), dataPointer);

  if (newTab) 
  {
    newTab    = false;
    firstTime = true;
  }

  if (clientActiveTab != TAB_GRAFIEK && clientActiveTab != TAB_HISTORIE) 
  {
    return;
  }

//  uint32_t  dt;
//  String    dateTime;
  
  jsonTimestamp = "\"timestamp\": [";
  jsonOutTemp   = "\"outTemp\": [";
  jsonInTemp0   = "\"inTemp0\": [";
  jsonInTemp1   = "\"inTemp1\": [";
  sKomma        = "";

  if (dataPointer > 180) startP = dataPointer - 180;
  else                   startP = 0;
  for ( int16_t r = startP ; r < dataPointer ; r++ ) 
  {
    jsonTimestamp   +=  sKomma + String(dataStore[r].timestamp);
    jsonOutTemp     +=  sKomma + String(dataStore[r].outsideTemp, 1);
    jsonInTemp0     +=  sKomma + String(dataStore[r].insideTemp0, 1);
    jsonInTemp1     +=  sKomma + String(dataStore[r].insideTemp1, 1);
    sKomma = ",";
    Debug(".");
  }
  Debugln(" ");

  jsonTimestamp   += "]";
  jsonOutTemp     += "]";
  jsonInTemp0     += "]";
  jsonInTemp1     += "]";
//Debugln(jsonInTemp1);

  if ((ESP.getFreeHeap() < 3288) && (waitCount < 100)) 
  {
    _dThis = true;
    Debugf("Wait for more freeHeap (is now %ld bytes)\n", ESP.getFreeHeap());
    delay(100);
    waitCount++;
    if (waitCount > 99) { Debugln("Ok, just try .."); }
  }


  jsonString  = "{";
  jsonString += "\"msgType\": \"sendHistTimestamp\",";
  _dThis = true;
  Debugf("websocket.send(%s .. %d bytes)\n", jsonString.c_str(), jsonTimestamp.length()); // + jsonTimestamp + jsonOutTemp + jsonInTemp1 + "}");
  webSocket.broadcastTXT(jsonString + jsonTimestamp + "}");
  delay(200);

  jsonString  = "{";
  jsonString += "\"msgType\": \"sendHistInTemp\",";
  _dThis = true;
  Debugf("websocket.send(%s .. %d bytes)\n", jsonString.c_str(), jsonInTemp1.length()); // + jsonTimestamp + jsonOutTemp + jsonInTemp1 + "}");
  webSocket.broadcastTXT(jsonString + jsonInTemp1 + "}");
  delay(200);

  jsonString  = "{";
  jsonString += "\"msgType\": \"sendHistOutTemp\",";
  _dThis = true;
  Debugf("websocket.send(%s .. %d bytes)\n", jsonString.c_str(), jsonOutTemp.length()); // + jsonTimestamp + jsonOutTemp + jsonInTemp1 + "}");
  webSocket.broadcastTXT(jsonString + jsonOutTemp + "}");
  delay(100);
  
} // sendHistory()


//=======================================================================
void displayHistory(String displayText, int16_t fromPoint = 0, int16_t toPoint = MAXDATA ) {
  uint32_t  dt;
  char      dateTime[30], indPoint; 
  int16_t   displayFrom, displayTo;

  _dThis = true;
  Debugln(displayText);
  DebugFlush();

  if (fromPoint < 0)  { fromPoint = 0; }
  displayFrom  = fromPoint - 1;
  while (displayFrom < 0) { displayFrom++; }
  if (toPoint >= dataPointer) { toPoint = dataPointer - 1; }
  displayTo    = toPoint + 1;
  while (displayTo >= dataPointer) { displayTo--; }

  //for ( int r = 0; r < dataPointer ; r++ ) 
  for ( int16_t r = displayFrom; r <= displayTo ; r++ ) 
  {
    if (r == fromPoint)     indPoint = '@';
    else if (r == toPoint)  indPoint = '#';
    else                    indPoint = ' ';
    dt = dataStore[r].timestamp + (timeZone * SECS_PER_HOUR); 
    sprintf(dateTime, "%04d-%02d-%02d %02d:%02d:%02d", year(dt), month(dt), day(dt)
                                                     , hour(dt), minute(dt), second(dt)
           );
    _dThis = true;
    Debugf("%s: [%3d]%c %d (%s) -> [%c] (out)%s (in0)%s (in1)%s \n", displayText.c_str(), r, indPoint
                                        , dataStore[r].timestamp
                                        , dateTime
                                        , dataStore[r].pointType
                                        , String(dataStore[r].outsideTemp, 1).c_str()
                                        , String(dataStore[r].insideTemp0, 1).c_str()
                                        , String(dataStore[r].insideTemp1, 1).c_str());
     
  }  
} // displayHistory()


//=======================================================================
void buildHistory() 
{
  uint32_t dt;
  char dateTime[30]; 

  dt = now() - (timeZone * SECS_PER_HOUR); 
  dt = (int)((dt) / 100) * 100; 

  
  randomSeed(minute(dt));

  _dThis = false;
  dataPointer = 0;

  for (int16_t r = (MAXDATA - 2); r >= 0 ; r-- ) 
  {
    dataStore[r].timestamp = dt;
    dataStore[r].pointType = 'm';

    // ===== verzin wat data =====
    inTemp0 = inTemp0 + ((float)(random(-15,184) / 50.0) * increment0);
    if (inTemp0 > 22.0)       { increment0 = -1.0; }
    else if (inTemp0 < -9.0)  { increment0 =  1.0; }
    inTemp1 = inTemp1 + ((float)(random(-15,192) / 52.0) * increment1);
    if (inTemp1 > 23.0)       { increment1 = -1.0; }
    else if (inTemp1 < -10.0) { increment1 =  1.1; }
    dataStore[r].insideTemp0 = inTemp0;
    dataStore[r].insideTemp1 = inTemp1;
    dataStore[r].outsideTemp = (inTemp0 + inTemp1 + (float)random(-10, 10) * 1.01) / 3.0;
    Debugf("r[%d]: %02d-%02d-%04d %02d:%02d => O[%s], T0[%s], T1[%s]\n", r
                              , day(dataStore[r].timestamp),  month(dataStore[r].timestamp),  year(dataStore[r].timestamp)
                              , hour(dataStore[r].timestamp), minute(dataStore[r].timestamp)
                              , String(outTemp,1).c_str(), String(inTemp0,1).c_str(), String(inTemp1,1).c_str());
                              
    dt = (int)((dt) / 100) * 100; 
    dt -=  300;  // plot ieder vijf minuten een punt!
    dataPointer++;
  }
  dataPointer--;
  saveHistory(true);
  loadHistory();
  
} // buildHistory()


//=======================================================================
void deletePoints(int16_t fromPoint, int16_t toPoint) 
{
  int16_t fromList, toList;
  
  _dThis = true;
  Debugf("[from %d, to %d]: ============BEFORE=============\n", fromPoint, toPoint);
  fromList = fromPoint - 2;
  while (fromList < 0) { fromList++; }
  toList = toPoint + 2;
  while (toList >= MAXDATA) { toList--; }
  displayHistory("deletePoints[display]", fromPoint, toPoint);
  //_dThis = true;
  //Debugf("before for r=%d; r < %d; r++\n", fromPoint, (MAXDATA - toPoint));
  //DebugFlush();

  _dThis = true;
  for (int16_t r=fromPoint; r < (MAXDATA - toPoint); r++) 
  {
    Debugf("%d ", r); DebugFlush();
    dataStore[r] = dataStore[r+toPoint];
  }
  //Debugln("Done"); DebugFlush();
  //_dThis = true;
  //Debugf("after for-loop / toPoint[%d], fromPoint[%d], dataPointer[%d] => [%d]\n", toPoint, fromPoint
  //                                                  , dataPointer, (dataPointer - (toPoint - fromPoint)));
  //DebugFlush();
  
  dataPointer -= (toPoint - fromPoint);
  if (dataPointer < 0)        dataPointer = 0;
  if (dataPointer > MAXDATA)  dataPointer = MAXDATA;
  dataStore[dataPointer].timestamp    = 0;
  dataStore[dataPointer].pointType    = '-';
  dataStore[dataPointer].outsideTemp  = 0;
  dataStore[dataPointer].insideTemp0  = 0;
  dataStore[dataPointer].insideTemp1  = 0;

  _dThis = true;
  Debugf("[from %d, to %d]: ============AFTER==============\n", fromPoint, toPoint);
  DebugFlush();
  displayHistory("[display]", (fromPoint -1), (toPoint + 1));
  DebugFlush();

} // deletePoints()


//=======================================================================
void compressHistory2Hours() 
{
  uint32_t  dt;
  uint64_t  gemTimestamp;
  int16_t   countH, startCompr, endCompr;
  int       savDay, savHour;
  float     gemInTemp0, gemInTemp1, gemOutTemp;
  

  startCompr = 0;
  while (dataStore[startCompr].pointType == 'h' && startCompr < MAXDATA) 
  {
      startCompr++;
  }
//if (startCompr > 48)    // maximaal 2 dagen op uur-basis (14-01-2019)
  if (startCompr > 96)    // maximaal 4 dag op uur-basis
  {
    _dThis = true;
    Debugln("delete hours 0 to 6 ...");
    deletePoints(0, 6);
  }

  startCompr = 0;
  // set startpoint to first non-"h"!
  while (dataStore[startCompr].pointType == 'h' && startCompr < MAXDATA) 
  {
      startCompr++;
  }
  // startCompr is the first NON-"h" dataPoint!
  
  dt           = dataStore[startCompr].timestamp;   // set timestamp @pointer
  gemTimestamp = 0;
  gemInTemp0   = 0;
  gemInTemp1   = 0;
  gemOutTemp   = 0;
  savDay       = day(dt);
  savHour      = hour(dt);

  for (endCompr = startCompr; endCompr < dataPointer; endCompr++) 
  {
    if (day(dataStore[endCompr].timestamp) != savDay || hour(dataStore[endCompr].timestamp) != savHour) 
    {
      break;
    }
    gemTimestamp += dataStore[endCompr].timestamp;
    gemOutTemp   += dataStore[endCompr].outsideTemp;
    gemInTemp0   += dataStore[endCompr].insideTemp0;
    gemInTemp1   += dataStore[endCompr].insideTemp1;
    /*
    Debugf("compressHistory2Hours(): compress [%3d] %04d-%02d-%02d %02d:%02d, %s)\n"
                                      , endCompr
                                      , year(dataStore[endCompr].timestamp)
                                      , month(dataStore[endCompr].timestamp)
                                      , day(dataStore[endCompr].timestamp)
                                      , hour(dataStore[endCompr].timestamp)
                                      , minute(dataStore[endCompr].timestamp)
                                      , String(dataStore[endCompr].insideTemp0, 1).c_str());
      */
      dataStore[endCompr].outsideTemp = 0;
      dataStore[endCompr].insideTemp0 = 0;
      dataStore[endCompr].insideTemp1 = 0;
  }
  endCompr--;
  _dThis = true;
  Debugf("from %d to %d\n", startCompr, endCompr);
  dataStore[startCompr].timestamp   = gemTimestamp / (1 + (endCompr - startCompr));
  dataStore[startCompr].outsideTemp = gemOutTemp   / (1 + (endCompr - startCompr));
  dataStore[startCompr].insideTemp0 = gemInTemp0  / (1 + (endCompr - startCompr));
  dataStore[startCompr].insideTemp1 = gemInTemp1  / (1 + (endCompr - startCompr));
  dataStore[startCompr].pointType   = 'h';

  for (int16_t r = (startCompr + 1); (r < (dataPointer - (endCompr - startCompr))) && (r < (MAXDATA - (endCompr - startCompr))); r++) 
  {
    _dThis = true;
    Debugf("dataStore[%d] := dataStore[%d]\n", r, (r + endCompr - startCompr));
    dataStore[r] = dataStore[(r + endCompr - startCompr)];
  }
  _dThis = true;
  Debugf("changed dataPointer from [%d] ", dataPointer);
  dataPointer -= (endCompr - startCompr);
  Debugf("to [%d]\n", dataPointer);

  for (int16_t r = dataPointer; r < MAXDATA; r++) 
  {
    dataStore[r].timestamp    = 0;
    dataStore[r].pointType    = '-';
    dataStore[r].outsideTemp  = 0;
    dataStore[r].insideTemp0  = 0;
    dataStore[r].insideTemp1  = 0;
  }
  
} // compressHistory2Hours()

//=======================================================================
void addPointToHistory()
{
  uint32_t  dt, tps = now() - (timeZone * SECS_PER_HOUR);  // 1 uur eraf;
  float     tempR, tempR2;
  uint8_t   savDay, countHours;
  char      dateTime[20];

  if (millis() > nextPlotTime) 
  {
    if (dataPointer < 25) {  // 50
      nextPlotTime = millis() + ((PLOTINTERVAL * 100) - (second() * 1000));  // 1 minuut
    } else if (dataPointer < 50) 
    {
      nextPlotTime = millis() + ((PLOTINTERVAL * 200) - (second() * 1000));  // 2 minuten
    } else if (minute()%10 != 0) 
    {
      int16_t secondsToGo = (((10 - (minute()%10)) * 60) - second() + 2);
      if (secondsToGo <= 1) 
      {
        //Debugf("secondsToGo[%d]\n", secondsToGo);
        secondsToGo = 30;
      }
      _dThis = true;
      Debugf("minute[%d], minute%%10[%d] ==> seconds from now[%d]\n", minute()
                                                                    , (minute()%10)
                                                                    , secondsToGo
           );
      nextPlotTime = millis() + (secondsToGo * 1000);  // next plot on the 10 minute marks
    } else 
    {
      nextPlotTime = millis() + ((PLOTINTERVAL - second()) * 1000);  // 10 minuten interval
    }
  } else 
  {
    _dThis = true;
    Debugf("Next plot in [%d] seconden ==> Skip!\n", ((nextPlotTime - millis()) / 1000));
    return;
  }
  if (!hasDS18B20sensor) 
  {
    nextPlotTime = millis() + 90000;  // 1,5 minute interval
  }
  _dThis = true;
  Debugf("%02d:%02d:%02d (timestamp: %d)\n", hour(), minute(), second(), tps);

  if ( tps > 1000 ) 
  {
    tps = (int)(tps / 100) * 100; // skip seconden
    if ( dataPointer >= MAXDATA ) 
    {
      //if (dataStore[23].pointType == 'h') {         // this was
      if (dataStore[MAXDATA - 48].pointType == 'h')   // this is as of 18-01-2019
      {
        deletePoints(0, 6); // delete first 6 hours
      } else 
      {
        deletePoints(0, 1); // just delete the oldest point 
      }
      savDay = day(dataStore[0].timestamp);
      countHours = 1;
      //---- compress 3 hours ----
      while(savDay == day(dataStore[countHours].timestamp) && (countHours <= 3)) 
      {
        compressHistory2Hours();
        countHours++;
        dt = dataStore[countHours].timestamp; 
        sprintf(dateTime, "%04d-%02d-%02d %02d:%02d:%02d", year(dt), month(dt), day(dt)
                                                         , hour(dt), minute(dt), second(dt)
           );
        _dThis = true;
        Debugf("[%3d] (%s) -> [%c] (out)%s \t(in)%s\n", countHours 
                                        , dateTime
                                        , dataStore[countHours].pointType
                                        , String(dataStore[countHours].outsideTemp, 1).c_str()
                                        , String(dataStore[countHours].insideTemp0, 1).c_str()
                                        , String(dataStore[countHours].insideTemp1, 1).c_str());
        }
    }
    if (dataPointer < MAXDATA) 
    {
      float newOutTemp = getOutsideTemp(false);
      if (newOutTemp < -999.0)  
      {
        _dThis = true;
        Debugln("Error reading outside Temperature!");
      } else {
        outTemp = newOutTemp;
      }
      _dThis = true;
      Debugf("AddPoint [%3d] %d, (out)%s \t(in0)%s\t(in1)%s\n", dataPointer
                                                 , tps
                                                 , String(outTemp, 1).c_str()
                                                 , String(inTemp0, 1).c_str()
                                                 , String(inTemp1, 1).c_str());
      dataStore[dataPointer].timestamp    = tps;
      dataStore[dataPointer].pointType    = 'm';
      dataStore[dataPointer].outsideTemp  = outTemp;
      dataStore[dataPointer].insideTemp0  = inTemp0;
      dataStore[dataPointer].insideTemp1  = inTemp1;
      dataPointer++;
    } else 
    {
      Debugf("Some error occured! dataPointer[%d] > maxdata[%d]\n", (dataPointer - 1), MAXDATA);
    }
    //char tc[20];    dtostrf(t, 4, 1, tc);
    _dThis = true;
    Debugf("dataPointer %d (next point)\n", dataPointer);
    _dThis = true;
    Debugf("Next plot in [%d] seconden\n", ((nextPlotTime - millis()) / 1000));
    saveHistory();
  }  
  
} // addPointToHistory()


//===========================================================================================
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) 
//===========================================================================================
{
    String  text = String((char *) &payload[0]);
    char *  textC = (char *) &payload[0];
    String  pOut[5], pDev[5], pVal[5], words[10], jsonString;
    int8_t  deviceNr;

    switch(type) 
    {
        case WStype_DISCONNECTED:
            _dThis = true;
            Debugf("[%u] Disconnected!\n", num);
            isConnected = false;
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                if (!isConnected) 
                {
                  _dThis = true;
                  Debugf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
                  isConnected = true;
                  webSocket.broadcastTXT("{\"msgType\":\"ConnectState\",\"Value\":\"Connected\"}");
                  //sendDevInfo();
                }
        
            }
            break;
        case WStype_TEXT:

            _dThis = true;
            Debugf("[%u] Got message: [%s]\n", num, payload);
            splitString(text, ':', words, 10);
            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            if (text.indexOf("tabControl") > -1) 
            {
              clientActiveTab   = TAB_CONTROL;
              newTab            = true;
              clientInTemp0     = -99.99;
              clientInTemp1     = -99.99;
              clientOutTemp     = -99.99;
              clientCoolTemp    =  99;
              clientHeatTemp    = -99;
              clientDemping     =  99;
              clientPollTime    =   0;
              clientPlotTime    =   0;
              clientDataPointer =   0;
              clientState       = '-';
              clientUpdateURL   = '-';
              clientFingerPrint = '-';
              clientWeerLiveKey = '-';
              clientPulsOff     = '-';
              clientPulsCool    = '-';
              clientPulsCoolMax = '-';
              clientPulsHeat    = '-';
              clientPulsHeatMax = '-';
              
            } else if (text.indexOf("tabGrafiek") > -1) 
            {
              clientActiveTab = TAB_GRAFIEK;
              newTab          = true;
              sendHistory();
              
            } else if (text.indexOf("tabHistorie") > -1) 
            {
              clientActiveTab = TAB_HISTORIE;
              newTab          = true;
              sendHistory();
              
            } 
                        
            if (text.indexOf("setCoolTemp") > -1) 
            {
              settingCoolThreshold = words[1].toInt();
            } else if (text.indexOf("setHeatTemp") > -1) 
            {
              settingHeatThreshold = words[1].toInt();
            } else if (text.indexOf("setDemping") > -1) 
            {
              settingDemping       = words[1].toInt();
            } else if (text.indexOf("setUpdateURL") > -1) 
            {
              settingUpdateURL = text.substring(text.indexOf("http"));
              settingUpdateURL.trim();
            } else if (text.indexOf("setFingerPrint") > -1) 
            {
              settingFingerPrint = text.substring(text.indexOf(":") + 1);
              settingFingerPrint.trim();
            } else if (text.indexOf("setWeerLiveKey") > -1) 
            {
              settingWeerLiveKey = text.substring(text.indexOf(":") + 1);
              settingWeerLiveKey.trim();
            } else if (text.indexOf("setPulsOff") > -1) 
            {
              settingPulsOff       = words[1];
            } else if (text.indexOf("setPulsCool") > -1) 
            {
              settingPulsCool      = words[1];
            } else if (text.indexOf("setPulsCoolMax") > -1) 
            {
              settingPulsCoolMax   = words[1];
            } else if (text.indexOf("setPulsHeat") > -1) 
            {
              settingPulsHeat      = words[1];
            } else if (text.indexOf("setPulsHeatMax") > -1) 
            {
              settingPulsHeatMax    = words[1];
              
            } else if (text.indexOf("saveSettings") > -1) 
            {
              writeSettings();
            }
                        
            if (text.indexOf("getDevInfo") > -1) 
            {
              jsonString  = "{";
              jsonString += "\"msgType\": \"devInfo\"";
              jsonString += ", \"devName\": \"" + String(_HOSTNAME) + " \"";
              jsonString += ", \"devIPaddress\": \"" + WiFi.localIP().toString() + " \"";
              jsonString += ", \"devVersion\": \" [" + String(_FW_VERSION) + "]\"";
              jsonString += "}";
              _dThis = true;
              Debugf("websocket.send(%s)\n", jsonString.c_str());
              webSocket.broadcastTXT(jsonString);
            } else if (text.startsWith("Button")) 
            {
              //splitString(text, ':', words, 10);
              _dThis = true;
              if (words[1] == "COOL")       updateAirco(1);
              else if (words[1] == "HEAT")  updateAirco(2);
              else if (words[1] == "OFF")   updateAirco(0);
            }
            break;
                        
    } // switch(type)
    
} // webSocketEvent()


//=======================================================================
void setup() 
{  
  Serial.begin ( 115200 );
  lastResetReason = ESP.getResetReason();

  pinMode(IR_LED, OUTPUT);
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, HIGH);

  //setTime(dateTime2Epoch(__DATE__, __TIME__));
  startWiFi();
    
  //--- ezTime initialisation
  setDebug(INFO);  
  waitForSync(); 
  CET.setLocation(F("Europe/Amsterdam"));
  CET.setDefault(); 
  
  Debugln("UTC time: "+ UTC.dateTime());
  Debugln("CET time: "+ CET.dateTime());

  _dThis = true;
  Debugf("last Reset Reason [%s]\n", lastResetReason.c_str());
  _dThis = true;
  Debugf("Compile time [%04d-%02d-%02d @ %02d:%02d:%02d]\n", year(), month(), day(), hour(), minute(), second());
  startArduinoOTA(_HOSTNAME);
  startTelnet();
 
  for (int f=0; f<10; f++) 
  {
    delay(200);
    digitalWrite(LEDPIN, !digitalRead(LEDPIN));
  }
  digitalWrite(LEDPIN, HIGH);

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  digitalWrite(LEDPIN, HIGH);

  _dThis = true;
  if (!SPIFFS.begin()) 
  {
    Debugln("SPIFFS Mount failed\n");           // Serious problem with SPIFFS 
  } else 
  { 
    Debugln("SPIFFS Mount succesfull");
    loadHistory();
  }

  // --- initialize -----
  readSettings();
  
//HttpServer.on("/tab_historie.json", sendTableHistory);
//HttpServer.on("/tab_temp.json",     sendTabTemp);
//HttpServer.on("/setAirco",          updateAirco);
  HttpServer.on("/getAirco.json",     apiAircoState);
//HttpServer.on("/graph_temp.json",   sendHistory);
  HttpServer.on("/ReBoot", HTTP_POST, handleReBoot);

  HttpServer.serveStatic("/",                SPIFFS, "/aircoCntrlr.html");
  HttpServer.serveStatic("/aircoCntrlr.js",  SPIFFS, "/aircoCntrlr.js");
  HttpServer.serveStatic("/aircoCntrlr.css", SPIFFS, "/aircoCntrlr.css");
  HttpServer.serveStatic("/FSexplorer.png",  SPIFFS, "/FSexplorer.png");

  HttpServer.on("/FSexplorer", HTTP_POST, handleFileDelete);
  HttpServer.on("/FSexplorer", handleRoot);
  HttpServer.on("/FSexplorer/upload", HTTP_POST, []() {
    HttpServer.send(200, "text/plain", "");
  }, handleFileUpload);

  HttpServer.onNotFound([]() {
    if (!handleFileRead(HttpServer.uri()))
      HttpServer.send(404, "text/plain", "FileNotFound");
  });

  HttpUpdater.setup(&HttpServer); // OTA upload!
  HttpServer.begin();
  _dThis = true;
  Debugln( "HTTP server started\n" );

/*  
 *   list all services with the cammand:
 *   dns-sd -B _arduinoe .
*/
  startMDNS(_HOSTNAME);
  MDNS.port(81);  // webSockets

  _dThis = true;
  Debug("setup(): Start Time: ");
  Debugf("%02d:%02d:%02d\n", hour(), minute(), second());

  DS18B20.begin();
  
  // locate devices on the bus
  _dThis = true;
  Debugf("Locating devices... Found [%d] devices\n", DS18B20.getDeviceCount());

  // search() looks for the next device. Returns 1 if a new address has been
  // returned. A zero might mean that the bus is shorted, there are no devices,
  // or you have already retrieved all of them. It might be a good idea to
  // check the CRC to make sure you didn't get garbage. The order is
  // deterministic. You will always get the same devices in the same order
  //
  // Must be called before search()
  //DS18B20_0 = null;
  oneWire.reset_search();
  _dThis = true;
  if (DS18B20.getDeviceCount() == 0) Debugln("Unable to find address for DS18B20's");

  inTemp0 = getInsideTemp(0);
  inTemp1 = getInsideTemp(1);
  
  _dThis = true;
  if (!hasDS18B20sensor) 
  {
    inTemp0  = 0;
    inTemp1  = 1;
    tn0      = 0;
    tn1      = 1;
    Debugln("setup(): no inside temperature sensors found!");
    Debugln("setup(): making up temperature data..\n");
  } else 
  {
    Debugln("setup(): has inside temperature sensor(s)");
    //inTemp0  = getInsideTemp(0);
    tn0     = inTemp0;
    tn1     = inTemp1;
  }
  
  outTemp = getOutsideTemp(true);

  for (int f=0; f<6; f++) 
  {
    delay(500);
    digitalWrite(LEDPIN, !digitalRead(LEDPIN));
  }
  digitalWrite(LEDPIN, HIGH);
  ledOnState    = true;
  ledOnAt       = now();
  _dThis = true;
  Debugf("last Reset Reason [%s]\n", lastResetReason.c_str());
  
  nextPollTime    = millis() + ((61 - second()) * 1000);
  nextPlotTime    = millis() + 30000;  // first plot after 30 seconds
  ledOnTime       = 0;
  ledOffTime      = millis() + 1000;  // switch LED off after 10 minutes;

  clientActiveTab = TAB_UNKNOWN;
  ledOffAt        = now();
  aircoOnAt       = now();
  aircoOffAt      = now();
  last21Hour      = hour();
  upTimeSeconds   = millis() / 1000;
  nextSecond      = millis() + 1000;
  nextStateSend   = millis() + 2000;
  
  _dThis = true;
  Debugln("================ done with setup() =====================\n");
  
} // setup()


//=======================================================================
void loop() 
{
  ArduinoOTA.handle();
  HttpServer.handleClient();
  webSocket.loop();
  handleLed();  
  events(); // trigger ezTime update etc.
//if (loopNTP()) {  // if time is not set .. no use in plotting
    handleSensor();
    handleAutonomous();
    handlePlotPoint();
//}
  handleKeyInput(); // menu

  if (millis() > nextSecond) 
  {
    nextSecond += 1000;
    upTimeSeconds++;
  }
  if (millis() > nextStateSend) 
  {
    nextStateSend += 2000;
    getAircoState();
  }

} // loop()
