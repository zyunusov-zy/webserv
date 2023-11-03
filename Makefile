CXX = g++
CXXFLAGS = -std=c++98 -Wall -Wextra -Werror

SRC_DIR = Config
BUILD_DIR = build

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPENDENCIES = $(OBJECTS:.o=.d)

TARGET = webserv

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

-include $(DEPENDENCIES)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

re: clean all

clean:
	rm -rf $(BUILD_DIR)

fclean: clean
	rm -f $(TARGET)

.PHONY: all re clean fclean
