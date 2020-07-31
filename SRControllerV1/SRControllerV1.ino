/*
 * Project: L08_01_Wemo
 * Description: Control smart lighting switch from a button. Program to controls
 * multiple devices connected to Wemo outlets.
 * Author: Ted Fites
 * Date: 7/20/20
 */
/*
 Connect Ethernet port per Fritzing diagram and a button to Pin 23 (with pulldown resistor).
 The functions switchOn(wemo) and switchOff(wemo) take an int and turn on the corresponding 
 Wemo in the classroom.
 Please make sure to move the wemo library folder into your Arduino libraries directory.
*/
// HEADER section
#include <Ethernet.h>
#include <SPI.h>
#include <mac.h>
#include <wemo.h> 
#include <OneButton.h>

int inPin23 = 23; // declare pin# for FIRST button for FIRST device 
int wemoCt=-1;
int wemo;
bool btnRead23; // declare variable used for FIRST button for FIRST device 
bool state23 = true; // declare variable used for FIRST button for FIRST device 
/*
 * Current IoT lab WEMO devices: 
 * 0: Lava Lamp
 * 1: coffee pot
 * 2: round fan   
 * 3: rectangular fan
 * 
 * Make wemo device selections from above from 4 wemo outlets 
 * in the IoT lab
 */

OneButton button1(inPin23, false);
// end HEADER section

void setup()
{
 button1.attachClick(click);
 button1.setClickTicks(500); 

//  pinMode(inPin23, INPUT); // declare pin mode for FIRST wemo device
  
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
  Serial.println("Lesson L08_01_Wemo");
}

void loop()
{
  button1.tick();
  yellowBtn();   

/*
 * Control FIRST device connected to Wemo outlet
 * by pressing the dedicated button (pin #23)to 
 * power the device on. Release the button to turn 
 * the device back off.
*/     

} // end loop

void click()
{
  Serial.printf("function CLICK: State > %i \n",state23);
  state23=!state23; // if clicked turning ON LED (HIGH voltage), then reset to OFF (LOW voltage) turning OFF light 
  wemoCt++;
return;  
}

void yellowBtn()
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
