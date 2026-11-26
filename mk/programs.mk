all: station

station: ruby_start ruby_utils ruby_i2c ruby_controller ruby_central ruby_rt_station ruby_tx_rc \
    ruby_rx_telemetry ruby_player_radxa

ruby_central: $(OBJ)/src/ui/update/vehicle_update.o \
    $(OBJ)/src/session/session_socket.o $(OBJ)/src/ui/frame_scheduler.o \
    $(OBJ)/src/ui/ruby_central.o $(MODULE_BASE) $(MODULE_MODELS) $(MODULE_COMMON) $(MODULE_BASE2) \
    $(UI_MENUS_ALL) $(CENTRAL_RENDER_CODE) $(CENTRAL_POPUP_ALL) $(CENTRAL_RENDER_ALL) \
    $(CENTRAL_OSD_ALL) $(CENTRAL_OLED_ALL) $(CENTRAL_ALL) $(CENTRAL_RADIO) \
    $(OBJ)/src/ipc/shared_mem_controller_only.o $(OBJ)/src/platform/radxa_zero3w/hdmi.o \
    $(OBJ)/src/storage/favorites.o $(OBJ)/src/settings/plugins_settings.o \
    $(OBJ)/src/settings/core_plugins_settings.o $(OBJ)/src/platform/radxa_zero3w/hardware_files.o \
    $(OBJ)/src/storage/models_connect_frequencies.o $(OBJ)/src/ipc/shared_mem_i2c.o \
    $(OBJ)/src/media/video_capture_res.o $(OBJ)/src/platform/radxa_zero3w/wiringPiI2C_radxa.o \
    $(OBJ)/src/platform/radxa_zero3w/hardware_audio.o
	$(CXX) $(_CPPFLAGS) $(CFLAGS_RENDERER) -export-dynamic -o $@ $^ $(_LDFLAGS) -ldl $(LDFLAGS_CENTRAL) $(LDFLAGS_CENTRAL2) $(LDFLAGS_RENDERER)


ruby_utils: ruby_logger ruby_initdhcp ruby_sik_config ruby_alive ruby_video_proc ruby_update \
    ruby_update_worker ruby_dbg

ruby_start: $(OBJ)/src/apps/startup/ruby_start.o $(MODULE_LOC) $(OBJ)/src/apps/startup/r_test.o \
    $(OBJ)/src/apps/startup/r_initradio.o $(OBJ)/src/apps/startup/first_boot.o \
    $(OBJ)/src/media/parser_h264.o $(OBJ)/src/platform/radxa_zero3w/hardware_audio.o \
    $(OBJ)/src/platform/radxa_zero3w/hardware_procs.o $(OBJ)/src/media/camera_utils.o \
    $(OBJ)/src/protocol/commands.o $(OBJ)/src/settings/vehicle_settings.o \
    $(OBJ)/src/platform/radxa_zero3w/hardware_radio_txpower.o $(OBJ)/src/protocol/radiopackets2.o \
    $(OBJ)/src/settings/ctrl_preferences.o $(OBJ)/src/settings/ctrl_interfaces.o \
    $(MODULE_MINIMUM_BASE) $(MODULE_MODELS) $(MODULE_MINIMUM_COMMON) $(OBJ)/src/ipc/ruby_ipc.o \
    $(OBJ)/src/settings/ctrl_settings.o $(OBJ)/src/settings/utils_controller.o $(OBJ)/src/core/utils.o \
    $(OBJ)/src/ipc/shared_mem.o $(OBJ)/src/core/encr.o  $(OBJ)/src/settings/core_plugins_settings.o \
    $(OBJ)/src/platform/radxa_zero3w/hardware_files.o $(OBJ)/src/radio/tx_powers.o \
    $(OBJ)/src/platform/radxa_zero3w/wiringPiI2C_radxa.o $(OBJ)/src/settings/utils_vehicle.o
	$(CXX) $(_CPPFLAGS) -o $@ $^ $(_LDFLAGS) -ldl

ruby_i2c: $(OBJ)/src/apps/i2c/ruby_i2c.o $(MODULE_BASE) $(MODULE_MODELS) $(MODULE_COMMON) \
    $(MODULE_BASE2) $(OBJ)/src/ipc/shared_mem_i2c.o \
    $(OBJ)/src/platform/radxa_zero3w/wiringPiI2C_radxa.o
	$(CXX) $(_CPPFLAGS) -o $@ $^ $(_LDFLAGS)

ruby_logger: $(OBJ)/src/apps/tools/ruby_logger.o $(MODULE_BASE)
	$(CXX) $(_CPPFLAGS) -o $@ $^ $(_LDFLAGS)

