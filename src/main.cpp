#include <Arduino.h>

/*
  16xLEDRing

  This Sketch includes many different effects for RGB LED Rings
  Each mode can be selected with an input (button)
  The speed of the effect can be changed

  Adapted Sketch to include Step Delay control using a Potentiometer (Analog input)
  Original Code by Erin St. Blaine for Adafruit.com
*/

#include <FastLED.h>

#define LED_PIN 5
#define COLOR_ORDER GRB
#define NUM_LEDS 16
#define NUM_MODES 6

const byte STEP_DELAY_PIN = PIN_A0; // control speed with the delay value from analog input (10K potentiometer)
const byte BUTTON_PIN = PIN3;// trigger alternate patterns

int HUE = 0;
int SATURATION = 255;
int BRIGHTNESS = 128;   // also affects power consumption (lower value to reduce power)
int STEPS = 4;          // change color value by this amount each time
int STEP_DELAY = 20;    // default step delay

int DELAY_MAX = 100;
int DELAY_MIN = 1;

uint8_t gHue = 0; // rotating "base color" used by confetti

CRGB leds[NUM_LEDS];
TBlendType currentBlending;
CRGBPalette16 currentPalette;

//BUTTON SETUP STUFF
byte prevKeyState = HIGH; // button is active low

unsigned long keyPrevMillis = 0;
const unsigned long keySampleIntervalMs = 25;
byte longKeyPressCountMax = 80; // 80 * 25 = 2000 ms
byte longKeyPressCount = 0;

int ledMode = 0; //FIRST ACTIVE MODE

//this bit is in every palette mode, needs to be in there just once
void FillLEDsFromPaletteColors(uint8_t colorIndex)
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = ColorFromPalette(currentPalette, colorIndex, BRIGHTNESS, currentBlending);
    colorIndex += STEPS;
  }
}

//BUTTON CONTROL STUFF
// called when button is kept pressed for less than 2 seconds
void shortKeyPress()
{
  Serial.println("short");
  ledMode++;
  if (ledMode > NUM_MODES)
  {
    ledMode = 0;
  }
}

// called when button is kept pressed for more than 2 seconds
void longKeyPress()
{
  Serial.println("long");
  ledMode = 888;// does a mode reset
}

// called when key goes from not pressed to pressed
void keyPress()
{
  Serial.println("key press");
  longKeyPressCount = 0;
}

// called when key goes from pressed to not pressed
void keyRelease()
{
  Serial.println("key release");
  if (longKeyPressCount >= longKeyPressCountMax)
  {
    longKeyPress();
  }
  else
  {
    shortKeyPress();
  }

  // other code goes here
}

// SOLID ----------------------------------------------------
void solid()
{
  fill_solid(leds, NUM_LEDS, CHSV(HUE, SATURATION, BRIGHTNESS));
  FastLED.show();
}

void rainbow()
{
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */

  FillLEDsFromPaletteColors(startIndex);

  FastLED.show();
  FastLED.delay(STEP_DELAY);
}

void confetti()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy(leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV(gHue + random8(64), 200, 255);
  FastLED.show();
}

//------------------SETUP------------------
void setup()
{
  // Serial.begin(9600);
  delay(3000); // power-up safety delay
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(STEP_DELAY_PIN, INPUT);
  FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  currentBlending = LINEARBLEND;
}

void updateStepDelay()
{
  int SD_PIN = analogRead(STEP_DELAY_PIN);
  STEP_DELAY = (int)max(DELAY_MIN, SD_PIN / 1024.0f * DELAY_MAX);
}

//------------------MAIN LOOP------------------
void loop()
{
  switch (ledMode)
  {
  case 999:
    break;
  case 0:
    confetti();
    break;
  case 1:
    currentPalette = OceanColors_p;
    rainbow();
    break;
  case 2:
    currentPalette = LavaColors_p;
    rainbow();
    break;
  case 3:
    currentPalette = ForestColors_p;
    rainbow();
    break;
  case 4:
    currentPalette = PartyColors_p;
    rainbow();
    break;
  case 5:
    currentPalette = RainbowColors_p;
    rainbow();
    break;
  case 6:
    currentPalette = RainbowStripeColors_p;
    rainbow();
    break;
  case 888:
    ledMode = 0;
    break;
  }

  // key management section
  if (millis() - keyPrevMillis >= keySampleIntervalMs)
  {
    keyPrevMillis = millis();

    byte currKeyState = digitalRead(BUTTON_PIN);

    if ((prevKeyState == HIGH) && (currKeyState == LOW))
    {
      keyPress();
    }
    else if ((prevKeyState == LOW) && (currKeyState == HIGH))
    {
      keyRelease();
    }
    else if (currKeyState == LOW)
    {
      longKeyPressCount++;
    }

    prevKeyState = currKeyState;
  }

  EVERY_N_MILLISECONDS(300) { updateStepDelay(); } // no need to read the step delay input every cycle
  EVERY_N_MILLISECONDS(STEP_DELAY) { gHue++; } // slowly cycle the "base color" through the rainbow
  FastLED.show();
  FastLED.delay(STEP_DELAY);
}
