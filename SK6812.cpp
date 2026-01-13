#include "SK6812.h"
#include <stdlib.h>

#define RMT_CLK_HZ 10000000  // 10 MHz → 100 ns resolution

// SK6812 timing (ns)
#define T0H 300
#define T0L 900
#define T1H 600
#define T1L 600

SK6812::SK6812(uint16_t num_leds)
{
  _count_led = num_leds;
  _pixels = (RGBW *)calloc(_count_led, sizeof(RGBW));
}

SK6812::~SK6812()
{
  free(_pixels);
}

void SK6812::set_output(uint8_t pin)
{
  _pin = pin;

  rmt_tx_channel_config_t tx_cfg = {
    .gpio_num = (gpio_num_t)_pin,
    .clk_src = RMT_CLK_SRC_DEFAULT,
    .resolution_hz = RMT_CLK_HZ,
    .mem_block_symbols = 64,
    .trans_queue_depth = 4
  };

  rmt_new_tx_channel(&tx_cfg, &_rmt_chan);

  rmt_bytes_encoder_config_t encoder_cfg{};
  
// bit 0
encoder_cfg.bit0.duration0 = T0H / 100;
encoder_cfg.bit0.level0    = 1;
encoder_cfg.bit0.duration1 = T0L / 100;
encoder_cfg.bit0.level1    = 0;

// bit 1
encoder_cfg.bit1.duration0 = T1H / 100;
encoder_cfg.bit1.level0    = 1;
encoder_cfg.bit1.duration1 = T1L / 100;
encoder_cfg.bit1.level1    = 0;

// MSB first
encoder_cfg.flags.msb_first = 1;

  rmt_new_bytes_encoder(&encoder_cfg, &_encoder);
  rmt_enable(_rmt_chan);
}

uint8_t SK6812::set_rgbw(uint16_t index, RGBW px)
{
  if (index >= _count_led) return 1;
  _pixels[index] = px;
  return 0;
}

RGBW SK6812::get_rgbw(uint16_t index)
{
  if (index >= _count_led) return {0,0,0,0};
  return _pixels[index];
}

void SK6812::sync()
{
  rmt_transmit_config_t tx_cfg = {
    .loop_count = 0
  };

  // SK6812 order: GRBW
  static uint8_t buffer[4 * 256]; // max 256 LEDs
  for (uint16_t i = 0; i < _count_led; i++) {
    buffer[i*4+0] = _pixels[i].g;
    buffer[i*4+1] = _pixels[i].r;
    buffer[i*4+2] = _pixels[i].b;
    buffer[i*4+3] = _pixels[i].w;
  }

  rmt_transmit(
    _rmt_chan,
    _encoder,
    buffer,
    _count_led * 4,
    &tx_cfg
  );

  rmt_tx_wait_all_done(_rmt_chan, portMAX_DELAY);
}