/*
 * Opvragen actueel weer in Nederland
 */

float getOutsideTemp(bool force) {
  static uint32_t waitForNext;
  const char* host = "weerlive.nl";
  const int   httpPort = 80;
  String      tempString;
  int         startPos, endPos;
  int32_t     maxWait;
  
  WiFiClient weerliveClient;

  _dThis = true;
  Debugf("getOutsideTemp(%s)\n", host);

  if (settingWeerLiveKey.length() < 5) return 15;

  if ((millis() < waitForNext) && !force) {
    _dThis = true;
  //Debugln(" too soon .. SKIP!");
    return outTemp;
  }
  waitForNext = millis() + (30 * 60 * 1000);  // 30 minuten interval

  // We now create a URI for the request
  String url = "/api/json-data-10min.php?locatie=Baarn&key="+settingWeerLiveKey;

  _dThis = true;
  Debugf("Requesting URL: [http://%s%s]\r\n", host, url.c_str());
  
  if (!weerliveClient.connect(host, httpPort)) {
    _dThis = true;
    Debugln("connection failed");
    return -999.9;
  }

  // This will send the request to the server
  weerliveClient.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(10);
  
  // Read all the lines of the reply from server and print them to Serial
  jsonString = "";
  //Debugln("Respond:");
  maxWait = millis() + 5000;  // max response time is 5 seconds
  while (maxWait > millis()) 
  {
    yield();
    while(weerliveClient.available())
    {
      String line = weerliveClient.readStringUntil('\r');
      _dThis = true;
      //Debugln(line);
      jsonString += line;
      maxWait = 0;
    }
  }
  if (jsonString.length() < 100) {   // timed out!
    return -999.9;
  }
  _dThis = true;
  Debugln(jsonString);
  startPos   = jsonString.indexOf("\"temp\": ") + 9;
  _dThis = true;
  Debugf("[\"temp\":] is on position [%d]\n", startPos);
  tempString = jsonString.substring(startPos);
  jsonString = tempString;
  endPos     = jsonString.indexOf("\",");
  //Debugf("\", is on position [%d]\n", endPos);
  tempString = jsonString.substring(0, endPos);
  //Debugf("start[%d], end[%d] => [%s]\n", startPos, endPos, tempString.c_str());
  _dThis = true;
  Debugf("From %s --->> outside temperature [%s] *C\n", host, tempString.c_str());

  return tempString.toFloat();
  
} // getOutsideTemp()


/* eof */
