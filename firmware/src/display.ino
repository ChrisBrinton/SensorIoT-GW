#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
//#include <Fonts/FreeMono18pt7b.h>
#include <Fonts/FreeSans24pt7b.h>
#include <Fonts/TomThumb.h>
#include <Fonts/PicoPixel.h>
#include <NtpClientLib.h>
#include "Adafruit_SSD1306.h"



//
// Display module
//

//Set up I2C for display
#define OLED_RESET 0
#define DISPLAYBUFFERSIZE 10

Adafruit_SSD1306 display(OLED_RESET);

typedef enum {
    DISPLAY_STATE_NONE = 0,
    DISPLAY_STATE_STARTUP,
    DISPLAY_STATE_WIFIINFO,
    DISPLAY_STATE_MESSAGE,
    DISPLAY_STATE_CONNECTED,
    DISPLAY_STATE_TEST
} displayStates_t;

struct _display_node_t {
	displayStates_t displayState = DISPLAY_STATE_NONE;
	uint8_t secondsToHold = 0;
};

_display_node_t displayStateBuffer[DISPLAYBUFFERSIZE];
int displayBufferIn = 0;
int displayBufferOut = 0;
String _wifiDescr, _wifiSSID, _wifiIP, _displayMsg;


void scrollUp(int numoflines){
    static int line;
    for(int x=0;x<numoflines;x++){
        line++;
        if(line >= 4){
            for (int i=0;i<8;i++){
               delay(50);
               display.ssd1306_command(SSD1306_SETSTARTLINE+i);
             }
            //delay(3000);
            //after the row has scrolled up, copy the 2nd to 4th row into the 1st to 3rd row
            memcpy(display.getBuffer(),display.getBuffer()+128,384);
            //then blank out the 4th row
            memset(display.getBuffer()+384,0,128);
            //then set the start line back to the beginning of the buffer and move the cursor to the start of the new 4th line.
            display.ssd1306_command(SSD1306_SETSTARTLINE);
            display.setCursor(0,24);
            //now it should be safe to display the buffer
            display.display();
        }
    }
}

void scrollUpDB(int numoflines){
    static int line=0;
    static int bufferline=0;
    static int row=0;
    //reset counters. yay for procedural code! j/k
    if(numoflines==-1){
        line=0;
        bufferline=0;
        row=0;
        display.ssd1306_command(SSD1306_SETSTARTLINE);
        return;
    }
    for(int x=0;x<numoflines;x++){
        line++;
        bufferline++;
        if(line > 4){
            for (int i=0;i<8;i++){
                delay(50);
                display.ssd1306_command(SSD1306_SETSTARTLINE | row % 64);
                row++;
            }
            if(bufferline >= 8) {
                bufferline = 0;
                display.setCursor(0,0);
            }
            if(row >= 64){
                row = 0;
            }
        }
        //blank out the row that we just scrolled off
        if(bufferline < 5){
            memset(display.getBuffer()+((bufferline+3)*128),0,128);
        }
        else {
            memset(display.getBuffer()+((bufferline-5)*128),0,128);
        }
    }
}

