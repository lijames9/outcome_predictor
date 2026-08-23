CXX := g++
CXXFLAGS := -std=c++14 -O2 -Wall -Wextra -Iinclude
SRC := src/main.cpp src/CsvReader.cpp src/Backtester.cpp
OBJ := $(SRC:.cpp=.o)
TARGET := backtester

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) data/backtest_input.csv

clean:
	rm -f $(OBJ) $(TARGET) $(TARGET).exe
