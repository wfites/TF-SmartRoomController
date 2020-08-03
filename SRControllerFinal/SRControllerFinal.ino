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
#include <Encoder.h>  // library for encoder objects
#include <Adafruit_NeoPixel.h>  // library for neo pixel ring objects
#include "colors.h" //file for neo pixel colors

#include <Ethernet.h>
#include <mac.h>  // Supplies IoT lab internal mac address  for PC to access Hue lamps & wemo outlets
#include <wemo.h> // library controlling 4 wemo outlets 
#include <hue.h>  // library controlling 4 Philips Hue bulbs
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

const int inPin23 = 23; // declare pin# for YELLOW button 
const int inPin14 = 14; // declare pin# for BLUE button 
int wemoCt=-1; /count for consecutive clicking thru wemo outlet devices
int wemo;
/*
 * BME-OLED sensor variables 
 */
bool BMEstatus;
bool BBdevEnabled=false; // intially turn off all devices associated with the blue button
float temp;
float lastTemp;
float prs;
float hum;

// HUE BULB variables
int mapHueColor;
const int indoorLoTemp=65;
const int indoorHiTemp=90;
const int hueCoolColor=6; // Hue.h: Violet "cool" color
const int hueWarmColor=0; // Hue.h: Red "warm" color
const int hueBrightness=127;  // set to mid-range brightness
// end HUE BULB variables

//*** ENCODER/NEOPIXEL CONTROLLER variables
const int encPinA2=2;
const int encPinB3=3;
const int encPinR20=20;
const int encPinG21=21;
const int encPinSw22=22;
const int pixelPin17=17; 
const int lastPixel=11;
const int maxEncPos=95;
const int firstBulb=1;
const int lastBulb=5;
const int totPixels=12;

int encPos=0;
int lastEncPos=999; 
int mapPixVal;
int hueBulb;
bool redPin=true;
bool greenPin=false;
bool lightCtrlON=false;
bool encBtnState;
//*** end ENCODER/NEOPIXEL variables

/*
 * Declare object"display" accessing Adafruit_SSD1306 library for OLED 
 * display device.
 */
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);  

// Define bme as the I2C interface to the BME280 sensor
Adafruit_BME280 bme; 

OneButton yellowBtn(inPin23, false); // INPUT pin# for yellow button
OneButton blueBtn(inPin14, false); // INPUT pin# for blue button

Encoder myEnc(encPinA2,encPinB3);  //params: 2 pins as INPUT pins on Teensy
Adafruit_NeoPixel pixels(totPixels, pixelPin17, NEO_GRB + NEO_KHZ800);

//***** end HEADER section  *****

void setup()
{
  /*
   * S1) Declare yellow & blue button click methods from Onebutton object
   */
  yellowBtn.attachClick(yellowClick);
  yellowBtn.setClickTicks(500);   
  blueBtn.attachClick(blueClick);
  blueBtn.setClickTicks(500);   
  /*
  * S2) Declare SSD1306 monochrome oled screen: Execute method .begin to set up the OLED display on I2C device
  */
  Serial.begin(9600);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.display(); // Turn on OLED display for Adafruit display software
  delay(2000); // Display Adafruit logo screen for 2 seconds to verify it's working
  display.clearDisplay();
  display.display();
  /*
  * S3) BME 280sensor : Use method .begin for bme object to turn on sensor 
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
  /*
   * S4) Open up the local Ethernet port for my PC to access wemo outlets & Hue bulbs.
   * Print my local IP address.
   */
  Serial.begin(9600);
  Ethernet.begin(mac);
  delay(1000);
  Serial.println("connecting...");

  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print("."); 
  }
  Serial.println();
  Serial.println("Smart Room Controller");
  /*
   * S5) Declare the neopixels & associated attributes. Turn on the serial monitor.
   */
  pixels.begin();  // declare pixels 
  pixels.setBrightness(hueBrightness);  // set the brightness
  pixels.show(); // Initialize pixels all off
  Serial.begin(9600);

/*
 * Declare pin modes for encoder (for digitalRead)
 */
  pinMode(encPinR20, OUTPUT);
  pinMode(encPinG21, OUTPUT);
  pinMode(encPinSw22,INPUT_PULLUP);
}

void loop()
{
/*  
 * Control the yellow button via functions yellowBtn.tick() and yellowBtn(). 
 * These two functions activate the yellow button and control its functioning.
 * 
 */
  yellowBtn.tick();
  yellowBtnAction();  // Function code: FM1
/*
 * Control the blue button via functions blueBtn.tick()and blueBtnAction(). 
 * These two functions activate the blue button and control its functioning, which is 
 * associated with the "automatic" programming of associated devices to turn on
 * when enabled, such as the BME 280 sensor, I2C OLED display and the Hue lamp bulbs. The
 * enabling/disabling of such devices is controlled by the blue button.
 * 
 */
  blueBtn.tick();
  if (BBdevEnabled)
  {
    Serial.printf("DEVICES ARE ACTIVE \n");
    blueBtnAction();   // Function code: FM2
  }
  else
  {
//    Serial.printf("DEVICES ARE N O T ACTIVE \n");
    // Turn off the OLED display
    display.clearDisplay();
    display.display();    
    /*
     * DEACTIVATED BLUE BUTTON: When blue button processing is finished deactivating,
     * press the encoder button to activate it.
     */
    ActivateEncoder(); // Function code: FM3
  }

} // *****E N D loop

/*
 * FM1) Y E L L O W  B U T T O N  CODE
 */

