HOST_CC ?= cc
HOST_CXX ?= c++
CHECK_DIR := build/host-tests
CHECKS := $(addprefix $(CHECK_DIR)/,short_packets frame_clock media_outputs session_socket session_workers)

.PHONY: check protocol-check
check: $(CHECKS)
	@set -e; for test in $(CHECKS); do "$$test"; echo "PASS $$test"; done

protocol-check: $(CHECK_DIR)/short_packets
	$<

$(CHECK_DIR):
	mkdir -p $@

$(CHECK_DIR)/short_packets: tests/unit/short_packets.c src/protocol/radiopackets_short.c \
    src/protocol/crc8.c src/protocol/radiopackets_short.h src/radio/config_radio.h src/core/base.h | $(CHECK_DIR)
	$(HOST_CC) -Wall -O2 -o $@ $(filter %.c,$^)

$(CHECK_DIR)/frame_clock: tests/unit/frame_clock.cpp src/ui/frame_clock.h | $(CHECK_DIR)
	$(HOST_CXX) -Wall -std=c++11 -O2 -o $@ $<

$(CHECK_DIR)/media_outputs: tests/unit/media_outputs.cpp src/media/fifo_output.cpp \
    src/media/fifo_output.h src/media/byte_queue.h | $(CHECK_DIR)
	$(HOST_CXX) -Wall -std=c++11 -O2 -pthread -o $@ $(filter %.cpp,$^)

$(CHECK_DIR)/session_socket: tests/unit/session_socket.cpp src/session/session_socket.cpp \
    src/session/session.h | $(CHECK_DIR)
	$(HOST_CXX) -Wall -std=c++11 -O2 -pthread -o $@ $(filter %.cpp,$^)

$(CHECK_DIR)/session_workers: tests/unit/session_workers.cpp src/session/processes.cpp src/session/session.h src/session/worker_settings.h | $(CHECK_DIR)
	$(HOST_CXX) -Wall -std=c++11 -O2 -ffunction-sections -Wl,--gc-sections -o $@ $(filter %.cpp,$^)

# The legacy model validator still references the Radxa audio and I2C adapters.
build/test_model_store: $(OBJ)/tests/unit/model_store.o $(MODULE_BASE) $(MODULE_BASE2) \
    $(MODULE_MODELS) $(MODULE_COMMON) $(MODULE_RADIO) $(OBJ)/src/platform/radxa_zero3w/hardware_audio.o \
    $(OBJ)/src/platform/radxa_zero3w/wiringPiI2C_radxa.o
	$(CXX) -o $@ $^ $(_LDFLAGS)
