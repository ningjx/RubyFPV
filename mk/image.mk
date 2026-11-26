# Relative to the repository build directory. Config and recordings live on the device.
STATION_PROGRAMS := ruby_start ruby_controller ruby_central ruby_rt_station ruby_rx_telemetry \
    ruby_tx_rc ruby_player_radxa ruby_i2c ruby_logger ruby_initdhcp ruby_sik_config ruby_alive \
    ruby_video_proc ruby_update ruby_update_worker ruby_dbg
STATION_OSD_PLUGINS := ruby_ahi_plugin.so.1.0.1 ruby_plugin_gauge_speed.so.1.0.1 \
    ruby_plugin_gauge_altitude.so.1.0.1 ruby_plugin_gauge_ahi.so.1.0.1 ruby_plugin_gauge_heading.so.1.0.1

.PHONY: stage-runtime
stage-runtime: station ruby_plugins
	@test -n "$(DESTDIR)" && test "$$(realpath -m "$(DESTDIR)")" != / || \
	    { echo "Specify an offline staging directory with DESTDIR"; exit 1; }
	install -d "$(DESTDIR)/home/radxa/ruby/plugins/osd" "$(DESTDIR)/home/radxa/ruby/res"
	install -m 755 $(STATION_PROGRAMS) "$(DESTDIR)/home/radxa/ruby/"
	install -m 755 $(STATION_OSD_PLUGINS) "$(DESTDIR)/home/radxa/ruby/plugins/osd/"
	install -m 644 version_ruby_base.txt "$(DESTDIR)/home/radxa/ruby/"
	cp -R res/. "$(DESTDIR)/home/radxa/ruby/res/"
