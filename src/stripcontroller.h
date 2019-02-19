#ifndef stripcontroller_H_   /* Include guard */
#define stripcontroller_H_

#include <Adafruit_NeoPixel.h>

#if (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #include <WProgram.h>
 #include <pins_arduino.h>
#endif

#ifndef COLOR_RED
  #define COLOR_RED 0x00FF0000
#endif
#ifndef COLOR_GREEN
  #define COLOR_GREEN 0x0000FF00
#endif
#ifndef COLOR_BLUE
  #define LED_COLOR_BLUE 0x000000FF
#endif
#ifndef COLOR_YELLOW
  #define COLOR_YELLOW 0x00FFFF00
#endif
#ifndef COLOR_PURPLE
  #define COLOR_PURPLE 0x00800080
#endif

struct LED_Model {
 volatile uint32_t rgb = 0;
 volatile long expires_at_ms = -1;
 volatile bool expired = true;
 volatile uint32_t blink_interval_ms = 600;
 volatile uint64_t last_blink_ms = 0;
};

class NeoPixel_StripController {
  public:
     NeoPixel_StripController(uint16_t num_leds, uint8_t led_driver_pin, uint16_t status_led_pin);
    ~NeoPixel_StripController(void);
    void begin(void);
    void begin(uint8_t brightness);
    void set_brightness(uint8_t brightness);
    void reset(void);
    void add_led (uint32_t rgb, uint16_t duration_secs);
    void add_led_with_blink (uint32_t rgb, uint16_t duration_secs, uint32_t blink_interval_ms);
    void set_status_led(uint32_t rgb);
    void loop(void);
    
  private:
    LED_Model             *_models;
    unsigned short        _status_led;
    Adafruit_NeoPixel     *_strip;
    

    void expire_index(uint16_t);
};

#endif //stripcontroller_H_
