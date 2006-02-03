# building of python bindings.
YANS_PYTHON_SRC= \
	python/yans-simulator.cc \
	python/export-simulator.cc \
	python/function-holder.cc \
	python/yans/__init__.py \
	python/yans/simulator/__init__.py \
	python/export-thread.cc \
	python/export-callback.cc \
	python/export-periodic-generator.cc \
	python/export-traffic-analyser.cc \
	python/export-packet.cc \
	python/export-ipv4-address.cc \
	python/export-mac-address.cc \
	python/export-host.cc \
	python/export-udp-sink.cc \
	python/export-udp-source.cc \
	python/export-network-interface.cc \
	python/export-ethernet-network-interface.cc \
	python/export-cable.cc \
	python/export-ipv4-route.cc \
	python/export-trace-container.cc \
	python/export-pcap-writer.cc \
	python/export-tcp-source.cc \
	python/export-tcp-sink.cc \
	python/test-periodic-generator.py \
	samples/test-simulator.py \
	samples/test-thread.py \
	samples/test-simulator-gc.py \
	samples/test-simple.py \
	$(NULL)
YANS_PYTHON_HDR= \
	python/function-holder.h \
	python/export-callback.tcc \
	python/export-callback-traits.tcc \
	$(NULL)
YANS_PYTHON_OUTPUT=$(TOP_BUILD_DIR)/python/$(call gen-pymod-name, _yans)
YANS_PYTHON_CXXFLAGS=$(CXXFLAGS) $(call gen-pymod-build-flags)
YANS_PYTHON_LDFLAGS=$(LDFLAGS) $(call gen-pymod-link-flags) -lyans -L$(TOP_BUILD_DIR)

ALL += \
	YANS_PYTHON \
	$(NULL)
