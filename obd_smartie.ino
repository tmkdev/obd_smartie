/**************************************************************************
  This is a library for several Adafruit displays based on ST77* drivers.

  Works with the Adafruit 1.8" TFT Breakout w/SD card
    ----> http://www.adafruit.com/products/358
  The 1.8" TFT shield
    ----> https://www.adafruit.com/product/802
  The 1.44" TFT breakout
    ----> https://www.adafruit.com/product/2088
  The 1.3" TFT breakout
  ----> https://www.adafruit.com/product/4313
  The 1.54" TFT breakout
    ----> https://www.adafruit.com/product/3787
  The 2.0" TFT breakout
    ----> https://www.adafruit.com/product/4311
  as well as Adafruit raw 1.8" TFT display
    ----> http://www.adafruit.com/products/618

  Check out the links above for our tutorials and wiring diagrams.
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional).

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 **************************************************************************/

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <Fonts/FreeSans24pt7b.h>
#include "SegGauge.h"
#include "Graph.h"
#include <CAN.h> // the OBD2 library depends on the CAN library
#include <OBD2.h>

static const int spiClk = 8000000; // 1 MHz

#define ST77XX_GRAY 0x4208

#ifdef ADAFRUIT_HALLOWING
#define TFT_CS        39 // Hallowing display control pins: chip select
#define TFT_RST       37 // Display reset
#define TFT_DC        38 // Display data/command select
#define TFT_BACKLIGHT  7 // Display backlight pin

#elif defined(ADAFRUIT_PYBADGE_M4_EXPRESS) || defined(ADAFRUIT_PYGAMER_M4_EXPRESS)
#define TFT_CS        44 // PyBadge/PyGamer display control pins: chip select
#define TFT_RST       46 // Display reset
#define TFT_DC        45 // Display data/command select
#define TFT_BACKLIGHT 47 // Display backlight pin

#elif defined(ESP32)
#define TFT_CS         5
#define TFT_RST        22
#define TFT_DC         21
//
// define not needed for all pins; reference for ESP32 physical pins connections to VSPI:
// SDA  GPIO23 aka VSPI MOSI
// SCLK GPIO18 aka SCK aka VSPI SCK
// D/C  GPIO21 aka A0 (also I2C SDA)
// RST  GPIO22 aka RESET (also I2C SCL)
// CS   GPIO5  aka chip select
// LED  3.3V
// VCC  5V
// GND - GND
//
#elif defined(ESP8266)
#define TFT_CS         4
#define TFT_RST        16
#define TFT_DC         5

#else
// For the breakout board, you can use any 2 or 3 pins.
// These pins will also work for the 1.8" TFT shield.
#define TFT_CS        10
#define TFT_RST        9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         8
#endif

// OPTION 1 (recommended) is to use the HARDWARE SPI pins, which are unique
// to each board and not reassignable. For Arduino Uno: MOSI = pin 11 and
// SCLK = pin 13. This is the fastest mode of operation and is required if
// using the breakout board's microSD card.

#if defined(ADAFRUIT_PYBADGE_M4_EXPRESS) || defined(ADAFRUIT_PYGAMER_M4_EXPRESS)
// For PyBadge and PyGamer
Adafruit_ST7735 tft = Adafruit_ST7735(&SPI1, TFT_CS, TFT_DC, TFT_RST);
#else
// For 1.44" and 1.8" TFT with ST7735 (including HalloWing) use:
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// For 1.3", 1.54", and 2.0" TFT with ST7789:
//Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
#endif

// OPTION 2 lets you interface the display using ANY TWO or THREE PINS,
// tradeoff being that performance is not as fast as hardware SPI above.
//#define TFT_MOSI 11  // Data out
//#define TFT_SCLK 13  // Clock out

// For ST7735-based displays, we will use this call
//Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// OR for the ST7789-based displays, we will use this call
//Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// Smart guage globals
bool obd_ok = false;
String lastval = "--";