void BuildCurrentTimePage(int iYOffset) {
    bool timeDisplayTest = false;
    String timeString = "";
    String ampmString = "";
    int hours, centerx;
    hours = hour(now());
    if(hours>12){
        hours = hour(now()) - 12;
    }
    timeString = timeString + String(hours) + ":";
    if(minute(now())<10){
        timeString = timeString + "0";
    }
    timeString = timeString + String(minute(now()));
    if(hour(now())>11){
        ampmString = "PM";
    }
    else{
        ampmString = "AM";
    }
    //Using this will dummy the time display, cycling through at a second per min
    //Useful for testing font compatibility and spacing.
    if(timeDisplayTest == true) {

        timeString = "";
        static int dummyHours = 12;
        static int dummyMin = 0;
        static int ampm = 0;
        dummyMin += 1;
        if(dummyMin>59){
            dummyHours +=1;
            dummyMin = 0;
        }
        if(dummyHours>12){
            dummyHours = dummyHours -12;
            if(ampm == 1){
                ampm = 0;
            } else {
                ampm = 1;
            }
        }
        if(ampm == 0){
            ampmString = "AM";
        } else {
            ampmString = "PM";
        }
        timeString = timeString + String(dummyHours) + ":";
        if(dummyMin<10){
            timeString = timeString + "0";
        }
        timeString = timeString + String(dummyMin);
    }
    
    // do these shenanigans to determine the width of the time we want to print
    // so that we can center it on the screen
    int16_t x1,y1;
    uint16_t w,h;
    int total_w = 0;
    int widthAdjust = 0;
    //There is an issue either with the Adafruit GFX lib or with the FreeSans18pt7b font
    //where the width returned via getTextBounds is off by a pixel or two depending on the
    //trailing digit. So this little hack corrects for that. There are other pixel shifts (i.e. for digits
    //that arent trailing) but they are less visible and I havent bothered to correct for them-CB.
    if(timeString.endsWith("1")){
        widthAdjust = 6;
    } else if (timeString.endsWith("4") || timeString.endsWith("9")) {
        widthAdjust = 2;
    } else {
        widthAdjust = 0;
    }
    display.setFont(&FreeSans18pt7b);
    display.getTextBounds((char*)timeString.c_str(),0,31,&x1,&y1,&w,&h);
    total_w = w + widthAdjust;
    display.setFont(&FreeSans9pt7b);
    display.getTextBounds((char*)ampmString.c_str(),0,31,&x1,&y1,&w,&h);
    //DEBUG_MSG("Width 1 %i Width 2 %i\n", total_w, w);
    total_w = total_w + w;

    int center_offset = 17;
    //offsetting the center gives us the room we need draw the node info to the right of the time.
    centerx = ((SSD1306_LCDWIDTH - total_w - center_offset)/2)-iYOffset;
    display.setCursor(centerx,31);
    display.setFont(&FreeSans18pt7b);
    display.print(timeString.c_str());
    display.setFont(&FreeSans9pt7b);
    display.println(ampmString.c_str());
}


void buildNodeDisplayPage(int iYOffset){
    int16_t x1,y1,iTmpX,iTmpY;
    uint16_t w,h;
    uint8_t iNodeID;
    String tempString = "";
    int iF, iH, iRSSI, iPageOffset;

    iNodeID = nodeList.currentNode();
    //if the day has rolled since the last update time then the age can return a neg number
    //its probably healthy to clear out the list daily, so clear it for any neg return val.
    //Dont bother to check node SENSORID as thats the onboard sensor
    if(iNodeID != SENSORID) {
        int age = nodeList.getNodeUpdateAge(iNodeID);
        //DEBUG_MSG("Node %i is %i min old\n", iNodeID, age);
        if (age > 60 || age < 0){
          DEBUG_MSG("Node ID: %i is %i min old. Deleting\n", iNodeID, age);
          nodeList.deleteNode(iNodeID);
        }
    }

    iF = int(atof(nodeInfo[nodeList.currentNode()].THP->F.c_str())+.49);
    iH = int(atof(nodeInfo[nodeList.currentNode()].THP->H.c_str())+.49);
    iRSSI = int(atof(nodeInfo[nodeList.currentNode()].THP->RSSI.c_str())+.49);

    iPageOffset = -1;

  //ID
    iTmpX = 93;
    iTmpY = 6;
    String nickname = nodeInfo[nodeList.currentNode()].THP->shortname;
    if(nickname.length() > 0) {
        tempString = nickname;
        iTmpX = 115 - nickname.length()*4;
    } else {
        tempString = "ID: " + String(iNodeID);
    }
    display.setFont(&TomThumb);
    display.setCursor(iTmpX,iTmpY+iPageOffset);
    display.print(tempString.c_str());

  //Humidity
    iTmpX=133;
    iTmpY=22;
    tempString = "H:" + String(iH);
    display.getTextBounds((char*)tempString.c_str(),0,iTmpY+iPageOffset,&x1,&y1,&w,&h);
    display.setCursor(iTmpX-w,iTmpY+iPageOffset);
    display.print(tempString.c_str());
  //Temp
    iTmpX = 125;
    iTmpY = 14;
    display.setFont(&TomThumb);
    tempString = String(iF);
    display.getTextBounds((char*)tempString.c_str(),0,iTmpY+iPageOffset,&x1,&y1,&w,&h);
    display.setCursor(iTmpX-w+4,iTmpY+iPageOffset);
    display.print(tempString.c_str());
    iTmpX = iTmpX;
    display.setCursor(iTmpX,iTmpY-2+iPageOffset);
    tempString = "o";
    display.print(tempString.c_str());
  //RSSI (based on where temp ends)
    display.getTextBounds((char*)tempString.c_str(),iTmpX,16+iPageOffset,&x1,&y1,&w,&h);
    iTmpX = iTmpX + w + 0;
    iTmpX = 122;
    iTmpY = 1;
    if(iRSSI>-60){
        display.drawRect(iTmpX+iYOffset,iTmpY+iPageOffset,2,5,WHITE);
    }
    if(iRSSI>-70){
        display.drawRect(iTmpX-3+iYOffset,iTmpY+iPageOffset+1,2,4,WHITE);
    }
    if(iRSSI>-80){
        display.drawRect(iTmpX-6+iYOffset,iTmpY+iPageOffset+2,2,3,WHITE);
    }
    if(iRSSI>-90){
        display.drawRect(iTmpX-9+iYOffset,iTmpY+iPageOffset+3,2,2,WHITE);
    }

    //Battery indicator
    //draw battery shape around indicator
    iTmpX = 111;
    iTmpY = 26;
    display.drawRect(iTmpX+iYOffset,iTmpY+iPageOffset,12,7,WHITE);
    display.drawRect(iTmpX+12+iYOffset,iTmpY+2+iPageOffset,1,3,WHITE);

    float dBAT = atof(nodeInfo[nodeList.currentNode()].THP->BAT.c_str());
    int iBatLvl = 0;
    if(dBAT>3.5){
        iBatLvl=3;
    } else if (dBAT > 3.3){
        iBatLvl=2;
    } else if (dBAT > 3.0){
        iBatLvl=1;
    } else if (dBAT < 2.7){
      if(dBAT > 1.2){
          iBatLvl=3;
      } else if (dBAT > 1.0) {
          iBatLvl=2;
      } else if (dBAT > .9) {
          iBatLvl=1;
      }
    }

    if(iBatLvl>2){
        display.drawRect(iTmpX+8+iYOffset,iTmpY+2+iPageOffset,2,3,WHITE);
    }
    if(iBatLvl>1){
        display.drawRect(iTmpX+5+iYOffset,iTmpY+2+iPageOffset,2,3,WHITE);
    }
    if(iBatLvl>0){
        display.drawRect(iTmpX+2+iYOffset,iTmpY+2+iPageOffset,2,3,WHITE);
    }


    nodeList.nextNode();
}

