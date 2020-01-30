#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct _rotary_encoder rotary_encoder_t;

typedef void (*rotary_encoder_callback_fn)(uint8_t position, void* context);

rotary_encoder_t *rotary_encoder_create(uint8_t gpio_a, uint8_t gpio_b, rotary_encoder_callback_fn callback, void* context);
void rotart_encoder_delete(rotary_encoder_t *rotary_encoder);

uint8_t rotary_encoder_get_position(rotary_encoder_t *rotary_encoder);
void rotary_encoder_set_position(rotary_encoder_t *rotary_encoder, uint8_t position);
