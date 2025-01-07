SRCDIR = src
INCDIR = include
OBJDIR = trash
BINDIR = bin/Linux

SRCS = $(wildcard $(SRCDIR)/*.cpp) $(SRCDIR)/glad.c
HEADERS = $(wildcard $(INCDIR)/*.h)

OBJS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o))

OUTFILE = $(BINDIR)/main

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wno-unused-function -g -I$(INCDIR)
LDFLAGS = ./lib-linux/libglfw3.a -lrt -lm -ldl -lX11 -lpthread -lXrandr -lXinerama -lXxf86vm -lXcursor

$(OUTFILE): $(OBJS)
	mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(HEADERS)
	mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HEADERS)
	mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean run

clean:
	rm -rf $(OBJDIR) $(OUTFILE)

run: $(OUTFILE)
	cd $(BINDIR) && ./main