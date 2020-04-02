
uint16_t rawData[500];  // placeholder for read data

/**
// Example of data captured by IRrecvDumpV2.ino
uint16_t rawOffData[] = {3152, 1636,  362, 432,  344, 1248,  364, 430,  344, 450,  
                            362, 1228,  364, 430,  362, 1228,  344, 452,  362, 430,  
                            362, 1228,  364, 1226,  344, 1246,  364, 432,  362, 1228,  
                            364, 430,  362, 1228,  364, 1226,  364, 1226,  342, 452,  344, 454,  
                            360, 432,  362, 432,  364, 1226,  362, 1228,  362, 434,  344, 1246,  
                            362, 432,  362, 1226,  362, 1230,  364, 430,  344, 450,  364, 432,  
                            364, 1226,  364, 432,  364, 1226,  342, 454,  342, 450,  344, 1248,  
                            360, 1230,  342, 1248,  362, 430,  344, 1246,  362, 1228,  342, 1248,  
                            344, 1248,  344, 1246,  344, 1246,  344, 1248,  344, 1246,  364, 432,  
                            344, 450,  344, 452,  364, 432,  362, 432,  364, 432,  362, 434,  
                            362, 1226,  344, 1246,  364, 1228,  364, 430,  364, 1226,  344, 1246,  
                            342, 1248,  364, 1226,  362, 434,  362, 432,  362, 432,  342, 1248,  
                            344, 452,  362, 430,  364, 432,  344, 450,  364, 1228,  344, 1246,  
                            344, 1246,  364, 1226,  362, 1228,  344, 1246,  344, 1248,  364, 1226,  
                            342, 450,  344, 454,  342, 452,  362, 434,  344, 450,  364, 430,  364, 
                            432,  344, 450,  346, 1246,  362, 1226,  364, 1228,  364, 1224,  
                            364, 1226,  364, 432,  362, 1228,  342, 1248,  344, 452,  342, 452,  
                            364, 430,  364, 432,  342, 452,  342, 1248,  342, 452,  362, 430,  
                            364, 1228,  364, 1226,  364, 1226,  364, 1226,  364, 1228,  344, 452,  
                            362, 1226,  362, 1228,  364, 430,  362, 432,  362, 434,  362, 432,  
                            364, 432,  364, 1228,  362, 430,  362, 432,  362, 1228,  364, 1226,  
                            362, 1228,  364, 1226,  362, 1228,  364, 1226,  344, 1248,  342, 1248,  
                            362, 432,  362, 432,  342, 452,  362, 434,  364, 432,  362, 430,  362, 432,  
                            362, 434,  342, 1248,  342, 1248,  362, 1228,  362, 1228,  
                            364, 1228,  344, 1246,  364, 1228,  362, 430,  364, 432,  362, 432,  
                            362, 432,  344, 452,  362, 434,  362, 432,  342, 452,  364, 1226,  344
                            };  // UNKNOWN BBA706B7


uint16_t rawCoolData[] = {3176, 1612,  366, 430,  366, 1222,  366, 430,  366, 428,  
                           366, 1224,  366, 430,  366, 1222,  366, 430,  366, 428,  
                           366, 1224,  366, 1224,  366, 1224,  366, 430,  366, 1224,  
                           366, 428,  368, 1224,  366, 1224,  366, 1224,  366, 428,  
                           366, 428,  366, 430,  366, 428,  366, 1224,  366, 1224,  
                           366, 430,  366, 1224,  366, 428,  366, 1224,  366, 1224,  
                           366, 428,  366, 428,  366, 428,  368, 1222,  366, 428,  
                           366, 1224,  366, 428,  366, 428,  366, 1224,  368, 1224,  
                           366, 1222,  366, 428,  366, 1224,  366, 1224,  366, 428,  
                           366, 1224,  366, 1224,  366, 1224,  366, 1224,  366, 1222,  
                           366, 430,  366, 428,  366, 1224,  366, 428,  366, 428,  366, 430,  
                           366, 428,  366, 1224,  366, 1224,  368, 1222,  
                           366, 430,  368, 1222,  368, 1224,  366, 1222,  366, 1224,  
                           366, 428,  366, 428,  366, 428,  366, 1222,  366, 430,  368, 426,  
                           366, 430,  366, 428,  366, 1224,  366, 1224,  366, 1224,  
                           366, 1224,  366, 1224,  366, 1224,  366, 1224,  366, 1224,  
                           366, 430,  366, 428,  366, 428,  366, 428,  366, 428,  
                           366, 428,  368, 428,  368, 428,  366, 1224,  366, 1224,  
                           366, 1224,  366, 1224,  366, 1226,  366, 428,  
                           366, 1224,  364, 1226,  366, 430,  366, 428,  366, 428,  
                           366, 430,  366, 428,  366, 1224,  368, 426,  366, 430,  364, 1224,  
                           366, 1224,  366, 1224,  364, 1226,  366, 1224,  366, 428,  
                           366, 1224,  364, 1228,  366, 426,  366, 430,  364, 428,  
                           366, 430,  364, 430,  364, 1224,  366, 430,  
                           366, 430,  364, 1226,  364, 1224,  366, 1224,  
                           366, 1224,  366, 1224,  366, 1224,  366, 1226,  364, 1224,  
                           366, 430,  364, 428,  366, 430,  366, 426,  366, 430,  
                           366, 430,  364, 428,  364, 432,  366, 1224,  364, 1226,  364, 1226,  
                           366, 1224,  366, 1226,  366, 1224,  366, 1224,  366, 428,  
                           366, 430,  364, 432,  364, 430,  366, 428,  366, 430,  366, 430,  
                           366, 428,  366, 1224,  364
                           };  // UNKNOWN C0E1705B

uint16_t rawHeatData[] = {3152, 1634,  364, 430,  364, 1226,  362, 434,  362, 432,  
                           364, 1224,  362, 432,  364, 1226,  362, 432,  364, 428,  
                           364, 1226,  364, 1228,  364, 1224,  364, 430,  364, 1224,  
                           344, 450,  344, 1246,  364, 1228,  364, 1224,  364, 432,  
                           362, 434,  344, 450,  364, 432,  364, 1226,  364, 1226,  
                           364, 430,  364, 1226,  344, 450,  364, 1228,  364, 1226,  
                           364, 430,  362, 432,  364, 430,  364, 1228,  364, 430,  
                           364, 1226,  362, 432,  364, 430,  362, 1226,  364, 1228,  
                           364, 1224,  346, 1246,  364, 1226,  364, 430,  344, 450,  
                           364, 1226,  364, 1226,  364, 1226,  364, 1226,  344, 450,  
                           364, 430,  366, 1224,  364, 1226,  364, 432,  364, 428,  
                           364, 430,  366, 428,  364, 432,  364, 1226,  362, 1228,  
                           364, 1226,  366, 1224,  364, 1226,  364, 1226,  364, 1226,  
                           362, 1228,  364, 430,  364, 430,  366, 430,  364, 430,  
                           364, 430,  364, 432,  362, 432,  364, 1224,  364, 1226,  
                           362, 1226,  364, 1228,  344, 1246,  364, 1226,  364, 1226,  
                           362, 1228,  344, 450,  364, 432,  364, 432,  364, 428,  344, 
                           452,  362, 432,  364, 430,  364, 432,  364, 1226,  364, 1226,  
                           362, 1228,  364, 1226,  344, 1248,  364, 430,  364, 1226,  
                           362, 1228,  364, 432,  364, 430,  344, 452,  362, 432,  364, 430,  
                           364, 1226,  344, 448,  362, 432,  364, 1226,  344, 1246,  364, 1228,  
                           364, 1226,  364, 1226,  364, 430,  364, 1226,  364, 1226,  364, 432,  
                           362, 430,  344, 452,  362, 430,  364, 430,  364, 1226,  364, 432,  
                           364, 428,  364, 1228,  362, 1228,  366, 1224,  364, 1228,  364, 1224,  
                           364, 1226,  364, 1228,  364, 1226,  364, 430,  364, 432,  362, 432,  
                           364, 430,  364, 430,  364, 430,  364, 430,  364, 430,  364, 1226,  
                           364, 1226,  362, 1228,  364, 1224,  364, 1226,  364, 1228,  364, 1226,  
                           364, 430,  364, 430,  364, 430,  364, 430,  364, 432,  364, 430,  366, 428,  
                           366, 430,  362, 1228,  344
                           };  // UNKNOWN E159F5F9

***/