ruby_initdhcp: $(OBJ)/src/apps/tools/ruby_initdhcp.o $(MODULE_BASE) $(MODULE_BASE2) \
    $(MODULE_MODELS) $(MODULE_COMMON)
	$(CXX) $(_CPPFLAGS) -o $@ $^ $(_LDFLAGS)

ruby_sik_config: $(OBJ)/src/apps/tools/ruby_sik_config.o $(MODULE_BASE) $(MODULE_BASE2) \
    $(MODULE_MODELS) $(MODULE_COMMON)
	$(CXX) $(_CPPFLAGS) -o $@ $^ $(_LDFLAGS) -ldl

ruby_alive: $(OBJ)/src/apps/tools/ruby_alive.o $(MODULE_BASE) $(MODULE_BASE2) $(MODULE_MODELS) \
    $(MODULE_COMMON)
	$(CXX) $(_CPPFLAGS) -o $@ $^ $(_LDFLAGS)

ruby_video_proc: $(OBJ)/src/apps/tools/ruby_video_proc.o $(MODULE_BASE) $(MODULE_BASE2) \
    $(MODULE_MODELS) $(MODULE_COMMON) $(OBJ)/src/platform/radxa_zero3w/hardware_audio.o \
    $(OBJ)/src/platform/radxa_zero3w/wiringPiI2C_radxa.o
	$(CXX) $(_CPPFLAGS) -o $@ $^ $(_LDFLAGS)

ruby_dbg: $(OBJ)/src/apps/tools/ruby_dbg.o $(MODULE_BASE) $(MODULE_BASE2) $(MODULE_MODELS) \
    $(MODULE_COMMON) $(OBJ)/src/settings/vehicle_settings.o
	$(CXX) $(_CPPFLAGS) -o $@ $^ $(_LDFLAGS)

ruby_update: $(OBJ)/src/apps/tools/ruby_update.o $(MODULE_BASE) $(MODULE_MODELS) \
    $(OBJ)/src/settings/ctrl_settings.o $(OBJ)/src/settings/ctrl_preferences.o \
    $(OBJ)/src/core/string_utils.o $(OBJ)/src/platform/radxa_zero3w/gpio.o \
    $(OBJ)/src/platform/radxa_zero3w/hardware_audio.o \
    $(OBJ)/src/platform/radxa_zero3w/wiringPiI2C_radxa.o
	$(CXX) $(_CPPFLAGS) -o $@ $^ $(_LDFLAGS)

ruby_update_worker: $(OBJ)/src/apps/tools/ruby_update_worker.o $(MODULE_BASE) $(MODULE_BASE2) \
    $(MODULE_MODELS) $(MODULE_COMMON)
	$(CXX) $(_CPPFLAGS_NOSDL) -o $@ $^ $(_LDFLAGS_NOSDL)

ruby_controller: $(OBJ)/src/platform/radxa_zero3w/wiringPiI2C_radxa.o \
    $(OBJ)/src/platform/radxa_zero3w/hardware_audio.o $(OBJ)/src/session/session_socket.o $(OBJ)/src/session/processes.o $(OBJ)/src/session/saved_session.o \
    $(OBJ)/src/session/ruby_controller.o $(MODULE_BASE) $(MODULE_BASE2) \
    $(MODULE_COMMON) $(MODULE_RADIO) $(MODULE_MODELS) $(MODULE_STATION)
	$(CXX) $(_CPPFLAGS) -o $@ $^ $(_LDFLAGS)

ruby_rx_telemetry: $(OBJ)/src/apps/link_services/ruby_rx_telemetry.o $(MODULE_BASE) \
    $(MODULE_BASE2) $(MODULE_COMMON) $(MODULE_RADIO) $(MODULE_MODELS) $(MODULE_STATION) \
    $(OBJ)/src/platform/radxa_zero3w/hardware_audio.o \
    $(OBJ)/src/platform/radxa_zero3w/wiringPiI2C_radxa.o
	$(CXX) $(_CPPFLAGS) -o $@ $^ $(_LDFLAGS)

ruby_tx_rc: $(OBJ)/src/apps/link_services/ruby_tx_rc.o $(MODULE_BASE) $(MODULE_BASE2) \
    $(MODULE_COMMON) $(MODULE_RADIO) $(MODULE_MODELS) $(MODULE_STATION) \
    $(OBJ)/src/ipc/shared_mem_i2c.o $(OBJ)/src/platform/radxa_zero3w/hardware_audio.o \
    $(OBJ)/src/platform/radxa_zero3w/wiringPiI2C_radxa.o
	$(CXX) $(_CPPFLAGS) -o $@ $^ $(_LDFLAGS)

