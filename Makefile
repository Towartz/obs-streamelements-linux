CXX ?= g++
MOC ?= /usr/lib/qt6/moc

BUILD_DIR = build
SRC_DIR = src

TARGET = $(BUILD_DIR)/obs-streamelements.so

QT_INCLUDES = $(shell pkg-config --cflags-only-I Qt6Widgets Qt6Core Qt6Gui Qt6WebEngineWidgets Qt6WebEngineCore Qt6WebSockets Qt6Network)
QT_LIBS = $(shell pkg-config --libs Qt6Widgets Qt6Core Qt6Gui Qt6WebEngineWidgets Qt6WebEngineCore Qt6WebSockets Qt6Network)

CXXFLAGS += -std=c++20 -O2 -fPIC -Wall -Wextra \
            -I/usr/include/obs \
            $(QT_INCLUDES) \
            -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -DQT_WEBENGINEWIDGETS_LIB -DQT_WEBSOCKETS_LIB \
            -I$(SRC_DIR) -I$(BUILD_DIR)

LDFLAGS += -shared \
           -lobs -lobs-frontend-api \
           $(QT_LIBS) -lcurl

SRCS = $(SRC_DIR)/plugin-main.cpp \
       $(SRC_DIR)/MenuManager.cpp \
       $(SRC_DIR)/BrowserDock.cpp \
       $(SRC_DIR)/BrowserWidgetManager.cpp \
       $(SRC_DIR)/ApiServer.cpp \
       $(SRC_DIR)/AudioWrapper.cpp \
       $(SRC_DIR)/BackupManager.cpp \
       $(SRC_DIR)/AuthManager.cpp \
       $(SRC_DIR)/AuthServer.cpp \
       $(SRC_DIR)/LoginDialog.cpp

MOC_HDRS = $(SRC_DIR)/MenuManager.hpp \
           $(SRC_DIR)/BrowserDock.hpp \
           $(SRC_DIR)/BrowserWidgetManager.hpp \
           $(SRC_DIR)/ApiServer.hpp \
           $(SRC_DIR)/AuthManager.hpp \
           $(SRC_DIR)/AuthServer.hpp \
           $(SRC_DIR)/LoginDialog.hpp

MOC_SRCS = $(patsubst $(SRC_DIR)/%.hpp,$(BUILD_DIR)/moc_%.cpp,$(MOC_HDRS))
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS)) $(patsubst $(BUILD_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(MOC_SRCS))

INSTALL_PLUGIN_DIR = $(HOME)/.config/obs-studio/plugins/obs-streamelements/bin/64bit
INSTALL_DATA_DIR = $(HOME)/.config/obs-studio/plugins/obs-streamelements/data

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@
	@echo "Build complete: $@"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/moc_%.cpp: $(SRC_DIR)/%.hpp
	@mkdir -p $(BUILD_DIR)
	$(MOC) $< -o $@

$(BUILD_DIR)/moc_%.o: $(BUILD_DIR)/moc_%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

install: $(TARGET)
	@mkdir -p $(INSTALL_PLUGIN_DIR)
	@mkdir -p $(INSTALL_DATA_DIR)
	cp $(TARGET) $(INSTALL_PLUGIN_DIR)/
	cp -r data/* $(INSTALL_DATA_DIR)/
	@echo "Installed plugin to $(INSTALL_PLUGIN_DIR)"
	@echo "Installed assets to $(INSTALL_DATA_DIR)"

clean:
	rm -rf $(BUILD_DIR)
