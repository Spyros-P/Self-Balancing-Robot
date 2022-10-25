
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels


#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


void setup() {
  Serial.begin(9600);

  oled_setup();
}

String mess = "Time remaining:\n\nPassword: ";
String pass = "________";
String print_mess;
int counter = 0;

void loop() {
  print_mess = mess;
  print_mess.concat(pass);
  
  oled_print(print_mess);
 
  while(Serial.available( ) == 0);
  int arrivingdatabyte = Serial.read( );
  
  if (arrivingdatabyte == '\b') {
    counter--;
    pass[counter]='_';
  }
  else if (arrivingdatabyte == '\n') {
    oled_print("Not valid!");
    pass = "________";
    counter = 0;
  }
  else {
    if (counter < 8) {
      pass[counter]='*';
      counter++;
    }
  }
}


void oled_setup(void) {
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.display();
}

void oled_print(String str) {
  display.clearDisplay();

  display.setCursor(0, 0);

  for (int i=0; i<str.length(); i++)
    display.write(str[i]);

  display.display();
}
