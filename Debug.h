
bool _dThis = false;

#define Debug(...)      ({  if (_dThis == true) { \
                                   Serial.printf("[%02d:%02d:%02d] ", hour(), minute(), second()); \
                                   TelnetStream.printf("[%02d:%02d:%02d] ", hour(), minute(), second()); \
                                   Serial.printf("%s(%d): ",__FUNCTION__, __LINE__); \
                                   TelnetStream.printf("%s(%d): ",__FUNCTION__, __LINE__); \
                                   _dThis = false;  \
                            } \
                            Serial.print(__VA_ARGS__); \
                            TelnetStream.print(__VA_ARGS__); \
                        })
#define Debugln(...)    ({  if (_dThis == true) { \
                                   Serial.printf("[%02d:%02d:%02d] ", hour(), minute(), second()); \
                                   TelnetStream.printf("[%02d:%02d:%02d] ", hour(), minute(), second()); \
                                   Serial.printf("%s(%d): ",__FUNCTION__, __LINE__); \
                                   TelnetStream.printf("%s(%d): ",__FUNCTION__, __LINE__); \
                            } \
                            Serial.println(__VA_ARGS__); \
                            TelnetStream.println(__VA_ARGS__); \
                        })
#define Debugf(...)     ({  if (_dThis == true) { \
                                    Serial.printf("[%02d:%02d:%02d] ", hour(), minute(), second()); \
                                    TelnetStream.printf("[%02d:%02d:%02d] ", hour(), minute(), second()); \
                                    Serial.printf("%s(%d): ",__FUNCTION__, __LINE__); \
                                    TelnetStream.printf("%s(%d): ",__FUNCTION__, __LINE__); \
                                    _dThis = false; \
                            } \
                            Serial.printf(__VA_ARGS__); \
                            TelnetStream.printf(__VA_ARGS__); \
                        })
#define DebugFlush()    ({ Serial.flush();   \
                           TelnetStream.flush(); \
                        })
