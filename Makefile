.DEFAULT_GOAL := all
.PHONY: all station tests clean cleanstation ruby_utils

ifneq ($(filter-out radxa,$(RUBY_BUILD_ENV)),)
$(error This branch builds only the Radxa Zero 3W ground station)
endif

_CFLAGS := $(CFLAGS) -Wall -Wno-stringop-truncation -Wno-format-truncation -O2 -fdata-sections -ffunction-sections
_CPPFLAGS := $(CPPFLAGS) $(CXXFLAGS) -Wall -Wno-stringop-truncation -Wno-format-truncation -O2 -fdata-sections -ffunction-sections

LDFLAGS_CENTRAL := -L/lib/aarch64-linux-gnu -lpthread -lrt -lm
LDFLAGS_CENTRAL2 := -lpthread -lrt -lm

LDFLAGS_RENDERER := -ldrm -lcairo
CFLAGS_RENDERER := -I/usr/include/drm -I/usr/include/libdrm
CFLAGS_RENDERER += `pkg-config cairo --cflags`
_LDFLAGS := $(LDFLAGS) -lrt -lpcap -lpthread -li2c -lgpiod -Wl,--gc-sections 

_CPPFLAGS_NOSDL := $(_CPPFLAGS)
_LDFLAGS_NOSDL := $(_LDFLAGS)
_CPPFLAGS := $(_CPPFLAGS) -I/usr/include/SDL2 -D_GNU_SOURCE=1 -D_REENTRANT
_LDFLAGS := $(_LDFLAGS) -lSDL2

OBJ := build/obj
CFLAGS_RENDERER += -I/usr/include/freetype2

$(OBJ)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(_CPPFLAGS) $(CFLAGS_RENDERER) -MMD -MP -c -o $@ $<

$(OBJ)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(_CFLAGS) $(CFLAGS_RENDERER) -MMD -MP -c -o $@ $<

include mk/modules.mk
include mk/programs.mk

.PHONY: ruby_plugins
ruby_plugins:
	$(MAKE) -C plugins/osd

clean:
	$(RM) -r build/obj
	$(RM) ruby_central ruby_start ruby_i2c ruby_logger ruby_initdhcp ruby_sik_config ruby_alive ruby_video_proc ruby_dbg ruby_update ruby_update_worker ruby_controller ruby_rx_telemetry ruby_tx_rc ruby_rt_station ruby_player_radxa test_cairo test_log test_gpio test_audio2 test_socket_in test_socket_out test_udp_to_pipe test_udp_to_udp test_pipe_to_udp test_radio_to_udp test_udp_to_radio test_rtpudp_read test_port_rx test_port_tx test_link test_joystick

cleanstation: clean

-include $(shell find $(OBJ) -name '*.d' 2>/dev/null)
