#ifndef SK6812_H_
#define SK6812_H_

#pragma once
#include <Arduino.h>
#include "driver/rmt_tx.h"

struct RGBW {
  uint8_t r, g, b, w;
};

class SK6812 {
public:
  SK6812(uint16_t num_leds);
  ~SK6812();

  void set_output(uint8_t pin);
  uint8_t set_rgbw(uint16_t index, RGBW px_value);
  RGBW get_rgbw(uint16_t index);
  void sync();

private:
  uint16_t _count_led;
  RGBW *_pixels;
  uint8_t _pin;

  rmt_channel_handle_t _rmt_chan;
  rmt_encoder_handle_t _encoder;
};

#endif /* SK6812_H_ */