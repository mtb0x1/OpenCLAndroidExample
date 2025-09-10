# some bunch of bs paths here
# good luck

# Detect home
ifeq ($(OS),Windows_NT)
    HOME_DIR := $(USERPROFILE)
    RM = if exist
    MKDIR = mkdir
else
    HOME_DIR := $(HOME)
    RM = rm -rf
    MKDIR = mkdir -p
endif

# Fallbacks if environment variable not set
ifeq ($(CMAKE_PATH),)
ifeq ($(OS),Windows_NT)
    CMAKE_PATH := "$(HOME_DIR)/AppData/Local/Android/Sdk/cmake/3.22.1/bin/cmake.exe"
    TOOLCHAIN_FILE := $(HOME_DIR)/AppData/Local/Android/Sdk/ndk/27.0.12077973/build/cmake/android.toolchain.cmake
else
    CMAKE_PATH := $(HOME_DIR)/Android/Sdk/cmake/3.22.1/bin/cmake
    TOOLCHAIN_FILE := $(HOME_DIR)/Android/Sdk/ndk/27.0.12077973/build/cmake/android.toolchain.cmake
endif
endif

ARCH ?= arm64-v8a
API ?= 24
BUILD_TYPE ?= Release
BUILD_DIR_CLI ?= build_cli
BUILD_DIR_CLI_DESKTOP ?= build_cli_desktop

all:
	@echo "Usage: make ui or make cli"

ui:
ifeq ($(OS),Windows_NT)
ifeq ($(BUILD_TYPE),Debug)
	.\gradlew app:assembleDebug
else
	.\gradlew app:assembleRelease
endif
else
ifeq ($(BUILD_TYPE),Debug)
	./gradlew app:assembleDebug
else
	./gradlew app:assembleRelease
endif
endif	

cli:
ifeq ($(OS),Windows_NT)
	if exist $(BUILD_DIR_CLI) rmdir /s /q $(BUILD_DIR_CLI)
else
	$(RM) $(BUILD_DIR_CLI)
endif
	$(MKDIR) $(BUILD_DIR_CLI)
	cd $(BUILD_DIR_CLI) && $(CMAKE_PATH) -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE="$(TOOLCHAIN_FILE)" -DANDROID_ABI=$(ARCH) -DANDROID_PLATFORM=android-$(API) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DBUILD_ENGINE_CLI=ON ../app/src/main/cpp
	cd $(BUILD_DIR_CLI) && $(MAKE)

#this is a quick&dirty setup to be able to test the native part on desktop
#in case of obscure bug on mobile
cli_desktop:
ifeq ($(OS),Windows_NT)
	if exist $(BUILD_DIR_CLI_DESKTOP) rmdir /s /q $(BUILD_DIR_CLI_DESKTOP)
else
	$(RM) $(BUILD_DIR_CLI_DESKTOP)
endif
	$(MKDIR) $(BUILD_DIR_CLI_DESKTOP)
	cd $(BUILD_DIR_CLI_DESKTOP) && cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DBUILD_ENGINE_CLI_DESKTOP=ON ../app/src/main/cpp
	cd $(BUILD_DIR_CLI_DESKTOP) && $(MAKE)

clean:
ifeq ($(OS),Windows_NT)
	if exist $(BUILD_DIR_CLI) rmdir /s /q $(BUILD_DIR_CLI)
	if exist $(BUILD_DIR_CLI_DESKTOP) rmdir /s /q $(BUILD_DIR_CLI_DESKTOP)
else
	$(RM) $(BUILD_DIR_CLI)
	$(RM) $(BUILD_DIR_CLI_DESKTOP)
endif
 