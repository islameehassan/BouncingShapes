CXX := g++
CXX_FLAGS := -std=c++20 -O3 -Wno-unused-result
INCLUDES := -I ./src -I ../imgui/
LIBS:= -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lGL

SRC_FILES := $(wildcard src/*.cpp src/imgui/*.cpp main.cpp)
OBJ_FILES := $(SRC_FILES:.cpp=.o)

OUTPUT := mygame

all: $(OUTPUT)
	./bin/./$(OUTPUT)

$(OUTPUT): $(OBJ_FILES) 
	$(CXX) $(OBJ_FILES) $(LIBS) -o ./bin/$@

%.o: %.cpp 
	$(CXX) -c $(CXX_FLAGS) $< -o $@

.PHONY: clean
clean: 
	rm -f ./bin/mygame
	rm -f $(wildcard *.o wildcard ./src/imgui/*.o)
