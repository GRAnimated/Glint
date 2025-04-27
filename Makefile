BUILD_DIR = build
BUILD_TYPE = Debug
GENERATOR = Unix Makefiles

all: build

build:
	@mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -G "$(GENERATOR)"
	$(MAKE) -C $(BUILD_DIR)

setup:
	sys/tools/setup_libcxx_prepackaged.py
	sys/tools/setup_sail.py

clean:
	@rm -rf $(BUILD_DIR)

.PHONY: all build clean
