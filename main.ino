#include "DHT.h"
#include <SD.h>
#include <DFRobot_DS1307.h>

File myFile;
const int CSPin = 10;

const int cellPin = A5;

const int pump = 5;
const int solarCell = A0;
const int soilPin = A1;
const int DHTPIN = 6;

int light ;
int humid ;
int temp ;
int soil ;
int need_water ;
int LL ;
int LC ;
int LV ;

int need_humid ;
int need_heat ;
int need_light ;

int old_humid ;
int new_humid ;
int old_temp ;
int new_temp ;
int old_light ;
int new_light ;
int old_water ;
int new_water ;

uint16_t getTimeBuff[7] = {0};

#define DHTTYPE DHT11
DFRobot_DS1307 RTC;

DHT dht(DHTPIN, DHTTYPE);






void setup() {
  
  pinMode(pump, OUTPUT);
  pinMode(soil, INPUT);
  Serial.begin(115200);
  dht.begin();

  Serial.print("Initializing SD card...");
  pinMode(CSPin, OUTPUT);
  pinMode(cellPin, INPUT);

  if (!SD.begin(CSPin)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  while(!(RTC.begin())){
    Serial.println("Communication with RTC failed");
    delay(3000);
  }
  Serial.println("RTC connected");
  RTC.setSqwPinMode(RTC.eSquareWave_1Hz);

}

void loop() {

  
  //float pumping = digitalRead(pump);
  //Serial.println(pumping);

  get_data();
  //soil is dry if between 0-300 is good between 300-700 and is too wet 700+
  //%50-%60 humidity is good for plants
  //the ideal range of temperature is 13c-24c 
  //
  use_data();
  RTC.getTime(getTimeBuff);
  if (((getTimeBuff[0]%59) == 0)&((getTimeBuff[1]%59) == 0)){
    record_data();
    delay(1000);
  }

  if((getTimeBuff[0]%10)==0){
    record_data();
    delay(1000);
  }
  
}

void conversion(float sensorValue){

   float voltage = (float)sensorValue * 5 / 1023;
   Serial.print(voltage);
   Serial.println("v");
   
}

void get_data(){
  float light = analogRead(solarCell);
  //Serial.print("light lux = ");
  float LV = (float)light * 5/1023;
  float LC = ((float)LV/47000) * 100000;
  float LL = (float)LC/0.075;
  //Serial.println(LL);

  float humid = dht.readHumidity();
  float temp = dht.readTemperature(); 
  //Serial.print("humidity = ");
  //Serial.print((float)humid,1);
  //Serial.println("%");

  //Serial.print("Temperature = ");
  //Serial.print((float)temp,1);
  //Serial.println("C");

  float soil = analogRead(soilPin);
  //Serial.print("soil moisture = ");
  //Serial.println(soil);

  //Serial.println("");
}

void use_data(){

  if (soil <= 300){
    need_water = 1;
  } else if (soil > 500){
    need_water = 0;
  }

  if (need_water == 1){
    digitalWrite(pump, HIGH);
    //Serial.println("watering the plant");
  }else{
    digitalWrite(5,LOW);
  }

  if (humid <= 50) {
    //Serial.println("the air is too dry");
    need_humid = 1;
  }else if(humid>=60){
    //Serial.println("the air is too humid");
    need_humid = 3;
  }else{
    need_humid = 2;
  }


  if (temp<=13){
    //Serial.println("the temperature is too cold");
    need_heat = 1;
  }else if (temp>=24){
    //Serial.println("the temperature is too hot");
    need_heat = 3;
  }else{
    need_heat = 2;
  }
  //light level that is good is between 7000-50,000

  if(LL<7000){
    need_light = 1;
  } else if (LL>=50000) {
    need_light = 3;
  }else{
    need_light = 2;
  }


  int new_water = need_water;
  int new_humid = need_humid;
  int new_temp = need_heat;
  int new_light = need_light;

  if (!(new_water == old_water)){
    record_data();
  }
  if (new_humid != old_humid){
    record_data();
  }
  if (new_temp != old_temp){
    record_data();
  }
  if (new_light != old_light){
    record_data();
  }

  old_water = new_water;
  old_humid = new_humid;
  old_temp = new_temp;
  old_light = new_light;


}

void record_data(){
writeOut();
Serial.println("");
}

// Adjusted to get one data point in, you can add more data as you add more sensors
void writeOut(){
  myFile = SD.open("plantLog.txt", FILE_WRITE);

  if(myFile){
    Serial.println("Trying to write!");

    // Timestamp
    RTC.getTime(getTimeBuff);
    char outputarr[128];
                      //8/5/2025,10:7:26,
    sprintf(outputarr, "%d/%d/%d,%d:%d:%d,", 
                        getTimeBuff[6],
                        getTimeBuff[5],
                        getTimeBuff[4],
                        getTimeBuff[2],
                        getTimeBuff[1],
                        getTimeBuff[0]);
    Serial.print(outputarr);
    myFile.print(outputarr);

    Serial.println();
    myFile.println();
    Serial.print("light level: ");
    myFile.print("light level: ");
    Serial.print(light);
    myFile.print(light);
    Serial.println(",");
    myFile.println(",");
    Serial.print("temperature: ");
    myFile.print("temperature: ");
    Serial.print(temp);
    myFile.print(temp);
    Serial.println(",");
    myFile.println(",");
    Serial.print("humidity; ");
    myFile.print("humidity: ");
    Serial.print(humid);
    myFile.print(humid);
    Serial.println(",");
    myFile.println(",");
    Serial.print("soil moisture content: ");
    myFile.print("soil moisture content: ");
    Serial.print(soil);
    myFile.print(soil);
    Serial.println(",");
    myFile.println(",");

    Serial.println();
    myFile.println();

  if (new_water != old_water){
    if(new_water == 1){
      Serial.println("the pump is on");
      myFile.println("the pump is on");
    }else{
      Serial.println("the pump is off");
      myFile.println("the pump is off");
    }
  }

  Serial.println();
  myFile.println();

  if (new_humid != old_humid){
    Serial.println(new_humid);
    if(new_humid == 1){
      Serial.println("the air is too dry");
      myFile.println("the air is too dry");
    }else if(new_humid == 2){
      Serial.println("the air is the right humidity");
      myFile.println("the air is the right humidity");
    }else if(new_humid == 3){
      Serial.println("the air is too humid");
      myFile.println("the air is too humid");
    }
  }

  Serial.println();
  myFile.println();

  if (new_temp != old_temp){
      if(new_temp == 1){
      Serial.println("the temperature is too cold");
      myFile.println("the temperature is too cold");
    }else if(new_temp == 2){
      Serial.println("the temperature is good");
      myFile.println("the temperature is good");
    }else if(new_temp ==3){
      Serial.println("the temperature is too hot");
      myFile.println("the temperature is too hot");
    }
  }

  Serial.println();
  myFile.println();

  if (new_light != old_light){
    if(new_light == 1){
      Serial.println("the room is too dark");
      myFile.println("the room is too dark");
    }else if(new_light == 2){
      Serial.println("the light level is good");
      myFile.println("the light level is good");
    }else if(new_light ==3){
      Serial.println("there is too much light");
      myFile.println("there is too much light");
    }
  }
    
    Serial.println();
    myFile.println();

    myFile.close();
    Serial.println("Finished.");
  } else{
    Serial.println("Error opening text file");
  }
}
