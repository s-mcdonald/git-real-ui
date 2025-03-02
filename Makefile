#
# Cross Platform Makefile
#
# You will need GLFW (http://www.glfw.org):
# Linux:
#   apt-get install libglfw-dev
#
#

TARGET = bin/gitreal-ui
SRC_DIR = ./src

VENDOR_DIR  = ../../vendor
INCLUDE_DIR = ../../includes

IMGUI_DIR = $(VENDOR_DIR)/ocornut/imgui


# App Sources
SOURCES = $(SRC_DIR)/main.cpp 
SOURCES += $(INCLUDE_DIR)/gr/gr-app.cpp
SOURCES += $(INCLUDE_DIR)/gr/gr-ui.cpp 

# Vendor Sources
SOURCES += $(VENDOR_DIR)/benhoyt/inih/ini.c
SOURCES += $(VENDOR_DIR)/benhoyt/inih/cpp/INIReader.cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp

OBJS = $(SOURCES:.cpp=.o)
OBJS := $(OBJS:.c=.o)

UNAME_S := $(shell uname -s)
LINUX_GL_LIBS = -lGL
LIBS = -lgit2

CXXFLAGS = -std=c++17 
CXXFLAGS += -I$(IMGUI_DIR)/backends 
CXXFLAGS += -g -Wall -Wformat

CXXFLAGS += -I$(IMGUI_DIR)
CXXFLAGS += -I$(VENDOR_DIR)/benhoyt/inih
CXXFLAGS += -I$(INCLUDE_DIR)/gr
CXXFLAGS += -I$(INCLUDE_DIR)/io



ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	LIBS += $(LINUX_GL_LIBS) `pkg-config --static --libs glfw3`

	CXXFLAGS += `pkg-config --cflags glfw3`
	CFLAGS = $(CXXFLAGS)
endif


##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:%.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(VENDOR_DIR)/*/%.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(VENDOR_DIR)/*/cpp/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<	

%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(INCLUDE_DIR)/gr/*.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(TARGET)
	@echo Build complete for $(ECHO_MESSAGE)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)
	rm -f $(OBJS)

clean:
	rm -f $(TARGET) $(OBJS)
