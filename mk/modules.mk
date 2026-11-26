CENTRAL_RENDER_CODE := $(OBJ)/src/ui/renderer/render_engine.o \
    $(OBJ)/src/ui/renderer/render_engine_cairo.o $(OBJ)/src/ui/plugins/render_engine_ui.o \
    $(OBJ)/src/platform/radxa_zero3w/display/drm_core.o
MODULE_LOC := $(OBJ)/src/core/i18n/strings_loc.o $(OBJ)/src/core/i18n/strings_table.o 

MODULE_MINIMUM_BASE := $(OBJ)/src/core/base.o $(OBJ)/src/core/config.o \
    $(OBJ)/src/radio/config_radio.o $(OBJ)/src/platform/radxa_zero3w/gpio.o \
    $(OBJ)/src/platform/radxa_zero3w/hardware_i2c.o \
    $(OBJ)/src/platform/radxa_zero3w/hardware_radio_sik.o \
    $(OBJ)/src/platform/radxa_zero3w/hardware_radio_serial.o \
    $(OBJ)/src/platform/radxa_zero3w/hardware_serial.o $(OBJ)/src/platform/radxa_zero3w/hardware.o \
    $(OBJ)/src/platform/radxa_zero3w/hardware_radio.o \
    $(OBJ)/src/platform/radxa_zero3w/hardware_radio_txpower.o \
    $(OBJ)/src/platform/radxa_zero3w/hardware_procs.o
MODULE_MINIMUM_COMMON := $(OBJ)/src/core/string_utils.o
MODULE_BASE := $(OBJ)/src/core/base.o $(OBJ)/src/ipc/shared_mem.o $(OBJ)/src/core/config.o \
    $(OBJ)/src/radio/config_radio.o $(OBJ)/src/platform/radxa_zero3w/hardware.o \
    $(OBJ)/src/platform/radxa_zero3w/hardware_files.o \
    $(OBJ)/src/platform/radxa_zero3w/hardware_procs.o $(OBJ)/src/core/utils.o $(OBJ)/src/core/encr.o \
    $(OBJ)/src/platform/radxa_zero3w/hardware_i2c.o $(OBJ)/src/platform/radxa_zero3w/hardware_radio.o \
    $(OBJ)/src/platform/radxa_zero3w/hardware_radio_serial.o \
    $(OBJ)/src/platform/radxa_zero3w/hardware_serial.o \
    $(OBJ)/src/platform/radxa_zero3w/hardware_radio_sik.o \
    $(OBJ)/src/platform/radxa_zero3w/hardware_radio_txpower.o $(OBJ)/src/ipc/ruby_ipc.o \
    $(OBJ)/src/platform/radxa_zero3w/hardware_files.o
MODULE_BASE2 := $(OBJ)/src/platform/radxa_zero3w/gpio.o $(OBJ)/src/settings/ctrl_settings.o \
    $(OBJ)/src/settings/utils_controller.o $(OBJ)/src/settings/utils_vehicle.o \
    $(OBJ)/src/ipc/controller_rt_info.o $(OBJ)/src/ipc/vehicle_rt_info.o \
    $(OBJ)/src/settings/ctrl_preferences.o $(OBJ)/src/settings/ctrl_interfaces.o \
    $(OBJ)/src/core/alarms.o $(OBJ)/src/protocol/commands.o
MODULE_COMMON := $(OBJ)/src/core/string_utils.o $(OBJ)/src/radio/relay_utils.o
MODULE_MODELS := $(OBJ)/src/settings/models.o $(OBJ)/src/settings/models_list.o
MODULE_RADIO := $(OBJ)/src/radio/radio_stats.o $(OBJ)/src/radio/link/fec.o \
    $(OBJ)/src/radio/link/radio_duplicate_det.o $(OBJ)/src/radio/link/radio_rx.o \
    $(OBJ)/src/radio/link/radio_tx.o $(OBJ)/src/radio/link/radiolink.o \
    $(OBJ)/src/protocol/radiopackets_rc.o $(OBJ)/src/protocol/radiopackets_short.o \
    $(OBJ)/src/protocol/radiopackets2.o $(OBJ)/src/protocol/radiopacketsqueue.o \
    $(OBJ)/src/radio/link/radiotap.o
MODULE_STATION := $(OBJ)/src/radio/station/shared_vars.o \
    $(OBJ)/src/radio/station/shared_vars_state.o $(OBJ)/src/radio/station/timers.o \
    $(OBJ)/src/media/receiver/adaptive_video.o


CENTRAL_MENU_ITEMS_ALL := $(OBJ)/src/ui/menu/widgets/menu_items.o \
    $(OBJ)/src/ui/menu/widgets/menu_item_select_base.o $(OBJ)/src/ui/menu/widgets/menu_item_select.o \
    $(OBJ)/src/ui/menu/widgets/menu_item_slider.o $(OBJ)/src/ui/menu/widgets/menu_item_range.o \
    $(OBJ)/src/ui/menu/widgets/menu_item_edit.o $(OBJ)/src/ui/menu/widgets/menu_item_section.o \
    $(OBJ)/src/ui/menu/widgets/menu_item_text.o $(OBJ)/src/ui/menu/widgets/menu_item_legend.o \
    $(OBJ)/src/ui/menu/widgets/menu_item_checkbox.o $(OBJ)/src/ui/menu/widgets/menu_item_radio.o