ruby_rt_station: $(OBJ)/src/media/fifo_output.o $(OBJ)/src/radio/station/ruby_rt_station.o $(MODULE_BASE) $(MODULE_BASE2) \
    $(MODULE_COMMON) $(MODULE_RADIO) $(MODULE_MODELS) $(MODULE_STATION) \
    $(OBJ)/src/radio/station/packets_utils.o $(OBJ)/src/radio/station/process_local_packets.o \
    $(OBJ)/src/radio/station/process_radio_in_packets.o \
    $(OBJ)/src/radio/station/process_radio_out_packets.o $(OBJ)/src/radio/station/periodic_loop.o \
    $(OBJ)/src/media/receiver/processor_rx_audio.o $(OBJ)/src/media/receiver/processor_rx_video.o \
    $(OBJ)/src/media/receiver/video_rx_buffers.o $(OBJ)/src/radio/station/radio_links.o \
    $(OBJ)/src/radio/station/relay_rx.o $(OBJ)/src/radio/station/test_link_params.o \
    $(OBJ)/src/media/receiver/process_video_packets.o $(OBJ)/src/media/receiver/rx_video_output.o \
    $(OBJ)/src/media/recording/rx_video_recording.o \
    $(OBJ)/src/media/recording/rx_video_recording_data.o $(OBJ)/src/ipc/shared_mem_controller_only.o \
    $(OBJ)/src/storage/models_connect_frequencies.o $(OBJ)/src/protocol/parse_fc_telemetry.o \
    $(OBJ)/src/protocol/parse_fc_telemetry_ltm.o $(OBJ)/src/radio/station/radio_links_sik.o \
    $(OBJ)/src/radio/radio_utils.o $(OBJ)/src/settings/core_plugins_settings.o \
    $(OBJ)/src/media/camera_utils.o  $(OBJ)/src/media/parser_h264.o $(OBJ)/src/radio/tx_powers.o \
    $(OBJ)/src/settings/utils_controller.o $(OBJ)/src/settings/utils_vehicle.o \
    $(OBJ)/src/radio/station/generic_rx_ecbuffers.o $(OBJ)/src/platform/radxa_zero3w/hardware_audio.o \
    $(OBJ)/src/platform/radxa_zero3w/wiringPiI2C_radxa.o $(OBJ)/src/protocol/msp.o
	$(CXX) $(_CPPFLAGS) -o $@ $^ $(_LDFLAGS) -ldl

ruby_player_radxa:$(OBJ)/src/media/player/ruby_player_radxa.o \
    $(OBJ)/src/platform/radxa_zero3w/video/mpp_core.o $(OBJ)/src/platform/radxa_zero3w/hdmi.o \
    $(OBJ)/src/settings/ctrl_settings.o $(OBJ)/src/ipc/shared_mem.o $(OBJ)/src/media/parser_h264.o \
    $(CENTRAL_RENDER_CODE) $(MODULE_MINIMUM_BASE) $(MODULE_MINIMUM_COMMON)
	$(CXX) $(_CPPFLAGS) $(CFLAGS_RENDERER) -o $@ $^ $(_LDFLAGS) $(LDFLAGS_RENDERER) $(LDFLAGS_CENTRAL) $(LDFLAGS_CENTRAL2) -ldl -lc -lrockchip_mpp

tests: test_port_rx test_port_tx test_link

test_cairo:$(OBJ)/tests/diagnostics/test_cairo.o $(MODULE_BASE) $(MODULE_BASE2) $(MODULE_COMMON) \
    $(MODULE_RADIO) $(MODULE_MODELS)
	$(CXX) $(_CPPFLAGS) -o $@ $^ $(_LDFLAGS) -ldl -lc

test_log:$(OBJ)/tests/diagnostics/test_log.o $(OBJ)/sdk/utils/core_plugins_utils.o $(MODULE_BASE) \
    $(MODULE_BASE2) $(MODULE_COMMON) $(MODULE_RADIO) $(MODULE_MODELS)
	$(CXX) $(_CPPFLAGS) -o $@ $^ $(_LDFLAGS) -ldl -lc

test_gpio:$(OBJ)/tests/diagnostics/test_gpio.o $(MODULE_BASE) $(MODULE_BASE2) $(MODULE_COMMON) \
    $(MODULE_RADIO) $(MODULE_MODELS)
	$(CXX) $(_CPPFLAGS) -o $@ $^ $(_LDFLAGS) -ldl -lc

