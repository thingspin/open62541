//wiring Pi
#include <wiringPi.h>
#include <wiringSerial.h>

//include system librarys
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#include "open62541.h"

UA_Boolean running = true;

// Find Serial device on Raspberry with ~ls /dev/tty*
// ARDUINO_UNO "/dev/ttyACM0"
// FTDI_PROGRAMMER "/dev/ttyUSB0"
// HARDWARE_UART "/dev/ttyAMA0"
char device[]= "/dev/ttyACM0";
// filedescriptor
int fd;
unsigned long baud = 9600;
unsigned long time=0;
unsigned long recvTime = 0;
int idx = 0; // for serial recv data check
int sensor_idx = 0; // for serial recv data check

//sensor parameter
char str_buf[20] = {0,};
int ambient = 0;
int sound = 0;
int temp = 0;
int humi = 0;

//--Node(fixed sensor) id setting 
UA_NodeId hTemp_SensorId = {1,UA_NODEIDTYPE_NUMERIC,{1001}};
UA_NodeId sound_SensorId = {1,UA_NODEIDTYPE_NUMERIC,{2001}};
UA_NodeId ambient_SensorId = {1,UA_NODEIDTYPE_NUMERIC,{3001}};

UA_Server *s = NULL; /* required to get the server pointer into the constructor
                        function (will change for v0.3) */

//--add Sensor Instance OPC
static void addTempInstance(UA_Server *server) {
    //Add object
    UA_ObjectAttributes oAttr;
    UA_ObjectAttributes_init(&oAttr);
    oAttr.displayName = UA_LOCALIZEDTEXT("en_US", "Temp Sensor");
    UA_Server_addObjectNode(server, UA_NODEID_NULL,
                            UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                            UA_QUALIFIEDNAME(1, "Temp Sensor"), UA_NODEID_NULL,
                            oAttr, NULL, &hTemp_SensorId);
    //Add Attributes
    UA_VariableAttributes modelAttr;
    UA_VariableAttributes_init(&modelAttr);
    UA_String modelName = UA_STRING("DHT11");
    UA_Variant_setScalar(&modelAttr.value, &modelName, &UA_TYPES[UA_TYPES_STRING]);
    modelAttr.displayName = UA_LOCALIZEDTEXT("en_US", "ModelName");
    UA_Server_addVariableNode(server, UA_NODEID_NULL, hTemp_SensorId,
                              UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                              UA_QUALIFIEDNAME(1, "ModelName"),
                              UA_NODEID_NULL, modelAttr, NULL, NULL);

    UA_VariableAttributes tempAttr;
    UA_VariableAttributes_init(&tempAttr);
    UA_Int32 temp = 24; //init dummy data
    UA_Variant_setScalar(&tempAttr.value, &temp, &UA_TYPES[UA_TYPES_INT32]);
    
    UA_NodeId TempNodeId = UA_NODEID_STRING(1,"temp_data");
    
    tempAttr.displayName = UA_LOCALIZEDTEXT("en_US", "temp(deg)");
    UA_Server_addVariableNode(server, TempNodeId, hTemp_SensorId,
                              UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                              UA_QUALIFIEDNAME(1, "temp(deg)"),
                              UA_NODEID_NULL, tempAttr, NULL, NULL);

    UA_VariableAttributes humiAttr;
    UA_VariableAttributes_init(&humiAttr);
    UA_Int32 humi = 50; //init dummy data
    UA_Variant_setScalar(&humiAttr.value, &humi, &UA_TYPES[UA_TYPES_INT32]);
    
    UA_NodeId HumiNodeId = UA_NODEID_STRING(1,"humi_data");
    
    humiAttr.displayName = UA_LOCALIZEDTEXT("en_US", "humi(percent)");
    UA_Server_addVariableNode(server, HumiNodeId, hTemp_SensorId,
                              UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                              UA_QUALIFIEDNAME(1, "humi(percent)"),
                              UA_NODEID_NULL, humiAttr, NULL, NULL);                          
}
static void addSoundInstance(UA_Server *server) {
    //Add object
    UA_ObjectAttributes oAttr;
    UA_ObjectAttributes_init(&oAttr);
    oAttr.displayName = UA_LOCALIZEDTEXT("en_US", "Sound Sensor");
    UA_Server_addObjectNode(server, UA_NODEID_NULL,
                            UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                            UA_QUALIFIEDNAME(1, "Sound Sensor"), UA_NODEID_NULL,
                            oAttr, NULL, &sound_SensorId);
    //Add Attributes
    UA_VariableAttributes modelAttr;
    UA_VariableAttributes_init(&modelAttr);
    UA_String modelName = UA_STRING("Analog Sound v2");
    UA_Variant_setScalar(&modelAttr.value, &modelName, &UA_TYPES[UA_TYPES_STRING]);
    modelAttr.displayName = UA_LOCALIZEDTEXT("en_US", "ModelName");
    UA_Server_addVariableNode(server, UA_NODEID_NULL, sound_SensorId,
                              UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                              UA_QUALIFIEDNAME(1, "ModelName"),
                              UA_NODEID_NULL, modelAttr, NULL, NULL);

    UA_VariableAttributes SoundAttr;
    UA_VariableAttributes_init(&SoundAttr);
    UA_Int32 sound = 1; // init dumy data
    UA_Variant_setScalar(&SoundAttr.value, &sound, &UA_TYPES[UA_TYPES_INT32]);
    
    UA_NodeId currentNodeId = UA_NODEID_STRING(1,"sound_data");
    
    SoundAttr.displayName = UA_LOCALIZEDTEXT("en_US", "sound value");
    UA_Server_addVariableNode(server, currentNodeId, sound_SensorId,
                              UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                              UA_QUALIFIEDNAME(1, "soud value"),
                              UA_NODEID_NULL, SoundAttr, NULL, NULL);
}
static void addAmbientInstance(UA_Server *server) {
    //Add object
    UA_ObjectAttributes oAttr;
    UA_ObjectAttributes_init(&oAttr);
    oAttr.displayName = UA_LOCALIZEDTEXT("en_US", "Ambient light Sensor");
    UA_Server_addObjectNode(server, UA_NODEID_NULL,
                            UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                            UA_QUALIFIEDNAME(1, "Ambient light Sensor"), UA_NODEID_NULL,
                            oAttr, NULL, &ambient_SensorId);
    //Add Attributes
    UA_VariableAttributes modelAttr;
    UA_VariableAttributes_init(&modelAttr);
    UA_String modelName = UA_STRING("Ambient light sensor v2");
    UA_Variant_setScalar(&modelAttr.value, &modelName, &UA_TYPES[UA_TYPES_STRING]);
    modelAttr.displayName = UA_LOCALIZEDTEXT("en_US", "ModelName");
    UA_Server_addVariableNode(server, UA_NODEID_NULL, ambient_SensorId,
                              UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                              UA_QUALIFIEDNAME(1, "ModelName"),
                              UA_NODEID_NULL, modelAttr, NULL, NULL);

    UA_VariableAttributes ambientAttr;
    UA_VariableAttributes_init(&ambientAttr);
    UA_Int32 ambient = 1; //init dummy data
    UA_Variant_setScalar(&ambientAttr.value, &ambient, &UA_TYPES[UA_TYPES_INT32]);
    ambientAttr.displayName = UA_LOCALIZEDTEXT("en_US", "ambient value(lux)");

    UA_NodeId currentNodeId = UA_NODEID_STRING(1,"ambient_data");

    UA_Server_addVariableNode(server, currentNodeId, ambient_SensorId,
                              UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                              UA_QUALIFIEDNAME(1, "ambient value(lux)"),
                              UA_NODEID_NULL, ambientAttr, NULL, NULL);
}


