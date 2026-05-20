CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2
TARGET   := dungeon
SRCDIR   := src
INCDIR   := include

SOURCES  := main.cpp \
            $(SRCDIR)/terminal.cpp \
            $(SRCDIR)/dungeon.cpp  \
            $(SRCDIR)/player.cpp   \
            $(SRCDIR)/combat.cpp   \
            $(SRCDIR)/renderer.cpp \
            $(SRCDIR)/game.cpp

OBJECTS  := $(SOURCES:.cpp=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

run: all
	./$(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET) save.dat
