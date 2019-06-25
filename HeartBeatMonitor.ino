#include <MAX30100.h>
#include <MAX30100_BeatDetector.h>
#include <MAX30100_Filters.h>
#include <MAX30100_PulseOximeter.h>
#include <MAX30100_Registers.h>
#include <MAX30100_SpO2Calculator.h>
#include <Wire.h>

#include <Adafruit_SH1106.h>

#define OLED_RESET 4
Adafruit_SH1106 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
#define REPORTING_PERIOD_MS     500

PulseOximeter pox;

const int numReadings=10;
float filterweight=0.5;
uint32_t tsLastReport = 0;
uint32_t last_beat=0;
int readIndex=0;
int average_beat=0;
int average_SpO2=0;
bool calculation_complete=false;
bool calculating=false;
bool initialized=false;
byte beat=0;


void onBeatDetected() //Calls back when pulse is detected
{
  viewBeat();
  last_beat=millis();
}

void viewBeat() 
{

  if (beat==0) {
    beat=1;
  } 
  else
  {
   Serial.print("^");
    beat=0;
  }
}

void initial_display() 
{
  if (not initialized) 
  {
    viewBeat();  
    initialized=true;
  }
}

void display_calculating(int j){

  viewBeat();
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(30, 30);
  display.println("Measuring...");
  display.display();
}

void display_values()
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("Bpm: ");
  display.setCursor(50,0);
  display.println(average_beat);
  display.setCursor(0,24);
  display.print("SpO2:%");
  display.setCursor(80,24);
  display.println(average_SpO2);
  display.display();
}

void calculate_average(int beat, int SpO2) 
{
  if (readIndex==numReadings) {
    calculation_complete=true;
    calculating=false;
    initialized=false;
    readIndex=0;
    display_values();
  }
  
  if (not calculation_complete and beat>30 and beat<220 and SpO2>50) {
    average_beat = filterweight * (beat) + (1 - filterweight ) * average_beat;
    average_SpO2 = filterweight * (SpO2) + (1 - filterweight ) * average_SpO2;
    readIndex++;
    display_calculating(readIndex);
  }
}

void setup()
{
    pox.begin();
    pox.setOnBeatDetectedCallback(onBeatDetected);
    display.begin(SH1106_SWITCHCAPVCC, 0x3C);
    // Clear the buffer.
    delay(1000);
    display.clearDisplay();
    display.display();
    // draw a single pixel
}


void loop()
{
    pox.update(); 
    if ((millis() - tsLastReport > REPORTING_PERIOD_MS) and (not calculation_complete)) {
        calculate_average(pox.getHeartRate(),pox.getSpO2());
        tsLastReport = millis();
        
    }
    if ((millis()-last_beat> 3000)) {
      calculation_complete=false;
      average_beat=0;
      average_SpO2=0;
      initial_display();
    }
 
}