//This should get called about every second or two
void displayInfo(){
    #if defined SSD1305_128_32DB //The SSD1305 displays are offset by 4 pixels
        int iYOffset = 4;
    #else
        int iYOffset = 0;
    #endif

    memset(display.getBuffer(),0,512);
    BuildCurrentTimePage(iYOffset);

    buildNodeDisplayPage(iYOffset);

    display.display();
}

//If the gateway is in AP mode, display onboard sensor info and any packets recieved by
//the radio. This will give a visual indictor on the screen that both the onboard sensor
//and the radio are functioning.
void displayTestInfo(){
    uint8_t iNodeID;
    String tempString = "";
    int iF, iRSSI;

    String status = "";
    int iStatus = WiFi.status();
    switch (iStatus) {
        case STATE_SCAN_ONGOING:
            status = "SCAN";
            break;
        case STATE_WPS_START:
            status = "C_ING";
            break;
        case STATE_WPS_SUCCESS:
            status = "CONN";
            break;
        default:
            char cStat[10];
            sprintf(cStat,"Stat: %i", iStatus);
            status = cStat;
    }

    iNodeID = nodeList.currentNode();

    iF = int(atof(nodeInfo[nodeList.currentNode()].THP->F.c_str())+.49);
    iRSSI = int(atof(nodeInfo[nodeList.currentNode()].THP->RSSI.c_str())+.49);
    float dBAT = atof(nodeInfo[nodeList.currentNode()].THP->BAT.c_str());

    tempString = "ID:" + String(iNodeID) + " T:" + String(iF) + " S:" + String(iRSSI) + " B:" + String(dBAT) + " CD:" + status;
    //clear the last line of text
    memset(display.getBuffer()+384,0,128);
    display.setFont(&TomThumb);
    display.ssd1306_command(SSD1306_SETSTARTLINE);
    display.setCursor(0,31);
    display.print(tempString.c_str());
    display.display();
    //DEBUG_MSG("[DISPLAY] TestInfo %s\n", tempString.c_str());
    nodeList.nextNode();
}

void displaySetup () {
    display.begin(SSD1306_SWITCHCAPVCC);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.setTextWrap(FALSE);

    displayStartupLogo();


}

void setDisplayState(displayStates_t newState, int secondsToHold = 0){
    if(displayBufferIn == ((displayBufferOut -1 + DISPLAYBUFFERSIZE) % DISPLAYBUFFERSIZE)) {
      return; //queue is full, ignore the request
    }

    DEBUG_MSG("[DISPLAY] Adding element to displayBufferState - state: %i secs: %i\n", newState, secondsToHold);
    displayStateBuffer[displayBufferIn].displayState = newState;
    displayStateBuffer[displayBufferIn].secondsToHold = secondsToHold;

    displayBufferIn = (displayBufferIn + 1) % DISPLAYBUFFERSIZE;

}

