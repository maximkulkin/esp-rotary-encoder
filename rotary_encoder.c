#include <stdlib.h>
#include <string.h>

#include "rotary_encoder.h"
#include "port.h"


struct _rotary_encoder {
    uint8_t gpio_a;
    uint8_t gpio_b;
    rotary_encoder_callback_fn callback;
    void* context;

    uint8_t last_state;
    uint8_t position;

    struct _rotary_encoder *next;
};


static SemaphoreHandle_t rotary_encoders_lock = NULL;
static rotary_encoder_t *rotary_encoders = NULL;
static TimerHandle_t rotary_encoder_timer = NULL;
static bool rotary_encoders_initialized = false;


static rotary_encoder_t *rotary_encoder_find_by_gpio(uint8_t gpio_a, uint8_t gpio_b) {
    rotary_encoder_t *rotary_encoder = rotary_encoders;
    while (rotary_encoder && !(rotary_encoder->gpio_a == gpio_a && rotary_encoder->gpio_b == gpio_b))
        rotary_encoder = rotary_encoder->next;

    return rotary_encoder;
}


static void rotary_encoder_process(rotary_encoder_t *rotary_encoder) {
    uint8_t state = my_gpio_read(rotary_encoder->gpio_a);
    if (state == rotary_encoder->last_state)
        return;

    rotary_encoder->last_state = state;
    if (my_gpio_read(rotary_encoder->gpio_b) != state) {
        rotary_encoder->position++;
    } else {
        rotary_encoder->position--;
    }

    rotary_encoder->callback(rotary_encoder->position, rotary_encoder->context);
}


static void rotary_encoder_timer_callback(TimerHandle_t timer) {
    if (xSemaphoreTake(rotary_encoders_lock, 0) != pdTRUE)
        return;

    rotary_encoder_t *rotary_encoder = rotary_encoders;

    while (rotary_encoder) {
        rotary_encoder_process(rotary_encoder);
        rotary_encoder = rotary_encoder->next;
    }

    xSemaphoreGive(rotary_encoders_lock);
}


static int rotary_encoders_init() {
    if (!rotary_encoders_initialized) {
        rotary_encoders_lock = xSemaphoreCreateBinary();
        xSemaphoreGive(rotary_encoders_lock);

        rotary_encoder_timer = xTimerCreate(
            "Rotary encoder timer", 1, pdTRUE, NULL, rotary_encoder_timer_callback
        );

        rotary_encoders_initialized = true;
    }

    return 0;
}


rotary_encoder_t *rotary_encoder_create(const uint8_t gpio_a, const uint8_t gpio_b, rotary_encoder_callback_fn callback, void* context) {
    if (!rotary_encoders_initialized)
        rotary_encoders_init();

    rotary_encoder_t *rotary_encoder = rotary_encoder_find_by_gpio(gpio_a, gpio_b);
    if (rotary_encoder)
        return NULL;

    rotary_encoder = malloc(sizeof(rotary_encoder_t));
    memset(rotary_encoder, 0, sizeof(*rotary_encoder));
    rotary_encoder->gpio_a = gpio_a;
    rotary_encoder->gpio_b = gpio_b;
    rotary_encoder->callback = callback;
    rotary_encoder->context = context;
    rotary_encoder->last_state = my_gpio_read(rotary_encoder->gpio_a);
    rotary_encoder->position = 0;

    my_gpio_enable(rotary_encoder->gpio_a);
    my_gpio_enable(rotary_encoder->gpio_b);

    rotary_encoder->next = rotary_encoders;

    xSemaphoreTake(rotary_encoders_lock, portMAX_DELAY);

    rotary_encoders = rotary_encoder;

    xSemaphoreGive(rotary_encoders_lock);

    if (!xTimerIsTimerActive(rotary_encoder_timer)) {
        xTimerStart(rotary_encoder_timer, 1);
    }

    return rotary_encoder;
}


void rotary_encoder_delete(rotary_encoder_t *rotary_encoder) {
    if (!rotary_encoders_initialized)
        rotary_encoders_init();

    xSemaphoreTake(rotary_encoders_lock, portMAX_DELAY);

    if (!rotary_encoders) {
        xSemaphoreGive(rotary_encoders_lock);
        return;
    }

    if (rotary_encoders == rotary_encoder) {
        rotary_encoders = rotary_encoders->next;
    } else {
        rotary_encoder_t *b = rotary_encoders;
        while (b->next) {
            if (b->next == rotary_encoder) {
                b->next = b->next->next;
                break;
            }
        }
    }

    if (!rotary_encoders) {
        xTimerStop(rotary_encoder_timer, 1);
    }

    xSemaphoreGive(rotary_encoders_lock);

    if (!rotary_encoder)
        return;

    free(rotary_encoder);
}

uint8_t rotary_encoder_get_position(rotary_encoder_t *rotary_encoder) {
    return rotary_encoder->position;
}

void rotary_encoder_set_position(rotary_encoder_t *rotary_encoder, uint8_t position) {
    rotary_encoder->position = position;
}