CENTRAL_MENU_ALL1 := $(OBJ)/src/ui/menu/menu.o $(OBJ)/src/ui/menu/menu_objects.o \
    $(OBJ)/src/ui/menu/menu_preferences_buttons.o $(OBJ)/src/ui/menu/menu_root.o \
    $(OBJ)/src/ui/menu/menu_search.o $(OBJ)/src/ui/menu/menu_spectator.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicles.o $(OBJ)/src/ui/menu/vehicle/menu_vehicle.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_general.o $(OBJ)/src/ui/menu/vehicle/menu_vehicle_expert.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_video.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_video_bidir.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_camera.o $(OBJ)/src/ui/menu/vehicle/menu_vehicle_osd.o \
    $(OBJ)/src/ui/menu/menu_confirmation.o $(OBJ)/src/ui/menu/menu_storage.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_telemetry.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_video_encodings.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_video_compare.o $(OBJ)/src/ui/menu/menu_text.o \
    $(OBJ)/src/ui/menu/menu_tx_raw_power.o $(OBJ)/src/ui/menu/menu_calibrate_hdmi.o
CENTRAL_MENU_ALL2 := $(OBJ)/src/ui/menu/vehicle/menu_vehicle_relay.o \
    $(OBJ)/src/ui/menu/controller/menu_controller.o $(OBJ)/src/ui/menu/vehicle/menu_vehicle_alarms.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_camera_gains.o \
    $(OBJ)/src/ui/menu/controller/menu_controller_peripherals.o \
    $(OBJ)/src/ui/menu/controller/menu_controller_expert.o \
    $(OBJ)/src/ui/menu/controller/menu_controller_cpu_priorities.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_cpu_priorities.o $(OBJ)/src/ui/menu/menu_system.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_osd_instruments.o $(OBJ)/src/ui/menu/menu_preferences_ui.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_video_profile.o \
    $(OBJ)/src/ui/menu/menu_confirmation_import.o $(OBJ)/src/ui/menu/menu_system_alarms.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_selector.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_osd_widgets.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_osd_widget.o $(OBJ)/src/ui/menu/vehicle/menu_vehicle_dev.o \
    $(OBJ)/src/ui/menu/controller/menu_controller_dev.o \
    $(OBJ)/src/ui/menu/controller/menu_controller_dev_stats.o
CENTRAL_MENU_ALL3 := $(OBJ)/src/ui/menu/vehicle/menu_vehicle_management.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_import.o $(OBJ)/src/ui/menu/menu_switch_vehicle.o \
    $(OBJ)/src/ui/menu/controller/menu_controller_joystick.o \
    $(OBJ)/src/ui/menu/menu_system_all_params.o $(OBJ)/src/ui/menu/menu_color_picker.o \
    $(OBJ)/src/ui/menu/controller/menu_controller_video.o \
    $(OBJ)/src/ui/menu/controller/menu_controller_telemetry.o $(OBJ)/src/ui/menu/menu_update_vehicle.o \
    $(OBJ)/src/ui/menu/menu_device_i2c.o $(OBJ)/src/ui/menu/vehicle/menu_vehicle_osd_stats.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_audio.o $(OBJ)/src/ui/menu/menu_channels_select.o \
    $(OBJ)/src/ui/menu/menu_system_dev_logs.o $(OBJ)/src/ui/menu/widgets/menu_item_vehicle.o \
    $(OBJ)/src/ui/menu/menu_radio_config.o $(OBJ)/src/ui/menu/menu_preferences.o
CENTRAL_MENU_ALL4 := $(OBJ)/src/ui/menu/vehicle/menu_vehicle_data_link.o \
    $(OBJ)/src/ui/menu/controller/menu_controller_network.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_osd_plugins.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_instruments_general.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_osd_elements.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_osd_plugin.o \
    $(OBJ)/src/ui/menu/controller/menu_controller_plugins.o \
    $(OBJ)/src/ui/menu/controller/menu_controller_encryption.o \
    $(OBJ)/src/ui/menu/menu_search_connect.o $(OBJ)/src/ui/menu/menu_system_hardware.o \
    $(OBJ)/src/ui/menu/controller/menu_controller_recording.o $(OBJ)/src/ui/menu/menu_info_booster.o \
    $(OBJ)/src/ui/menu/menu_info_procs.o \
    $(OBJ)/src/ui/menu/controller/menu_controller_radio_interface.o \
    $(OBJ)/src/ui/menu/menu_negociate_radio.o $(OBJ)/src/ui/menu/menu_about.o
CENTRAL_MENU_ALL5 := $(OBJ)/src/ui/menu/menu_system_dev_stats.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_radio_link.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_radio_interface.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_management_plugins.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_peripherals.o \
    $(OBJ)/src/ui/menu/menu_confirmation_delete_logs.o $(OBJ)/src/ui/menu/vehicle/menu_vehicle_radio.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_cpu_oipc.o \
    $(OBJ)/src/ui/menu/menu_confirmation_vehicle_board.o