void setup(void) {
  Serial.begin(115200);
  Serial.print(F("OBD Smartgauge starting.."));

#ifdef ADAFRUIT_HALLOWING
  // HalloWing is a special case. It uses a ST7735R display just like the
  // breakout board, but the orientation and backlight control are different.
  tft.initR(INITR_HALLOWING);        // Initialize HalloWing-oriented screen
  pinMode(TFT_BACKLIGHT, OUTPUT);
  digitalWrite(TFT_BACKLIGHT, HIGH); // Backlight on

#elif defined(ADAFRUIT_PYBADGE_M4_EXPRESS) || defined(ADAFRUIT_PYGAMER_M4_EXPRESS)
  tft.initR(INITR_BLACKTAB);        // Initialize ST7735R screen
  tft.setRotation(1);
  pinMode(TFT_BACKLIGHT, OUTPUT);
  digitalWrite(TFT_BACKLIGHT, HIGH); // Backlight on

#else
  // Use this initializer if using a 1.8" TFT screen:
  tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab

  // OR use this initializer (uncomment) if using a 1.44" TFT:
  //tft.initR(INITR_144GREENTAB); // Init ST7735R chip, green tab

  // OR use this initializer (uncomment) if using a 0.96" 180x60 TFT:
  //tft.initR(INITR_MINI160x80);  // Init ST7735S mini display

  // OR use this initializer (uncomment) if using a 1.3" or 1.54" 240x240 TFT:
  //tft.init(240, 240);           // Init ST7789 240x240

  // OR use this initializer (uncomment) if using a 2.0" 320x240 TFT:
  //tft.init(240, 320);           // Init ST7789 320x240
#endif

  Serial.println(F("Initialized"));

  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);

  int obd_connect_attempts = 0;

  // rx, tx
  CAN.setPins(14, 27);
  OBD2.setTimeout(300);

  while (obd_connect_attempts < 6) {
    displayMessage(F("Waiting for OBD..."), ST77XX_WHITE);
    Serial.println(F("Attempting OBD Initialization"));

    if (!OBD2.begin()) {
      Serial.println(F("OBD Connection failed!"));
      obd_connect_attempts += 1;
      delay(1000);
    } else {
      Serial.println(F("OBD Connection success!"));
      obd_ok = true;
      break;
    }
  }

  tft.fillScreen(ST77XX_BLACK);
}


void dualDisplay(int bg_color, int fg_col_1, int fg_col_2, int pid1, int pid2, int precision1, int precision2, String title, String label1, String label2, float min1, float max1, float min2, float max2 ) {
  SegGauge seggauge = SegGauge(&tft, 58, 60, 65, 8, 8, 2, fg_col_1, bg_color);
  SegGauge seggauge2 = SegGauge(&tft, 58, 130, 65, 8, 8, 2, fg_col_2, bg_color);

  float val1 = 0;
  float val2 = 0;

  if (obd_ok) {
    val1 = OBD2.pidRead(pid1);
    val2 = OBD2.pidRead(pid2);
  } else {
    val1 = random(min1 * 100,  max1 * 100) / 100.0;
    val2 = random(min2 * 100,  max2 * 100) / 100.0;
    delay(random(60, 85));
  }

  tft.setTextWrap(false);
  String sval1 = String(val1, precision1);
  String sval2 = String(val2, precision2);

  sval1 = String(sval1 + "  ");
  sval2 = String(sval2 + "  ");

  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setCursor(80 - (title.length() * 6) / 2, 95);
  tft.print(title);

  tft.setCursor(5, 75);
  tft.print(label1);

  tft.setCursor(70, 75);
  tft.print(label2);

  tft.setCursor(30, 45);
  tft.setTextSize(1);
  tft.println(sval1);

  tft.setCursor(105, 45);
  tft.setTextSize(1);
  tft.println(sval2);

  seggauge.draw((val1 - min1) / (max1 - min1));
  seggauge2.draw((val2 - min2) / (max2 - min2));
}

void displayMessage(String text, int textcolor ) {
  tft.setTextSize(1);
  tft.setTextColor(textcolor, ST77XX_BLACK);
  int xpos = 80 - (text.length() * 6) / 2;

  tft.setCursor(xpos, 56);
  tft.print(text);
}

void bignumbers(int pid, int precision, String title, float minval, float maxval, int textcolor) {
  tft.setTextWrap(false);
  float val1;

  if (obd_ok) {
    val1 = OBD2.pidRead(pid);
  } else {
    val1 = random(minval * 100,  maxval * 100) / 100.0;
    delay(random(60, 85));
  }

  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  String sval = String(val1, precision);

  tft.setTextSize(1);
  tft.setCursor(80 - (title.length() * 6) / 2, 95);
  tft.print(title);

  tft.setTextColor(ST77XX_BLACK, ST77XX_BLACK);
  tft.setCursor(80  - (3 * 7 * lastval.length()), 20);
  tft.setTextSize(7);
  tft.print(lastval);
  lastval = sval;

  tft.setTextColor(textcolor, ST77XX_BLACK);
  tft.setCursor( 80  - (3 * 7 * sval.length()), 20);
  tft.setTextSize(7);
  tft.print(sval);

}

