// 
//   FILE:  dht11_test1.pde
// PURPOSE: DHT11 library test sketch for Arduino
//
#include <dht11.h>
dht11 DHT;
#define DHT11_PIN 4 //digital
#define SOUND_PIN 5 // analog
#define AMBIENT_PIN 0 // analog

char str_buf[20];

void setup(){
  Serial.begin(9600);
  //Serial.println("**ambient, sound, temp, humi**");
  //Serial.println("Type,\tstatus,\tHumidity (%),\tTemperature (C)");
}



void loop(){
  // light check
  int ambient = 0;
  ambient = analogRead(AMBIENT_PIN);
  
  // sound check
  int sound = 0;
  sound = analogRead(SOUND_PIN);
  
  //Temp Check--
  int chk;
  int temp = 0;
  int humi = 0;
  chk = DHT.read(DHT11_PIN);    // READ DATA
  switch (chk)
  {
    case DHTLIB_OK:  
                //Serial.print("OK,\t"); 
                break;
    case DHTLIB_ERROR_CHECKSUM: 
                //Serial.print("Checksum error,\t"); 
                break;
    case DHTLIB_ERROR_TIMEOUT: 
                //Serial.print("Time out error,\t"); 
                break;
    default: 
                //Serial.print("Unknown error,\t"); 
                break;
  }
 // DISPLAT DATA
//Serial.println(ambient);
//Serial.println(sound);
temp = (int)DHT.temperature;
humi = (int)DHT.humidity;
//Serial.println(temp); 
//Serial.println(humi);

// assemble data
memset(str_buf,'0',20);
sprintf(str_buf,"%d %d %d %d #",ambient,sound,temp,humi);
Serial.println(str_buf);

delay(200);
}