CENTRAL_MENU_ALL6 := $(OBJ)/src/ui/menu/controller/menu_controller_radio.o \
    $(OBJ)/src/ui/menu/menu_confirmation_video_rate.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_simplesetup.o \
    $(OBJ)/src/ui/menu/menu_confirmation_sdcard_update.o \
    $(OBJ)/src/ui/menu/controller/menu_controller_update.o \
    $(OBJ)/src/ui/menu/controller/menu_controller_update_net.o $(OBJ)/src/ui/menu/menu_quick_menu.o \
    $(OBJ)/src/ui/menu/menu_quick_menu_settings.o
CENTRAL_MENU_RC := $(OBJ)/src/ui/menu/vehicle/menu_vehicle_rc.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_rc_failsafe.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_rc_channels.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_rc_expo.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_rc_camera.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_rc_input.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_functions.o
CENTRAL_MENU_RADIO := $(OBJ)/src/ui/menu/controller/menu_controller_radio_interface_sik.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_radio_link_sik.o \
    $(OBJ)/src/ui/menu/menu_diagnose_radio_link.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_radio_link_elrs.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_radio_pit.o \
    $(OBJ)/src/ui/menu/vehicle/menu_vehicle_radio_rt_capab.o
CENTRAL_POPUP_ALL := $(OBJ)/src/ui/popups/popup.o $(OBJ)/src/ui/popups/popup_log.o \
    $(OBJ)/src/ui/popups/popup_commands.o $(OBJ)/src/ui/popups/popup_camera_params.o \
    $(OBJ)/src/ui/popups/popup_radio_int.o
CENTRAL_RENDER_ALL := $(OBJ)/src/ui/colors.o $(OBJ)/src/ui/render_commands.o \
    $(OBJ)/src/ui/render_joysticks.o $(OBJ)/src/ui/process_router_messages.o \
    $(OBJ)/src/ui/video_playback.o
CENTRAL_OSD_ALL := $(OBJ)/src/ui/osd/osd_common.o $(OBJ)/src/ui/osd/osd.o \
    $(OBJ)/src/ui/osd/osd_stats.o $(OBJ)/src/ui/osd/osd_debug_stats.o $(OBJ)/src/ui/osd/osd_ahi.o \
    $(OBJ)/src/ui/osd/osd_lean.o $(OBJ)/src/ui/osd/osd_warnings.o $(OBJ)/src/ui/osd/osd_gauges.o \
    $(OBJ)/src/ui/osd/osd_plugins.o $(OBJ)/src/ui/osd/osd_stats_dev.o \
    $(OBJ)/src/ui/osd/osd_stats_video_bitrate.o $(OBJ)/src/ui/osd/osd_links.o \
    $(OBJ)/src/ui/osd/osd_stats_radio.o $(OBJ)/src/ui/osd/osd_widgets.o \
    $(OBJ)/src/ui/osd/osd_widgets_builtin.o $(OBJ)/src/ipc/vehicle_rt_info.o
CENTRAL_OLED_ALL := $(OBJ)/src/ui/oled/driver_ssd1306.o $(OBJ)/src/ui/oled/oled_icon_loader.o \
    $(OBJ)/src/ui/oled/oled_ssd1306.o $(OBJ)/src/ui/oled/oled_render.o
CENTRAL_ALL := $(OBJ)/src/ui/notifications.o $(OBJ)/src/ui/launchers_controller.o \
    $(OBJ)/src/ui/local_stats.o $(OBJ)/src/ui/rx_scope.o $(OBJ)/src/ui/forward_watch.o \
    $(OBJ)/src/ui/timers.o $(OBJ)/src/ui/ui_alarms.o $(OBJ)/src/ui/media.o $(OBJ)/src/ui/pairing.o \
    $(OBJ)/src/ui/link_watch.o $(OBJ)/src/ui/warnings.o $(OBJ)/src/ui/handle_commands.o \
    $(OBJ)/src/ui/events.o $(OBJ)/src/ui/shared_vars_ipc.o $(OBJ)/src/ui/shared_vars_state.o \
    $(OBJ)/src/ui/shared_vars_osd.o $(OBJ)/src/ui/fonts.o $(OBJ)/src/ui/keyboard.o \
    $(OBJ)/src/ui/quickactions.o $(OBJ)/src/ui/shared_vars.o $(OBJ)/src/media/camera_utils.o \
    $(OBJ)/src/ui/parse_msp.o $(OBJ)/src/platform/radxa_zero3w/hardware_files.o \
    $(OBJ)/src/core/i18n/strings_table.o $(OBJ)/src/core/i18n/strings_loc.o \
    $(OBJ)/src/platform/radxa_zero3w/wiringPiI2C_radxa.o $(OBJ)/src/protocol/msp.o
CENTRAL_RADIO := $(OBJ)/src/protocol/radiopackets2.o $(OBJ)/src/protocol/radiopackets_short.o \
    $(OBJ)/src/radio/link/radiotap.o $(OBJ)/src/radio/tx_powers.o

