CC ?= gcc

FW_ROOT := ../../Software/Apollo\ -\ DSP
CORE_INC := $(FW_ROOT)/Core/Inc

CFLAGS ?= -std=c11 -Wall -Wextra -Werror -I$(CORE_INC)

SOURCES := \
	$(FW_ROOT)/Core/Src/apollo_status.c \
	$(FW_ROOT)/Core/Src/dsp/dsp_filters.c \
	$(FW_ROOT)/Core/Src/app/apollo_signal_chain.c \
	$(FW_ROOT)/Core/Src/app/apollo_diagnostics.c \
	$(FW_ROOT)/Core/Src/app/apollo_cli.c \
	$(FW_ROOT)/Core/Src/app/apollo_telemetry.c \
	$(FW_ROOT)/Core/Src/app/apollo_sd_format.c \
	$(FW_ROOT)/Core/Src/drivers/sd_card_proto.c \
	test_runner.c

BUILD_DIR := build
TARGET := $(BUILD_DIR)/apollo_host_tests

.PHONY: all test clean

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(SOURCES) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

test: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