void graph(int pid, String title, float minval, float maxval, int tracecolor) {
  float val1;

  if (obd_ok) {
    val1 = OBD2.pidRead(pid);
  } else {
    val1 = random(minval * 100,  maxval * 100) / 100.0;
    delay(random(60, 85));
  }

  Graph g = Graph(&tft, title, minval, maxval, tracecolor);
  g.draw(val1);

}


int currentScreen = 0;
bool upKey = false;
bool downKey = false;

int touchAve(int pin, int numSamples) {
  int total = 0;

  for (int x = 0; x < numSamples; x += 1 ) {
    total = total + touchRead(pin);
    delay(1);
  }

  return total / numSamples;

}

int screens = 9;

void loop() {

  int uptouch = touchAve(T4, 3);
  int downtouch = touchAve(T5, 3);

  if ( !upKey && uptouch < 30) {
    upKey = true;
  }

  if ( upKey && uptouch > 50 ) {
    tft.fillScreen(ST77XX_BLACK);
    currentScreen += 1;
    currentScreen = currentScreen % screens;
    upKey = false;
  }

  if ( !downKey && downtouch < 30) {
    downKey = true;
  }

  if ( downKey && downtouch > 50 ) {
    tft.fillScreen(ST77XX_BLACK);
    currentScreen -= 1;
    if (currentScreen < 0) {
      currentScreen = screens - 1;
    }
    currentScreen = currentScreen % screens;
    downKey = false;
  }

  //Serial.println(currentScreen);
  Serial.println(uptouch);
  //Serial.println(downtouch);

  switch (currentScreen) {
    case 0:
      dualDisplay(ST77XX_GRAY, ST77XX_YELLOW, ST77XX_YELLOW, SHORT_TERM_FUEL_TRIM_BANK_1, SHORT_TERM_FUEL_TRIM_BANK_2, 1, 1, F("STFT"), F("Bank 1"), F("Bank 2"), -100, 100, -100, 100 );
      break;
    case 1:
      dualDisplay(ST77XX_GRAY, ST77XX_BLUE, ST77XX_BLUE, LONG_TERM_FUEL_TRIM_BANK_1, LONG_TERM_FUEL_TRIM_BANK_2, 1, 1, F("LTFT"), F("Bank 1"), F("Bank 2"), -100, 100, -100, 100 );
      break;
    case 2:
      dualDisplay(ST77XX_GRAY, ST77XX_GREEN, ST77XX_RED, VEHICLE_SPEED, ENGINE_RPM, 0, 0, F("Speed/RPM"), F("Speed kph"), F("RPM"), 0, 200, 0, 6500 );
      break;
    case 3:
      dualDisplay(ST77XX_GRAY, ST77XX_BLUE, ST77XX_CYAN, ENGINE_COOLANT_TEMPERATURE, AIR_INTAKE_TEMPERATURE, 0, 0, F("Temperatures"), F("ECT C"), F("IAT C"), -40, 215, -40, 215 );
      break;
    case 4:
      dualDisplay(ST77XX_GRAY, ST77XX_RED, ST77XX_ORANGE, THROTTLE_POSITION, CALCULATED_ENGINE_LOAD, 0, 0, F("Engine Info"), F("TPS %"), F("LOAD %"), 0, 100, 0, 100 );
      break;
    case 5:
      dualDisplay(ST77XX_GRAY, ST77XX_CYAN, ST77XX_CYAN, MAF_AIR_FLOW_RATE, INTAKE_MANIFOLD_ABSOLUTE_PRESSURE, 0, 0, F("Airflow"), F("MAF gps"), F("MAP kpa"), 0, 655, 0, 255 );
      break;
    case 6:
      bignumbers(VEHICLE_SPEED, 0, F("Speed kph"), 0, 200, ST77XX_RED);
      break;
    case 7:
      bignumbers(TIMING_ADVANCE, 0, F("Timing Advance Deg"), -64, 64, ST77XX_WHITE);
      break;
    case 8:
      graph(OXYGEN_SENSOR_1_SHORT_TERM_FUEL_TRIM, F("O2 S1 V"), 0, 1.28, ST77XX_YELLOW);
      break;
  };

}