static void AmbientDataUpdate(void *handle, const UA_NodeId nodeid, const UA_Variant *data,const UA_NumericRange *range)
{
    UA_Server *server = (UA_Server*)handle;
    UA_Variant value;
    UA_Int32 ambient_data;
    ambient_data = ambient;
    UA_Variant_setScalar(&value, &ambient_data, &UA_TYPES[UA_TYPES_INT32]);
    UA_NodeId AmbientNodeId = UA_NODEID_STRING(1, "ambient_data");
    UA_Server_writeValue(server, AmbientNodeId, value);
}
static void afterAmbientUpdate(void *handle, const UA_NodeId nodeid, const UA_Variant *data,
               const UA_NumericRange *range) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "The Ambient variable was updated");
}

static void SoundDataUpdate(void *handle, const UA_NodeId nodeid, const UA_Variant *data,const UA_NumericRange *range)
{
    UA_Server *server = (UA_Server*)handle;
    UA_Variant value;
    UA_Int32 sound_data;
    sound_data = sound;
    UA_Variant_setScalar(&value, &sound_data, &UA_TYPES[UA_TYPES_INT32]);
    UA_NodeId SoundNodeId = UA_NODEID_STRING(1, "sound_data");
    UA_Server_writeValue(server, SoundNodeId, value);
}
static void afterSoundUpdate(void *handle, const UA_NodeId nodeid, const UA_Variant *data,
               const UA_NumericRange *range) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "The Sound variable was updated");
}

static void TempDataUpdate(void *handle, const UA_NodeId nodeid, const UA_Variant *data,const UA_NumericRange *range)
{
    UA_Server *server = (UA_Server*)handle;
    UA_Variant value;
    UA_Int32 temp_data;
    UA_Int32 humi_data;
    temp_data = temp;
    UA_Variant_setScalar(&value, &temp_data, &UA_TYPES[UA_TYPES_INT32]);
    UA_NodeId TempNodeId = UA_NODEID_STRING(1, "temp_data");
    UA_Server_writeValue(server, TempNodeId, value);
}
static void afterTempUpdate(void *handle, const UA_NodeId nodeid, const UA_Variant *data,
               const UA_NumericRange *range) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "The Temp variable was updated");
}