test_audio2:$(OBJ)/tests/diagnostics/test_audio2.o $(MODULE_BASE) $(MODULE_BASE2) $(MODULE_COMMON) \
    $(MODULE_RADIO) $(MODULE_MODELS)
	$(CXX) $(_CPPFLAGS) -o $@ $^ $(_LDFLAGS) -ldl -lc

test_socket_in:$(OBJ)/tests/diagnostics/test_socket_in.o $(MODULE_BASE) $(MODULE_BASE2) \
    $(MODULE_COMMON) $(MODULE_RADIO) $(MODULE_MODELS)
	$(CXX) $(_CPPFLAGS) -o $@ $^ $(_LDFLAGS) -ldl -lc

test_socket_out:$(OBJ)/tests/diagnostics/test_socket_out.o $(MODULE_BASE) $(MODULE_BASE2) \
    $(MODULE_COMMON) $(MODULE_RADIO) $(MODULE_MODELS)
	$(CXX) $(_CPPFLAGS) -o $@ $^ $(_LDFLAGS) -ldl -lc

test_udp_to_pipe:$(OBJ)/tests/diagnostics/test_udp_to_pipe.o $(MODULE_BASE) $(MODULE_BASE2) \
    $(MODULE_COMMON) $(MODULE_RADIO) $(MODULE_MODELS)
	$(CXX) $(_CPPFLAGS) -o $@ $^ $(_LDFLAGS) -ldl -lc

test_udp_to_udp:$(OBJ)/tests/diagnostics/test_udp_to_udp.o $(MODULE_BASE) $(MODULE_BASE2) \
    $(MODULE_COMMON) $(MODULE_RADIO) $(MODULE_MODELS)
	$(CXX) $(_CPPFLAGS) -o $@ $^ $(_LDFLAGS) -ldl -lc

test_pipe_to_udp:$(OBJ)/tests/diagnostics/test_pipe_to_udp.o $(MODULE_BASE) $(MODULE_BASE2) \
    $(MODULE_COMMON) $(MODULE_RADIO) $(MODULE_MODELS)
	$(CXX) $(_CPPFLAGS) -o $@ $^ $(_LDFLAGS) -ldl -lc

test_radio_to_udp:$(OBJ)/tests/diagnostics/test_radio_to_udp.o $(MODULE_BASE) $(MODULE_BASE2) \
    $(MODULE_COMMON) $(MODULE_RADIO) $(MODULE_MODELS)
	$(CXX) $(_CPPFLAGS) -o $@ $^ $(_LDFLAGS) -ldl -lc

test_udp_to_radio:$(OBJ)/tests/diagnostics/test_udp_to_radio.o $(MODULE_BASE) $(MODULE_BASE2) \
    $(MODULE_COMMON) $(MODULE_RADIO) $(MODULE_MODELS)
	$(CXX) $(_CPPFLAGS) -o $@ $^ $(_LDFLAGS) -ldl -lc

test_rtpudp_read:$(OBJ)/tests/diagnostics/test_rtpudp_read.o $(MODULE_BASE) $(MODULE_BASE2) \
    $(MODULE_COMMON) $(MODULE_RADIO) $(MODULE_MODELS)
	$(CXX) $(_CPPFLAGS) -o $@ $^ $(_LDFLAGS) -ldl -lc

test_port_rx:$(OBJ)/tests/diagnostics/test_port_rx.o $(MODULE_BASE) $(MODULE_BASE2) \
    $(MODULE_COMMON) $(MODULE_RADIO) $(MODULE_MODELS)
	$(CXX) $(_CPPFLAGS) -o $@ $^ $(_LDFLAGS) -ldl -lc

test_port_tx:$(OBJ)/tests/diagnostics/test_port_tx.o $(MODULE_BASE) $(MODULE_BASE2) \
    $(MODULE_COMMON) $(MODULE_RADIO) $(MODULE_MODELS)
	$(CXX) $(_CPPFLAGS) -o $@ $^ $(_LDFLAGS) -ldl -lc


test_link:$(OBJ)/tests/diagnostics/test_link.o $(MODULE_BASE) $(MODULE_BASE2) $(MODULE_COMMON) \
    $(MODULE_RADIO) $(MODULE_MODELS)
	$(CXX) $(_CPPFLAGS) -o $@ $^ $(_LDFLAGS) -ldl -lc

test_joystick:$(OBJ)/tests/diagnostics/test_joystick.o $(MODULE_BASE) $(MODULE_BASE2) \
    $(MODULE_COMMON) $(MODULE_RADIO) $(MODULE_MODELS)
	$(CXX) $(_CPPFLAGS) -o $@ $^ $(_LDFLAGS) -ldl -lc

