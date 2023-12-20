/* 
 * Project NeoPixel controller with Blue Tooth and Encoder Input
 * Author: Jon Phillips
 * Last Updated: 19 Decenmber 2023
 * 
 */

#include "Particle.h"
#include <colors.h>
#include <Encoder.h>
#include <neopixel.h>
#include <math.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <button.h>
SYSTEM_MODE(SEMI_AUTOMATIC);
int OLED_RESET(-1);
Adafruit_SSD1306 display(OLED_RESET);

const BleUuid serviceUuid("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
const BleUuid rxUuid("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");
const BleUuid txUuid("6E400003-B5A3-F393-E0A9-E50E24DCCA9E");
const int UART_TX_BUF_SIZE = 50;
uint8_t txBuf[UART_TX_BUF_SIZE];
uint8_t i;

void onDataReceived(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context);
BleCharacteristic txCharacteristic("tx", BleCharacteristicProperty::NOTIFY, txUuid, serviceUuid);
BleCharacteristic rxCharacteristic("rx", BleCharacteristicProperty::WRITE_WO_RSP, rxUuid, serviceUuid, onDataReceived, NULL);
BleAdvertisingData data;

Encoder enCoder(D8,D9);
const int PIXELCOUNT = 24;
Adafruit_NeoPixel pixel(PIXELCOUNT,SPI1,WS2812B);
int encRead;
int encPos;
int newEnc;
int prevEnc;
int j;
int k;
int rBow;
int rRed;
int rBlue;
int rGreen;
void pixelFill(int startPixel, int endPixel,int hex);
void ChooseShow();
void DisplayShow();
int setColor;
int pBright=60;
int rainB;
int randColor; 
int hexColor;
int x;
int y;
int z;
float t;
float t2;
int sinWave;
int sinPix;
int ShowChoice;
const int button1 = D12;
const int button2 = D13;
const int button3 = D14;
const int button4 = D19;
const int button5 = D18;
const int button6 = D17;
bool buttonState1;
bool buttonState2;
bool buttonState3;
bool buttonState4;
bool buttonState5;
bool buttonState6;


void setup() {
  Serial.begin(9600);
  pinMode(button1,INPUT);
  pinMode(button2,INPUT);
  pinMode(button3,INPUT);
  pinMode(button4,INPUT);
  pinMode(button5,INPUT);
  pinMode(button6,INPUT);

  BLE.on();
  BLE.addCharacteristic(txCharacteristic);
  BLE.addCharacteristic(rxCharacteristic);
  data.appendServiceUUID(serviceUuid);
  BLE.advertise(&data);

  pixel.begin();
  pixel.show();
  pixel.setBrightness(pBright);

  display.begin(SSD1306_SWITCHCAPVCC,0x3C);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.display();


  Serial.printf("Photon2 BLE Address %s\n",BLE.address().toString().c_str());
}

void loop() {

                                                        // Manual Button Operation

  
// button 1 turns on bright white room light

buttonState1 = digitalRead(button1);

if(buttonState1){
  pixel.setBrightness(pBright);
  pixelFill(0,PIXELCOUNT,white);
  setColor=white;
 }

 // Button 2 turns off all lights

 buttonState2 = digitalRead(button2);

 if(buttonState2){
  pixel.clear();
  pixel.show();
 }

// Button 3 cycles through colors of the rainbow

buttonState3 = digitalRead(button3);

if(buttonState3){
  rainB=rainbow[y];
  pixelFill(0,PIXELCOUNT,rainB);
  setColor=rainB;  
  y++;
  delay(100);
  Serial.printf("Rainbow Value ---  %i\n",y);
  if(y>=7){     
    y=0;
  }
 }

 // Button 4 decreases Brightness

 buttonState4 = digitalRead(button4);

 if(buttonState4){
  pBright = pBright-20;
  pixel.setBrightness(pBright);
  pixelFill(0,PIXELCOUNT,setColor);

 }

 // Button 5 increases Brightness

 buttonState5 = digitalRead(button5);

if(buttonState5){
  pBright = pBright+20;
  pixel.setBrightness(pBright);
  pixelFill(0,PIXELCOUNT,setColor);
  
}
  
// Button 6 starts chosen show

buttonState6 = digitalRead(button6);

if(buttonState6){
  pixel.clear();
  pixel.show();
  DisplayShow();
}


ChooseShow(); //uses encoder to choose which show will be presented when button is pushed

  
}
                                                    //    BLUE TOOTH CONTROL VIA BLUEFRUIT MOBILE APP


void onDataReceived(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context){
  uint8_t i;
  int rRx, bRx, gRx;  

  Serial.printf("Received data from: %02X:%02X:%02X:%02X:%02X:%02X\n",peer.address()[0],peer.address()[1],peer.address()[2],peer.address()[3],peer.address()[4],peer.address()[5]);
  Serial.printf("Bytes: ");
  for (i=0;i<len;i++){
    Serial.printf("%02X ",data[i]);
  }
  Serial.printf("\n");
  Serial.printf("Message: %s\n",(char *)data);

                                                      //COLOR PICKER
  
  
  // Photon2 Recieves RGB data from color picker, converts to Hex, and displays color on pixels

  if(data[1]==0x43){ 
    rRx = data[2];
    gRx = data[3];
    bRx = data[4];
    hexColor = (rRx<<16)|(gRx<<8)|(bRx);
    pixelFill(0,PIXELCOUNT,hexColor);
    Serial.printf("Hex color -- %i",hexColor);
    setColor = hexColor;  // allows color to remain when switcing to Control Pad
  }


                                                      // BLUEFRUIT BLUETOOTH CONTROL PAD 

    
 
// button 1 puts pixels in room light mode
if((data[1]==0x42)&&(data[2]==0x31)&&(data[3]==0x31)){
  pixel.setBrightness(150);
  pixelFill(0,PIXELCOUNT,white);
 
}
 //up arrow increases brightness 
if((data[1]==0x42)&&(data[2]==0x35)&&(data[3]==0x31)){
  pBright = pBright+20;
  pixel.setBrightness(pBright);
  pixelFill(0,PIXELCOUNT,setColor);
             
}
  //down arrow decreases brightness
if((data[1]==0x42)&&(data[2]==0x36)&&(data[3]==0x31)){
  pBright=pBright-20;
  pixel.setBrightness(pBright);
  pixelFill(0,PIXELCOUNT,setColor);
}

  // left arrow decreases encoder position to previous show
if((data[1]==0x42)&&(data[2]==0x37)&&(data[3]==0x31)){
  enCoder.write(encPos-10);
  encPos = encPos-10;
} 

// right arrow increases encoder position to next show
if((data[1]==0x42)&&(data[2]==0x38)&&(data[3]==0x31)){
  enCoder.write(encPos+10);
  encPos = encPos+10;
}

  // button 2 cylcles through the colors of the rainbow
if((data[1]==0x42)&&(data[2]==0x32)&&(data[3]==0x31)){ 
  rainB=rainbow[y];
  pixelFill(0,PIXELCOUNT,rainB);
  setColor=rainB;   //sets color at a constant for the rest of the code
  y++;
  if(y>=7){
    y=0;
  }

  
}

   // button 3 displays show on OLED screen
if((data[1]==0x42)&&(data[2]==0x33)&&(data[3]==0x31)){ 
    pixel.clear();  
    DisplayShow();
}



   // button 4 turns off pixels
if((data[1]==0x42)&&(data[2]==0x34)&&(data[3]==0x31)){
    pixel.clear();
    pixel.show();
  
  
}

}

void pixelFill(int startPixel, int endPixel, int hex){
int i;
int j;

pixel.clear();
for(i=startPixel;i<endPixel;i++){
    pixel.setPixelColor(i,hex);   
}

pixel.show();

for(j=PIXELCOUNT-1;j>endPixel;j--){
    pixel.clear();
}
}

void ChooseShow(){
  encPos = enCoder.read();
  
  
  if((encPos>=0)&&(encPos<=5)){
    display.clearDisplay();
    display.setCursor(0,0);
    display.printf("Sparkle");
    display.display();
    ShowChoice=1;
  }
  if((encPos>=10)&&(encPos<=15)){
    display.clearDisplay();
    display.setCursor(0,0);
    display.printf("Blob");
    display.display();
    ShowChoice=2;
  }

  if((encPos>=20)&&(encPos<=25)){
    display.clearDisplay();
    display.setCursor(0,0);
    display.printf("Rainbow");
    display.display();
    ShowChoice=3;
  }
  if((encPos>=30)&&(encPos<=35)){
    display.clearDisplay();
    display.setCursor(0,0);
    display.printf("Fill The\nGlass");
    display.display();
    ShowChoice=4;
  }

  if((encPos>=40)&&(encPos<=45)){
    display.clearDisplay();
    display.setCursor(0,0);
    display.printf("Breath");
    display.display();
    ShowChoice=5;

  }

  if((encPos>=50)&&(encPos<=55)){
    display.clearDisplay();
    display.setCursor(0,0);
    display.printf("Chase");
    display.display();
    ShowChoice=6;
  }

  if((encPos>=60)&&(encPos<=65)){
    display.clearDisplay();
    display.setCursor(0,0);
    display.printf("Void");
    display.display();
    ShowChoice=7;
  }
  
  if(encPos<0){
    enCoder.write(65);
    encPos=65;
  }
  if(encPos>65){
    enCoder.write(0);
    encPos=0;
  }


}
void DisplayShow(){
  int rbTimer;
  int blobTimer;
  static int y;
  

//sparkle
if(ShowChoice==1){
  for(x=0;x<75;x++){
      randColor = random(0x000000,0xFFFFFF);
      pixel.setBrightness(random(0,255));
      pixel.setPixelColor(random(0,PIXELCOUNT),randColor);
      pixel.show();
      pixel.clear();
      delay(random(50,100));   
  }
  pixel.setBrightness(pBright);   
  pixelFill(0,PIXELCOUNT,setColor);

}

//Blob
if(ShowChoice==2){
  blobTimer=millis();
  while((millis()-blobTimer)<15000){
      t=millis()/4000.0;
      t2=millis()/20000.0;
      sinWave = 125*sin(2*M_PI*(2)*t)+127;
      sinPix = (PIXELCOUNT/2)*sin(2*M_PI*(2)*t2)+(PIXELCOUNT/2);
      pixel.setBrightness(sinWave);
      pixelFill(sinPix-1,sinPix+2,setColor);
     
  }
  pixel.setBrightness(pBright);   
  pixelFill(0,PIXELCOUNT,setColor);
}

//Rainbow
if(ShowChoice==3){
  rbTimer=millis();
  while ((millis()-rbTimer)<10000){
    for(x=0;x<7;x++){
      pixelFill(0,PIXELCOUNT,rainbow[x]);
      delay(250);
    }
  }
  pixel.setBrightness(pBright);   
  pixelFill(0,PIXELCOUNT,setColor);
}


// fill the glass
if(ShowChoice==4){
y=0;

  while(y<PIXELCOUNT){    
    for(x=PIXELCOUNT+1;x>y;x--){
      pixel.setPixelColor(x,setColor);
      pixel.show();
      delay(20);
      pixel.setPixelColor(x,black);
      pixel.show();       
    }
    y=y+1;
    for(z=0;z<y;z++){
      pixel.setPixelColor(z,setColor);
      pixel.show();
    }   
  }   
}

//Breath

if(ShowChoice==5){
  
  for(x=0; x<5000; x++){
    t=millis()/6000.0;
    sinWave = 125*sin(2*M_PI*(2)*t)+127;
    pixel.setBrightness(sinWave);
    pixelFill(0,PIXELCOUNT,setColor);
    pixel.clear();
    pixel.show();
   
  }
  pixel.setBrightness(pBright);   
  pixelFill(0,PIXELCOUNT,setColor);
 }

// Chase

if(ShowChoice==6){
  
  for(x=0; x<PIXELCOUNT+7; x++){
    pixel.setPixelColor(x,rainbow[0]);
    pixel.setPixelColor(x-1,rainbow[1]);
    pixel.setPixelColor(x-2,rainbow[2]);
    pixel.setPixelColor(x-3,rainbow[3]);
    pixel.setPixelColor(x-4,rainbow[4]);
    pixel.setPixelColor(x-5,rainbow[5]);
    pixel.setPixelColor(x-6,rainbow[6]);
    pixel.setPixelColor(x-7,0x000000);
    // pixel.setPixelColor(x-8,0x000000);
    // pixel.setPixelColor(x-9,0x000000);
    // pixel.setPixelColor(x-10,0x000000);
    // pixel.setPixelColor(x-11,0x000000);
    // pixel.setPixelColor(x-12,0x000000);
    // pixel.setPixelColor(x-13,rainbow[0]);
    // pixel.setPixelColor(x-14,rainbow[1]);
    // pixel.setPixelColor(x-15,rainbow[2]);
    // pixel.setPixelColor(x-16,rainbow[3]);
    // pixel.setPixelColor(x-17,rainbow[4]);
    // pixel.setPixelColor(x-18,rainbow[5]);
    // pixel.setPixelColor(x-19,rainbow[6]);
    // pixel.setPixelColor(x-20,0x000000);
    pixel.show();
    delay(50);
  }

    for(x=PIXELCOUNT; x>-8; x--){
    pixel.setPixelColor(x,rainbow[0]);
    pixel.setPixelColor(x+1,rainbow[1]);
    pixel.setPixelColor(x+2,rainbow[2]);
    pixel.setPixelColor(x+3,rainbow[3]);
    pixel.setPixelColor(x+4,rainbow[4]);
    pixel.setPixelColor(x+5,rainbow[5]);
    pixel.setPixelColor(x+6,rainbow[6]);
    pixel.setPixelColor(x+7,0x000000);
    // pixel.setPixelColor(x+8,0x000000);
    // pixel.setPixelColor(x+9,0x000000);
    // pixel.setPixelColor(x+10,0x000000);
    // pixel.setPixelColor(x+11,0x000000);
    // pixel.setPixelColor(x+12,0x000000);
    // pixel.setPixelColor(x+13,rainbow[0]);
    // pixel.setPixelColor(x+14,rainbow[1]);
    // pixel.setPixelColor(x+15,rainbow[2]);
    // pixel.setPixelColor(x+16,rainbow[3]);
    // pixel.setPixelColor(x+17,rainbow[4]);
    // pixel.setPixelColor(x+18,rainbow[5]);
    // pixel.setPixelColor(x+19,rainbow[6]);
    // pixel.setPixelColor(x+20,0x000000);
    pixel.show();
    delay(50);
  }

  pixel.setBrightness(pBright);   
  pixelFill(0,PIXELCOUNT,setColor);
}

// Void

if(ShowChoice==7){
  pixel.setBrightness(75);
  pixelFill(0,PIXELCOUNT,white);
 
  for(x=-6;x<PIXELCOUNT+6;x++){
    pixel.setPixelColor(x-4,100,100,100);
    pixel.setPixelColor(x-3,75,75,75);
    pixel.setPixelColor(x-2,50,50,50);
    pixel.setPixelColor(x-1,25,25,25);
    pixel.setPixelColor(x,0,0,0);
    pixel.setPixelColor(x+1,25,25,25);
    pixel.setPixelColor(x+2,50,50,50);
    pixel.setPixelColor(x+3,75,75,75);
    pixel.setPixelColor(x+4,100,100,100);
    pixel.show();

   
    delay(50);


  }
}
}