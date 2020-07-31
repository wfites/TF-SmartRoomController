/*
 * Project: SRControllerV2
 * Description: Control smart lighting switch from a button. Program to controls
 * multiple devices connected to Wemo outlets.
 * Author: Ted Fites
 * Date: 7/20/20
 * 
 * Current IoT lab WEMO devices: 
 * 0: Lava Lamp
 * 1: coffee pot
 * 2: round fan   
 * 3: rectangular fan
 * 
 * Make wemo device selections from above from 4 wemo outlets 
 * in the IoT lab
 */

// ***** HEADER section *****

#include <Ethernet.h>
#include <mac.h>  // Supplies IoT lab internal mac address  for PC to access Hue lamps & wemo outlets
#include <wemo.h> 
#include <OneButton.h> // library for One Button library
#include <Wire.h>   // library for BMP280 sensor for getting sensor readings
#include <SPI.h>  // library used for ADAFRUIT_SSD1306 for object "display"
#include <Adafruit_BME280.h>    //INSTALLED 7/26/20 supplies Adafruit_BME280 object class for instance "bme"
#include <Adafruit_GFX.h>     //library used for ADAFRUIT_SSD1306 for object "display"
#include <Adafruit_SSD1306.h>  //INSTALLED 7/25/20: supplies ADAFRUIT_SSD1306 object class for instance "display"
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels, 128*32 allows for display of 4096 characters
#define OLED_RESET     4 // Reset pin # (this is defined and needed in the next line, but not used)

int inPin23 = 23; // declare pin# for YELLOW button 
int inPin22 = 22; // declare pin# for BLUE button 
/*
 * Wemo outlet variables
 */
int wemoCt=-1;
int wemo;
/*
 * BME-OLED sensor variables
 */
const int OLEDdelayTime = 12000;  // OLED screen on 12 second delay
bool BMEstatus;
float temp;
float prs;
float hum;

bool BBdevActive=true;
/*
bool btnRead23; // declare variable used for FIRST button for FIRST device 
bool state23 = true; // declare variable used for FIRST button for FIRST device 
*/
/*
 * Declare object"display" accessing Adafruit_SSD1306 library for OLED 
 * display device.
 */
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);  

// Define bme as the I2C interface to the BME280 sensor
Adafruit_BME280 bme; 

//OneButton yellowBtn(inPin23, false); // INPUT pin# for yellow button
OneButton blueBtn(inPin22, false); // INPUT pin# for blue button

//***** end HEADER section  *****

void setup()
{
  /*
   * 1) Declare yellow & blue button click methods from Onebutton object
   */
//  yellowBtn.attachClick(yellowClick);
//  yellowBtn.setClickTicks(500);   
  blueBtn.attachClick(blueClick);
  blueBtn.setClickTicks(500);   

  /*
  * 2) Declare SSD1306 monochrome oled screen: Execute method .begin to set up the OLED display on I2C device
  */
  Serial.begin(9600);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.display(); // Turn on OLED display for Adafruit display software
  delay(2000); // Pause for 2 seconds

  /*
  * 3) BME 280sensor : Use method .begin for bme object to turn on sensor 
  */
    while(!Serial);    // time to get serial running
    delay (1000);
    Serial.println(F("BME280 test"));   // "F" in println statement: Reference to FLASH memory on SD card)
    BMEstatus = bme.begin(0x76);  
    if (!BMEstatus) {
        Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),HEX);
        Serial.println("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085");
        Serial.println("   ID of 0x56-0x58 represents a BMP 280,");
        Serial.println("        ID of 0x60 represents a BME 280.");
        Serial.println("        ID of 0x61 represents a BME 680.");
        while (1);
    }
    else {
      Serial.println("BME280 Up and Running");
    }
  
    Wire.begin();  // Use to display BMP 280 sensor data to OLED screen
  
  // 4) Open up the local Ethernet port for my PC to access wemo outlets & Hue bulbs
  
  Serial.begin(9600);
  Ethernet.begin(mac);
  delay(1000);
  Serial.println("connecting...");

  // print your local IP address:
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print("."); 
  }
  Serial.println();
  Serial.println("Smart Room Controller");
}

