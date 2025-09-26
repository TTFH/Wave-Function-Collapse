TARGET = wfc
CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -Wpedantic
CXXFLAGS += -Wno-missing-field-initializers
RELEASEFLAGS = -O3
DEBUGFLAGS = -g

OBJ_DIR = obj
OUTPUT_DIR = output

SRCS = main.cpp lib/tinyxml2.cpp
SRCS += src/image.cpp src/symmetry.cpp
SRCS += src/propagator.cpp src/wave.cpp src/wfc.cpp
SRCS += src/overlapping_wfc.cpp src/simpletiled_wfc.cpp src/imagemosaic_wfc.cpp
OBJS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(notdir $(SRCS)))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/%.cpp src/%.h | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: lib/%.cpp lib/%.h | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

debug: CXXFLAGS += $(DEBUGFLAGS)
debug: $(TARGET)

release: CXXFLAGS += $(RELEASEFLAGS)
release: $(TARGET)

clean:
	rm -rf $(OBJ_DIR)/*.o $(TARGET)

.PHONY: all debug release clean