void yellowClick()
{
  wemoCt++;
  Serial.printf("function YELLOW click: wemoCt> %i \n",wemoCt);
return;  
}

void yellowBtnAction()
{
  if ((wemoCt>=0)&&(wemoCt<=3))
  {
  Serial.printf("function yellowBtnAction: wemoCt> %i \n",wemoCt);
    wemo=wemoCt;
    switchON(wemo);    
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
 * FM2) B L U E  B U T T O N  CODE
 */
void blueClick()
{
  BBdevEnabled=!BBdevEnabled;
  Serial.printf("function BLUE click BBdevEnabled: >%i\n",BBdevEnabled);  
}
void blueBtnAction()
{
    temp = (bme.readTemperature() * 9.0 / 5.0) + 32.0;
    prs = (bme.readPressure() / 100.0F * 0.02953)+5.0; // Adjust to conventional pressure by adding 5%
    hum = bme.readHumidity();
    /*
     * 8/1: Per BR, display temps to OLED display, map & assign mapped Hue lamp
     * colors only if there is 1/2 degree temperature change.OLED displays. This reduces
     * inadvertant overprocessing by the Teensy of calls to turn on Hue bulbs.
     */
    if (abs(temp - lastTemp) > 0.5) 
    {
      BBPrintSensorValues(temp,prs,hum);

      BBdisplayToOLED(temp,prs,hum);
      /*
       * Map the current temperature to the associated hue lamp bulb color. A lower
       * temperature maps with a cooler hue bulb color; warmer temps map with a warmer
       * color.
       */
      mapHueColor = map(temp,indoorLoTemp,indoorHiTemp, hueCoolColor, hueWarmColor);
      for (int i=0; i<5; i++)
      {
        setHue(i, true, HueRainbow[mapHueColor], hueBrightness);
      }
      lastTemp=temp;
    }
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
/*
 * FM3) E N C O D E R  B U T T O N  CODE
 */
void ActivateEncoder()
{
  encBtnState=digitalRead(encPinSw22);
//  Serial.printf("Encoder button State>%i lightCtrlON>%i \n",encBtnState,lightCtrlON);
  /*
   * BUTTON PRESSED: When pressing the encoder button, toggle the enable/disable 
   * light control modes to the opposite of the button's current value (disable to 
   * enable/ enable to disable). 
   * 
   */
  if (encBtnState==LOW)
  {
    Serial.printf("**ActivateEncoder** Encoder LED button PRESSED lightCtrlON>%i \n", lightCtrlON);
    lightCtrlON=!lightCtrlON;
  }
  else
  // Button not pressed 
  {  
//    Serial.printf("**ActivateEncoder**Encoder LED button N O T pressed lightCtrlON>%i \n",lightCtrlON);
    digitalWrite(encPinR20, HIGH);
    digitalWrite(encPinG21, LOW);
  }
  /*
   * CHECK CURRENT OPERATING MODE: 
   * MANUAL mode: Set the encoder pins to light the encoder GREEN. Don't enable 
   * light operation of associated devices.
   * LIGHT CONTROL MODE: Set the encoder pins to light the encoder GREEN. Include logic
   * to enable automatic operation of neopixels and Hue bulbs lamps.
   */
  if (lightCtrlON)
    {
      Serial.printf("GREEN LIT \n"); 
      digitalWrite(encPinR20,LOW );
      digitalWrite(encPinG21,HIGH);      
      /*
       * LIGHT CONTROL MODE: Once enabled, dial the encoder to light up both the Hue 
       * lamp bulbs and the neopixels one at a time simultaneusly. 
       */
      encPos=myEnc.read();
      if (encPos<0)
        myEnc.write(maxEncPos);
      if (encPos>maxEncPos)
        myEnc.write(0);
      hueBulb=map(encPos,0,maxEncPos,firstBulb,lastBulb);
      Serial.printf(" HUE BULB> %i \n",hueBulb);
      /*
       * Program the encoder to perform the mapping only when the encoder position 
       * changes.
       */
      if (lastEncPos!=hueBulb)
      {
      /* 
       * 1)TURN ON HUE LAMP: Map the current encoder position to the corresponding
       * Hue bulb. When just starting to dial the encoder, this should map to the 
       * first Hue bulb to light up.
       */
        setHue(hueBulb, true, HueRed, hueBrightness);
          // Turn OFF all other Hue lamps
          for (int i=firstBulb; i<=lastBulb; i++)
          {
            if (i !=hueBulb) // Turn off all other Hue bulbs
            {
              setHue(i,false,0,0);
            }
          }
        /*
         * 2) TURN ON THE NEO PIXEL: Similarly, map the current position Hue bulb to 
         * the corresponding Neopixel. When just starting to dial the encoder, this 
         * should also map to the one of the first neopixels in the ring.
         */
        mapPixVal=map(hueBulb,firstBulb,lastBulb,0,lastPixel);
//        Serial.printf("Encoder position >%i Scaled Pixel#> %i \n",encPos,mapPixVal);
        pixels.setPixelColor(mapPixVal, red);
        pixels.show(); // show the single pixel
        delay(100); // is this delay problematic??
        pixels.clear(); // clear the single pixel
//        pixels.show();
      } // end if (lastEncPos!=encPos)
      lastEncPos=hueBulb;
    } // end if (lightCtrlON)
   else
    // set pins to light up a RED encoder button without any lighting control
    {
      Serial.printf("LIGHT CONTROL NOT ON RED LIT \n");
      digitalWrite(encPinR20, HIGH);
      digitalWrite(encPinG21, LOW);      
    }  
  return;    
}