//The display buffer is preloaded with the logo, so it just needs to be displayed
//and held for a time.
void displayStartupLogo(){
  setDisplayState(DISPLAY_STATE_STARTUP, 5);
}

void stageWifiInfo(char* descr, char* SSID, char* IP){
    _wifiDescr = String(descr);
    _wifiSSID = String(SSID);
    _wifiIP = String(IP);

    setDisplayState(DISPLAY_STATE_WIFIINFO, 8);
}

void displayWifiInfo(){

    display.clearDisplay();
    //clear the double buffer
    memset(display.getBuffer(),0,1024);
    display.setFont(&TomThumb);
    display.ssd1306_command(SSD1306_SETSTARTLINE);
    display.setCursor(0,7);
    display.print(_wifiDescr+_wifiSSID);
    display.setCursor(0,15);
    display.print(String("IP Addr:" + _wifiIP));
    display.setCursor(0,23);
    display.print(String("Firmware ver:") + String(APP_VERSION));
    display.display();

}

void stageDisplayMsg(char* msg) {
    _displayMsg = String(msg);
    setDisplayState(DISPLAY_STATE_MESSAGE, 8);
}

void displayMsg(){
    display.clearDisplay();
    //clear the double buffer
    memset(display.getBuffer(),0,1024);
    display.setFont(&TomThumb);
    display.ssd1306_command(SSD1306_SETSTARTLINE);
    display.setCursor(0,7);
    display.print(_displayMsg);
    display.display();
}


int getCurrentDisplayState() {

    displayStates_t returnVal = DISPLAY_STATE_NONE;
    if(displayBufferIn != displayBufferOut){
        returnVal = displayStateBuffer[displayBufferOut].displayState;
    } else if (WiFi.status() == WL_CONNECTED){
        returnVal = DISPLAY_STATE_CONNECTED;
    } else {
        returnVal = DISPLAY_STATE_TEST;
    }

    //DEBUG_MSG("[DISPLAY] getCurrentDisplayState returning %i\n", returnVal);
    return (int)returnVal;
}


void maintainDisplayState() {
    static int lastMillis = 0;
    int nowMillis = millis();

    //if there are any timed display states, decrement the current one every
    //second and when it times out, move to the next
    //DEBUG_MSG("[DISPLAY] displayBufferIn: %i displayBufferOut: %i\n", displayBufferIn, displayBufferOut);
    if(displayBufferIn != displayBufferOut)
    if(nowMillis > (lastMillis + 1000)){
        lastMillis = nowMillis;
        displayStateBuffer[displayBufferOut].secondsToHold--;
        if(displayStateBuffer[displayBufferOut].secondsToHold <= 0){
            DEBUG_MSG("[DISPLAY] Removing element: %i from displayStateBuffer\n", displayBufferOut);
            displayStateBuffer[displayBufferOut].displayState = DISPLAY_STATE_NONE;
            displayStateBuffer[displayBufferOut].secondsToHold = 0;

            displayBufferOut = (displayBufferOut + 1) % DISPLAYBUFFERSIZE;
            
        }
    }


}

void doDisplay() {
    static int lastMillis = 0;
    int nowMillis = millis();
    if(nowMillis > (lastMillis + 2000)){
        lastMillis = nowMillis;
        int state = getCurrentDisplayState();
        switch (state) {
            case DISPLAY_STATE_STARTUP:
                //DEBUG_MSG("[DISPLAY] DISPLAY_STATE_STARTUP\n");
                display.display();
                break;
            case DISPLAY_STATE_WIFIINFO:
                //DEBUG_MSG("[DISPLAY] DISPLAY_STATE_WIFIINFO\n");
                displayWifiInfo();
                break;
            case DISPLAY_STATE_MESSAGE:
                //DEBUG_MSG("[DISPLAY] DISPLAY_STATE_WIFIINFO\n");
                displayMsg();
                break;
            case DISPLAY_STATE_CONNECTED:
                //DEBUG_MSG("[DISPLAY] DISPLAY_STATE_CONNECTED\n");
                displayInfo();
                break;
            case DISPLAY_STATE_TEST:
                //DEBUG_MSG("[DISPLAY] DISPLAY_STATE_TEST\n");
                displayTestInfo();
                break;
            case DISPLAY_STATE_NONE:
                displayInfo();
                break;
            default:
                displayInfo();
                break;
        }
    }
}


void displayLoop(){

    maintainDisplayState();

    doDisplay();

}
