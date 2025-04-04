/*  ______                              ___                         
   / ____/_______  ___  ____  __  __   /   |  ________  ____  ____ _
  / /_  / ___/ _ \/ _ \/_  / / / / /  / /| | / ___/ _ \/ __ \/ __ `/
 / __/ / /  /  __/  __/ / /_/ /_/ /  / ___ |/ /  /  __/ / / / /_/ / 
/_/ __/_/___\___/\___/ /___/\__, /  /_/  |_/_/   \___/_/ /_/\__,_/  
   / ____/ ___// /_____  __/____/___                                
  / __/  \__ \/ __/ __ \/ __ \/ ___/                                
 / /___ ___/ / /_/ /_/ / /_/ (__  )                                 
/_____//____/\__/\____/ .___/____/                                  
                     /_/                                            
*/
#include <Arduino.h>
#define FASTLED_INTERNAL        // Suppress build banner
#include <FastLED.h>

#define USE_SERIAL Serial

// Define the LED strip
#define LED_PIN        	47
#define BUTTON_PIN 46
#define NUM_LED_SRIPS  5
#define NUM_LEDS_SRIPS_L  5
#define NUM_LEDS_PER_M      30   
#define NUM_LEDS       	(NUM_LED_SRIPS * NUM_LEDS_SRIPS_L * NUM_LEDS_PER_M)
//#define NUM_LEDS 15
#define GROUPS 15
#define LEDS_PER_GROUP (NUM_LEDS / GROUPS)
#define BRIGHTNESS 255
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define DELAY_TIME 1000 // 1 second delay for each LED group

CRGB leds[NUM_LEDS];

void setup() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.clear();
    FastLED.show();
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {
      // While the button is pressed, light up all LEDs as violet
      while (digitalRead(BUTTON_PIN) == LOW) {
          for (int i = 0; i < NUM_LEDS; i++) {
              leds[i] = CRGB::Purple;
          }
          FastLED.show();
      }
      return; // Reset immediately after button is released
  }
  
  // First minute: Show all LEDs as green
  for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Green;
  }
  FastLED.show();

  // Countdown for the first minute
  for (int t = 0; t < 60; t++) {
      delay(DELAY_TIME);
      if (digitalRead(BUTTON_PIN) == LOW) return; // Reset immediately
  }
  
  // Turn off all LEDs after the first minute
  FastLED.clear();
  FastLED.show();
  
  // Countdown for the next 13 minutes, lighting up progressively
  for (int i = 0; i < 13; i++) {
      int startIndex = 0;
      int endIndex = (i + 1) * LEDS_PER_GROUP;
      
      FastLED.clear(); // Turn off all LEDs before lighting up the new ones
      for (int j = startIndex; j < endIndex; j++) {
          leds[j] = (j < NUM_LEDS / 2) ? CRGB::Blue : CRGB::Red;
      }
      FastLED.show();
      // Delay for 1 minute
      for (int t = 0; t < 60; t++) {
          delay(DELAY_TIME);
          if (digitalRead(BUTTON_PIN) == LOW) return; // Reset immediately
      }
  }
  
  // Last minute: Flash all LEDs
  for (int i = 0; i < 120; i++) {
      for (int j = 0; j < NUM_LEDS; j++) {
          leds[j] = (i % 2 == 0) ? ((j < NUM_LEDS / 2) ? CRGB::Blue : CRGB::Red) : CRGB::Black;
      }
      FastLED.show();
      delay(500);
      if (digitalRead(BUTTON_PIN) == LOW) return; // Reset immediately
  }
  
  // Turn off all LEDs and restart the cycle
  FastLED.clear();
  FastLED.show();
}
