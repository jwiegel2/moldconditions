#include <SimpleDHT.h>
#include <rgb_lcd.h>
#include <Wire.h>
#include <signal.h>

// for DHT11, 
//      VCC: 5V or 3V
//      GND: GND
//      DATA: 2
int pinDHT11 = 2;
SimpleDHT11 dht11;
rgb_lcd lcd;

byte tempArray[30];
byte humidArray[30];
int* passiveTests;
unsigned long oldMillis;
int man_nPassive = 0;

const int buttonPin = 5;
int buttonState = 0;

const int buzzerPin = 6;
int buzzerState = 0;

int colorR = 255;
int colorG = 0;
int colorB = 0;

void printResult(int avgTemp, int avgHumid, int passed);
int checkMoldy(int tmp, int hum);
int convertFahrenheit(int celcius);
int* calcAverages();
void manualTest();
int* collectData();


void setup() {
  Serial.begin(9600);
  pinMode(buttonPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  lcd.begin(16, 2);
  colorB = 255;colorR = 0; colorG = 0;
  lcd.setRGB(colorR, colorG, colorB);
  lcd.print("Collecting Data:");
  delay(500);
  oldMillis = millis();
}


void loop() {
  unsigned long currMillis = millis();
  if((currMillis - oldMillis) > 2000 && (man_nPassive == 0)){
    lcd.setCursor(0,0);lcd.print("Passive Testing ");
    lcd.setCursor(0,1);lcd.print("                    ");
    delay(100);
    oldMillis = currMillis;
    man_nPassive = 1;
  }
  else if((currMillis - oldMillis) > 2000 && (man_nPassive == 1)){
    lcd.setCursor(0,0);lcd.print("Press Button    ");
    lcd.setCursor(0,1);lcd.print("for Manual Test ");
    delay(100);
    oldMillis = currMillis;
    man_nPassive = 0;
    
  }
  buttonState = digitalRead(buttonPin);
  if(buttonState == HIGH){
    manualTest();
  }
  passiveTests = collectData();  
  passiveTests[0] = convertFahrenheit(passiveTests[0]);

  delay(200);
  if(checkMoldy(passiveTests[0], passiveTests[1]) == 1){
  delay(1000);
    beepFlash();
  } 
  buttonState = digitalRead(buttonPin);
  if(buttonState == HIGH){
    manualTest();
  }
  free(passiveTests);


}

void beepFlash(){
  lcd.setCursor(0,0);lcd.print("beep              ");
  colorB = 0;
  colorG = 0;
  lcd.setCursor(0,0);
  lcd.print("Potential        ");
  lcd.setCursor(0,1);
  lcd.print("Mold Hazard     ");
  for(int i = 0; i < 3; i++){
    colorR = 255;
    lcd.setRGB(colorR, colorG, colorB);
    digitalWrite(buzzerPin, HIGH);
    delay(500);
    colorR = 128;
    lcd.setRGB(colorR, colorG, colorB);
    digitalWrite(buzzerPin, LOW);
    delay(500);
  }
}

int* collectData(){
  
   // read with raw sample data.
  byte temperature = 0;
  byte humidity = 0;
  byte data[40] = {0};
  if (dht11.read(pinDHT11, &temperature, &humidity, data)) {
    Serial.print("Read DHT11 failed");
    return NULL;
  }
  int *collectedData = (int*) malloc(sizeof(int)*2);
  collectedData[0] = (int) temperature;
  collectedData[1] = (int) humidity;
  return collectedData;
  
}

void manualTest(){

  lcd.setCursor(0,0);
  lcd.print("Testing...      ");
  
  for(int i = 0; i < 30; i++){
    lcd.setCursor(0,1);
    lcd.print(30 - i); lcd.print(" sec remaining ");
    
  
  byte data[40] = {0};
    if (dht11.read(pinDHT11, &tempArray[i], &humidArray[i], data)) {
    Serial.print("Read DHT11 failed");
    return;
  }
  delay(1000);
}
  
    int *averages = calcAverages();
   
    averages[0] = convertFahrenheit(averages[0]);

  int passed = 0;
  if(checkMoldy(averages[0], averages[1]) != 1)
    passed = 1;
  printResult(averages[0], averages[1], passed);
  free(averages);
  
  lcd.setCursor(0,0);
  lcd.print("Passive Testing ");
  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setRGB(0, 0, 255);
  for(int i = 0; i <5; i++){
    lcd.setCursor(0,1);
    lcd.print("Resumes in: ");lcd.print(5-i);
    delay(1000);
  }
}


int* calcAverages(){
  int *avg = (int *) malloc(sizeof(int)*2);
  int totalTemp = 0;
  int totalHumid = 0;
  for(int i = 0; i  < 30; i++){
    totalTemp += (int)tempArray[i];
    totalHumid += (int)humidArray[i];
  }
  avg[0] = (totalTemp/30);
  avg[1] = (totalHumid/30);
  return avg;
}

int convertFahrenheit(int celcius){
  int fahrenheit = celcius * 1.8 + 32;
  return(fahrenheit); 
}

int checkMoldy(int tmp, int hum){
    if(tmp >= 60 && tmp <= 80 && hum >= 70){
      return 1;
    }else{
      return 0;
    }
}

void printResult(int avgTemp, int avgHumid, int passed){
  if (passed){
    colorG = 255;
    colorR = 0;
    colorB = 0;
    lcd.setRGB(colorR, colorG, colorB);
    lcd.setCursor(0,0);
    lcd.print("Test Passed     ");
    lcd.setCursor(0,1);
    lcd.print("No Hazard       ");
  } else {
    colorR = 255;
    colorG = 0;
    colorB = 0;
    lcd.setRGB(colorR, colorG, colorB);
    lcd.setCursor(0,0);
    lcd.print("Test Failed     ");
    lcd.setCursor(0,1);
    lcd.print("Mold Hazard     ");
  }
  delay(2000);
  lcd.setCursor(0,0);
  lcd.print("Temp:     ");lcd.print(avgTemp); lcd.print("*F  ");
  lcd.setCursor(0,1);
  lcd.print("Humidity: ");lcd.print(avgHumid); lcd.println(" %  ");
  delay(5000);
}

