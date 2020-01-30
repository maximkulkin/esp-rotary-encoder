PROGRAM = rotary_encoder_example

A_GPIO ?= 4
B_GPIO ?= 5

EXTRA_CFLAGS += -DA_GPIO=$(A_GPIO) -DB_GPIO=$(B_GPIO)

EXTRA_COMPONENTS = $(abspath ..)

include $(SDK_PATH)/common.mk

monitor:
	$(FILTEROUTPUT) --port $(ESPPORT) --baud 115200 --elf $(PROGRAM_OUT)