void loop()
{
/*  
 * Control the yellow button via functions yellowBtn.tick() and yellowBtn(). 
 * These two functions activate the yellow button and control its functioning.
 * 
 */
//  yellowBtn.tick();
//  yellowBtnAction();   
/*
 * Control the blue button via functions blueBtn.tick()and blueBtn(). 
 * These two functions activate the blue button and control its functioning.
 * 
 */
  blueBtn.tick();
  if (BBdevActive)
  {
    Serial.printf("DEVICE IS ACTIVE \n");
    blueBtnAction();   
  }
  else
  {
    Serial.printf("DEVICE IS N O T ACTIVE \n");
    display.clearDisplay();
    display.display();
  }

} // *****E N D loop

/*
 * Y E L L O W  B U T T O N  CODE
 */

void yellowClick()
{
//  state23=!state23; // if clicked turning ON LED (HIGH voltage), then reset to OFF (LOW voltage) turning OFF light 
  wemoCt++;
  Serial.printf("function YELLOW click: wemoCt> %i \n",wemoCt);
return;  
}

void yellowBtnAction()
{
  if ((wemoCt>=0)&&(wemoCt<=3))
  {
//  Serial.printf("function yellowBtnAction: wemoCt> %i \n",wemoCt);
    wemo=wemoCt;
//    switchON(wemo);    
  }
  if (wemoCt==4)
  {
  Serial.printf("function yellowCLICK: wemoCt> %i \n",wemoCt);
    for (int i=3; i>=0; i--)
    {   
      wemo=i;
      switchOFF(wemo);
    }
    wemoCt=-1;
  }
return;
}

/*
 * B L U E  B U T T O N  CODE
 */
void blueClick()
{
  BBdevActive=!BBdevActive;
  Serial.printf("function BLUE click BBdevActive: >%i\n",BBdevActive);  
}
void blueBtnAction()
{
    temp = (bme.readTemperature() * 9.0 / 5.0) + 32.0;
    prs = (bme.readPressure() / 100.0F * 0.02953)+5.0; // Adjust to conventional pressure by adding 5%
    hum = bme.readHumidity();
    BBPrintSensorValues(temp,prs,hum);
    BBdisplayToOLED(temp,prs,hum);
    return;
}

/*
 * Function BBPrintSensorValues: Display sensor reading to the serial monitor 
 * for quick visual check.
 */
void BBPrintSensorValues(float Ptemp, float Pprs, float Phum)
{ 
    Serial.print("Temperature = ");
    Serial.print(Ptemp);
    Serial.println(" *F");

    Serial.print("Pressure = ");
    Serial.print(Pprs);
    Serial.println(" inHg");

    Serial.print("Humidity = ");
    Serial.print(Phum);
    Serial.println(" %");

    Serial.println();
    return;
}  

/*
 * Function BBdisplayToOLED: Display sensor readings to the I2C OLED screen 
 */
void BBdisplayToOLED(float Otemp, float Oprs, float Ohum) 
{
  Serial.println("BBdisplayToOLED: BEGIN PRINTING TO OLED**********");  

  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
//  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
  display.setCursor(0,0);              // LINE #0: Start at top-left corner

  display.printf("BME280 Temp(F): %5.2f\n",Otemp);  // LINE #0 TEMPERATURE: Start at top-left corner then define line

  display.printf("Prs(inHg): %5.2f\n",Oprs);       // LINE #1 AIR PRESSURE: Start at top-left corner then define line

  display.printf("Hum(%): %5.2f\n",Ohum);          // LINE #2 REL HUMIDITY: Start at top-left corner then define line
  
  display.display();                    // AFTER setting up all display lines: PRINT to OLED screen
//  delay(OLEDdelayTime);    // 12 second delay causing execution problems, so disable
  return;
}
