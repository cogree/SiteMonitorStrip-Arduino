#include "stripcontroller.h"

NeoPixel_StripController::NeoPixel_StripController(uint16_t num_leds, uint8_t led_driver_pin, uint16_t status_led) {  
  _status_led = status_led;
  _models = new LED_Model[num_leds];  
  _strip = new Adafruit_NeoPixel(num_leds, led_driver_pin, NEO_GRB + NEO_KHZ400);
  reset();
}

NeoPixel_StripController::~NeoPixel_StripController() {
  _strip -> ~Adafruit_NeoPixel();
  delete [] _models;
}

void NeoPixel_StripController::begin() {
  begin(70);
}

void NeoPixel_StripController::begin(uint8_t brightness) {
  _strip -> begin();
  _strip -> setBrightness(brightness);
  _strip -> show();
}

void NeoPixel_StripController::add_led(uint32_t color, uint16_t duration_secs) {
  add_led_with_blink(color, duration_secs, 0);
}

void NeoPixel_StripController::add_led_with_blink(uint32_t color, uint16_t duration_secs, uint32_t blink_interval_ms) {
  int pixelCount =  _strip -> numPixels();
  int indexSeed = random(pixelCount);
    
  for(int i = 0; i < pixelCount; ++i) {
    int index = (indexSeed + i) % pixelCount;
    LED_Model* model = &_models[index];    
    
    if(index != _status_led && model -> expired) {
      Serial.println("adding @ index " + String(index));
      // TODO - consider removing this call to millis().        
      model -> expires_at_ms = millis() + duration_secs * 1000;
      model -> expired = false;
      model -> rgb = color;
      model -> blink_interval_ms = blink_interval_ms;
      break;
    }
  }  
}

void NeoPixel_StripController::reset() {
  for(int i = 0; i < _strip -> numPixels(); ++i)
    expire_index(i);
}

void NeoPixel_StripController::set_status_led(uint32_t rgb) {
  LED_Model* model = &_models[_status_led];
  model -> rgb = rgb;
  model -> expires_at_ms = -1;
  model -> expired = false;  
  _strip -> setPixelColor(_status_led, model -> rgb);
  _strip -> show();
}

void NeoPixel_StripController::set_brightness(uint8_t brightness) {
  _strip -> setBrightness(brightness);
  _strip -> show();
}

void NeoPixel_StripController::expire_index(uint16_t index) {
  Serial.println("expiring index " + String(index));
    
  LED_Model* model = &_models[index];
  model -> expired = true;
  model -> rgb = 0;
  model -> expires_at_ms = 0;
  model -> last_blink_ms = 0;
  model -> blink_interval_ms = 0;
  
  _strip -> setPixelColor(index, 0);
}

void NeoPixel_StripController::loop() {
  unsigned long cur_time = millis();
  
  for(int i = 0; i < _strip -> numPixels(); ++i) {
    LED_Model* model = &_models[i];

    // Rules do not apply to the status LED.
    if(i == _status_led) {
      _strip -> setPixelColor(i, model -> rgb);
      continue;
    }

    // Expiration
    if(!model -> expired && model -> expires_at_ms <= cur_time)
      expire_index(i);
      
    if(model -> expired) {
       _strip -> setPixelColor(i, model -> rgb);
      continue;
    }
    
    // Case: Blinking LED
    if(model -> blink_interval_ms > 0) {
      if(cur_time - (model -> last_blink_ms) < (model -> blink_interval_ms))
        continue;  

      model -> last_blink_ms = cur_time;
      _strip -> setPixelColor(i, _strip -> getPixelColor(i) == 0 ? model -> rgb : 0);   
    } 
    // Case: Standard LED
    else {
       _strip -> setPixelColor(i, model -> rgb);
    }
  }

  _strip -> show();
}