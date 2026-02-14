CPP_COMPILER=g++

SRC := $(shell find . -name '*.cpp')
OBJ := $(SRC:.cpp=.o)

TARGET := build/a

default:
	@echo $(OBJ)

	$(CPP_COMPILER) $(SRC) -lGL -g -o $(TARGET)