static void HumiDataUpdate(void *handle, const UA_NodeId nodeid, const UA_Variant *data,const UA_NumericRange *range)
{
    UA_Server *server = (UA_Server*)handle;
    UA_Variant value;
    UA_Int32 humi_data;
    humi_data = humi;
    UA_Variant_setScalar(&value, &humi_data, &UA_TYPES[UA_TYPES_INT32]);
    UA_NodeId HumiNodeId = UA_NODEID_STRING(1, "humi_data");
    UA_Server_writeValue(server, HumiNodeId, value);
}
static void afterHumiUpdate(void *handle, const UA_NodeId nodeid, const UA_Variant *data,
               const UA_NumericRange *range) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "The Humi variable was updated");
}

//call back
static void addAmbientCallbackVariable(UA_Server *server)
{
    UA_NodeId AmbientNodeId = UA_NODEID_STRING(1, "ambient_data");
    UA_ValueCallback callback ;
    callback.handle = server;
    callback.onRead = AmbientDataUpdate;
    callback.onWrite = afterAmbientUpdate;
    UA_Server_setVariableNode_valueCallback(server, AmbientNodeId, callback);
}

static void addSoundCallbackVariable(UA_Server *server)
{
    UA_NodeId SoundNodeId = UA_NODEID_STRING(1, "sound_data");
    UA_ValueCallback callback ;
    callback.handle = server;
    callback.onRead = SoundDataUpdate;
    callback.onWrite = afterSoundUpdate;
    UA_Server_setVariableNode_valueCallback(server, SoundNodeId, callback);
}

static void addTempCallbackVariable(UA_Server *server)
{
    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "temp_data");
    UA_ValueCallback callback ;
    callback.handle = server;
    callback.onRead = TempDataUpdate;
    callback.onWrite = afterTempUpdate;
    UA_Server_setVariableNode_valueCallback(server, currentNodeId, callback);
}

static void addHumiCallbackVariable(UA_Server *server)
{
    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "humi_data");
    UA_ValueCallback callback ;
    callback.handle = server;
    callback.onRead = HumiDataUpdate;
    callback.onWrite = afterHumiUpdate;
    UA_Server_setVariableNode_valueCallback(server, currentNodeId, callback);
}

static void stopHandler(int sig) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "received ctrl-c");
    running = false;
}

//get sensor data from arduino by USBserial
PI_THREAD(waitingThread)
{
  //get filedescriptor
  if ((fd = serialOpen (device, baud)) < 0){
    fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
    exit(1); //error
  }
 
  //setup GPIO in wiringPi mode
  if (wiringPiSetup () == -1){
    fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno)) ;
    exit(1); //error
  }

 while(1)
  {
    // Pong every 3 seconds
    if(millis()-time>=500)
    {
     serialPuts (fd, "Pong!\n");
     // you can also write data from 0-255
     // 65 is in ASCII 'A'
     serialPutchar (fd, 65);
     time=millis();
    }
    // read signal
    if(serialDataAvail (fd))
    {
     char newChar =serialGetchar(fd);    
     //printf("%c\n",newChar);
     fflush(stdout);
     if(newChar == 35) // 35 == '#'
     {
          //printf("%s len:%d\n",str_buf,idx);
              char *ptr = strtok(str_buf," ");
              while(ptr != NULL)
              {
                  switch(sensor_idx)
                  {
                      case 0:
                          ambient = atoi(ptr);
                          sensor_idx++;
                          break;
                      case 1:
                          sound = atoi(ptr);
                          sensor_idx++;
                          break;
                      case 2:
                         temp = atoi(ptr);
                         sensor_idx++;
                         break;
                      case 3:
                         humi = atoi(ptr);
                         sensor_idx++;
                         break;
                      default:
                          //printf("error data\n");    
                          break;
                  }
                  ptr = strtok(NULL," ");
              }
          // reset parameter 
          if(sensor_idx == 4)
          {
            idx = 0;
            sensor_idx = 0;
            memset(str_buf,'0',20);
          }  
          else
          {
            idx = 0;
            sensor_idx = 0;
            memset(str_buf,'0',20);
          }  
      }
      else
      {
        //printf("idx: %d, %c\n",idx,newChar);
        str_buf[idx] = newChar;
        idx++;
      } 
    }  
  }

 return 0;
}

int main(void) 
{
    //
    int x = piThreadCreate(waitingThread);
    if(x != 0)printf("It didn't start\n");
    
    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);

    //initiallize the server
    UA_ServerConfig config = UA_ServerConfig_standard;
    UA_ServerNetworkLayer nl = UA_ServerNetworkLayerTCP(UA_ConnectionConfig_standard, 16664);
    config.networkLayers = &nl;
    config.networkLayersSize = 1;
    UA_Server *server = UA_Server_new(config);
    s = server; /* required for the constructor */

    //create nodes from nodeset
    addTempInstance(server);
    addSoundInstance(server);
    addAmbientInstance(server);
   
    //update
    addAmbientCallbackVariable(server);
    addSoundCallbackVariable(server);
    addTempCallbackVariable(server);
    addHumiCallbackVariable(server);
    
    // start server
    UA_Server_run(server, &running);

    UA_Server_delete(server);
    nl.deleteMembers(&nl);
    return 0;
}