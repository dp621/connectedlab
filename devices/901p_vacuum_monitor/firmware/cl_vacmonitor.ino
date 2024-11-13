#include <ConnectedLab8266.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     4
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

ConnectedLab8266 connectedLab;
boolean debug=false;
long lastPressure=0;
int lastPressureTimeout=100;

void setupOutput(){
  if (debug){
    //if we are in debug mode, we are printing to serial out
    Serial.begin(115200);
  }
  else {
    //if we are in normal mode, we are rx/tx with the 901p loadlock transducer
    Serial.begin(9600);
  }
  delay(10);
}

void setup() {
  Wire.begin(0,2);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextSize(2);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  // Not all the characters will fit on the display. This is normal.
  // Library will draw what it can and the rest will be clipped.
  display.write("CONNECTED");
  display.setCursor(0,15);
  display.write("LAB");

  drawFlask();
  drawBeaker();
  
  display.display();
  delay(3000);
  
  setupOutput();
  //connectedLab.setDebug(true);
  connectedLab.begin();
}
void drawFlask(){
  int offsetX=50;
  int offsetY=35;
  display.drawLine(offsetX+5, offsetY, offsetX+15, offsetY, SSD1306_WHITE);
  display.drawLine(offsetX+5, offsetY, offsetX+5, offsetY+10, SSD1306_WHITE);
  display.drawLine(offsetX+15, offsetY, offsetX+15, offsetY+10, SSD1306_WHITE);
  display.drawLine(offsetX, offsetY+25, offsetX+5, offsetY+10, SSD1306_WHITE);
  display.drawLine(offsetX+20, offsetY+25, offsetX+15, offsetY+10, SSD1306_WHITE);
  display.drawLine(offsetX, offsetY+25, offsetX+20, offsetY+25, SSD1306_WHITE);
}
void drawBeaker(){
  int offsetX=70;
  int offsetY=28;
  display.drawLine(offsetX+5, offsetY, offsetX+15, offsetY, SSD1306_WHITE);
  display.drawLine(offsetX+5, offsetY, offsetX+5, offsetY+25, SSD1306_WHITE);
  display.drawLine(offsetX+15, offsetY, offsetX+15, offsetY+25, SSD1306_WHITE);
  display.drawLine(offsetX, offsetY+25, offsetX+20, offsetY+25, SSD1306_WHITE);
}

float readPressure(){
  Serial.write("@254PR4?;FF");
  delay(100);
  String res="";
  float val=-1;
  res=Serial.readString();
  res.remove(0,7);
  res.remove(8);
    
  //parse response for conversion
  if (res.endsWith("E+2")){
    res.remove(5);
    val=res.toFloat();
    val=val*100.0;
    //data += val;
    //data += ", \"unit\":\"torr\"}";
    val=val*1000;
  }
  else if (res.endsWith("E+1")){
    res.remove(5);
    val=res.toFloat();
    val=val*10.0;
    //data += val;
    //data += ", \"unit\":\"torr\"}";
    val=val*1000;
  }
  else if (res.endsWith("E+0")){
    res.remove(5);
    val=res.toFloat();
    val=val*1000.0;
    //data += val;
    //data += ", \"unit\":\"micron\"}";
  }
  else if (res.endsWith("E-1")){
    res.remove(5);
    val=res.toFloat();
    val=val*100.0;
    //data += val;
    //data += ", \"unit\":\"micron\"}";
  }
  else if (res.endsWith("E-2")){
    res.remove(5);
    val=res.toFloat();
    val=val*10.0;
    //data += val;
    //data += ", \"unit\":\"micron\"}";
  }
  else if (res.endsWith("E-3")){
    res.remove(5);
    val=res.toFloat();
    val=val*1.0;
    //data += val;
    //data += ", \"unit\":\"micron\"}";
  }

  //draw
  display.clearDisplay();
  display.setTextSize(2);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  // Not all the characters will fit on the display. This is normal.
  // Library will draw what it can and the rest will be clipped.
  if (val>10000){
    val=val/1000.0;
    char buffer[7];
    dtostrf(val, 2, 1, buffer);
    //int num = val;
    //char cstr[16];
    //itoa(num, cstr, 10);
    display.setTextSize(3);
    //display.write(cstr);
    display.write(buffer);
    display.setCursor(0, 50);
    display.setTextSize(2);
    display.write("torr");
  }
  else if (val>1000){
    char buffer[7];
    dtostrf(val, 2, 1, buffer);
    //int num = val;
    //char cstr[16];
    //itoa(num, cstr, 10);
    display.setTextSize(3);
    //display.write(cstr);
    display.write(buffer);
    display.setCursor(0, 50);
    display.setTextSize(2);
    display.write("microns");
  }
  else if (val>0){
    char buffer[7];
    dtostrf(val, 2, 3, buffer);
    //int num = val;
    //char cstr[16];
    //itoa(num, cstr, 10);
    display.setTextSize(3);
    //display.write(cstr);
    display.write(buffer);
    display.setCursor(0, 50);
    display.setTextSize(2);
    display.write("microns");
  }
  else{
    display.write("connect");
    display.setCursor(0, 15);
    display.write("sensor");
  }

  
    /*display.setCursor(100, 5);
    if (connectedLab.isReady()){
      display.write("R");
    }
    else if (connectedLab.inSetupMode()){
      display.write("S");
    }
    else {
      display.write("?");
    }*/
    display.display();

  return val;
}


void loop() {

  connectedLab.run();
  //check if its time to look at sensor
  if (!debug && (((lastPressure+lastPressureTimeout)<millis()) || lastPressure==0)){
    float pressure=readPressure();
    if (pressure>-1){
      connectedLab.setUploadValue(pressure);
      connectedLab.setUploadUnit("micron");
      connectedLab.uploadData();
    }
    lastPressure=millis();
  }

}
