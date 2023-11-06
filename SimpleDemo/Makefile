## naive version
# all: simple_server simple_server_epoll
# simple_server: simple_server.cpp
# 	g++ -g -o simple_server simple_server.cpp
# simple_server_epoll: simple_server_epoll.cpp
# 	g++ -g -o simple_server_epoll simple_server_epoll.cpp

CXX = g++
CXXFLAGS = -std=c++14

SRC_DIR = .
OBJ_DIR = bin

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
EXECUTABLES = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%,$(SOURCES))

all: $(EXECUTABLES)

$(OBJ_DIR)/%: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -rf $(OBJ_DIR)

.PHONY: all clean
