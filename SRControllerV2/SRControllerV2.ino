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

// HEADER section
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

int inPin23 = 23; // declare pin# for FIRST button 
int inPin22 = 22; // declare pin# for SECOND button 
int wemoCt=-1;
int wemo;
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

OneButton yellowBtn(inPin23, false); // INPUT pin# for yellow button
OneButton blueBtn(inPin22, false); // INPUT pin# for blue button
// end HEADER section

void setup()
{
  yellowBtn.attachClick(click);
  yellowBtn.setClickTicks(500);   
  blueBtn.attachClick(click);
  blueBtn.setClickTicks(500);   
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
  yellowBtn.tick();
  yellowBtnAction();   
/*
 * Control the blue button via functions blueBtn.tick()and blueBtn(). 
 * These two functions activate the blue button and control its functioning.
 * 
 */
  blueBtn.tick();
//  blueBtnAction();   

} // end loop

void click()
{
//  Serial.printf("function CLICK: State > %i \n",state23);
//  state23=!state23; // if clicked turning ON LED (HIGH voltage), then reset to OFF (LOW voltage) turning OFF light 
  wemoCt++;
return;  
}

void yellowBtnAction()
{
  if ((wemoCt>=0)&&(wemoCt<=3))
  {
    wemo=wemoCt;
    switchON(wemo);    
  }
  if (wemoCt==4)
  {
    for (int i=3; i>=0; i--)
    {   
      wemo=i;
      switchOFF(wemo);
    }
    wemoCt=-1;
  }
return;
}
