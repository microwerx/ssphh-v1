
# Debian packages
# libczmq-dev libzmq3-dev libcurl4-gnutls-dev libsodium-dev zlib1g-dev python3-dev freeglut3-dev libglew-dev

# Fedora packages
# dnf install czmq-devel zeromq-devel libcurl-devel libsodium-devel zlib-devel python3-devel glew-devel SDL2-devel SDL2_image-devel

# development packages
# global

DEP_INCDIR = ../fluxions/dep/include
DEP_SRCDIR = ../fluxions/dep/src
SRCDIR = src
INCDIR = include
OBJDIR = build
FLUXIONS = ../fluxions/build/libfluxions.a
FLUXIONS_INCDIR = ../fluxions/include
FLUXIONS_LIBDIR = ../fluxions/build
FLUXIONS_SRCDIR = ../fluxions/src
FLUXIONS_HEADERS = $(wildcard $(FLUXIONS_INCDIR)/*.hpp) $(wildcard $(FLUXIONS_INCDIR)/*.h)
FLUXIONS_SOURCES = $(wildcard $(FLUXIONS_SRCDIR)/*.cpp) $(wildcard $(FLUXIONS_SRCDIR)/*.c)
DEPCXXSOURCES = $(wildcard $(DEP_SRCDIR)/*.cpp)
DEPCSOURCES = $(wildcard $(DEP_SRCDIR)/*.c)
CXXSOURCES = $(wildcard $(SRCDIR)/*.cpp)
CXXHEADERS = $(wildcard $(INCDIR)/*.hpp) $(wildcard $(DEP_SRCDIR)/*.hpp)
CSOURCES = $(wildcard $(SRCDIR)/*.c)
CHEADERS = $(wildcard $(INCDIR)/*.h) $(wildcard $(DEP_SRCDIR)/*.h)
SOURCES = $(CXXSOURCES) $(CSOURCES)
HEADERS = $(CXXHEADERS) $(CHEADERS)
SRCOBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(CXXSOURCES)) $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(CSOURCES))
DEPOBJECTS = $(patsubst $(DEP_SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(DEPCXXSOURCES)) $(patsubst $(DEP_SRCDIR)/%.c,$(OBJDIR)/%.o,$(DEPCSOURCES))
# DEPCOBJECTS= $(patsubst $(DEP_SRCDIR)/%.c,$(OBJDIR)/%.o,$(DEPCSOURCES))
OBJECTS = $(SRCOBJECTS) $(DEPOBJECTS)
TARGET = build/ssphh
GCH = $(SRCDIR)/pch.h.gch
GCH_SRC = $(SRCDIR)/pch.h

CC = gcc
CCFLAGS = -Wall -I$(INCDIR) -I$(DEP_INCDIR) `python3-config --includes`
CXX = g++
CXXFLAGS = -std=c++14 -g -Wall -I$(INCDIR) -I$(DEP_INCDIR) -I$(FLUXIONS_INCDIR) `python3-config --includes`
LDFLAGS = -L../fluxions/build -lfluxions -lglut -lGLEW -lGL -lcurl -lczmq -lzmq -lSDL2_image -lSDL2 -lIlmImf -lHalf -lImath -lpthread -lstdc++

.PHONY: all clean precompiled

all: GTAGS $(GCH) $(TARGET) $(FLUXIONS)

precompiled: $(GCH)
	echo $(SRCOBJECTS)
	echo DEPOBJECTS =======================
	echo $(DEPOBJECTS)

cobjects: $(DEPCOBJECTS)
	echo $(DEPCOBJECTS)
	echo
	echo $(DEPOBJECTS)

$(TARGET): $(OBJECTS) ../fluxions/build/libfluxions.a
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)

$(GCH): $(GCH_SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(FLUXIONS): $(FLUXIONS_HEADERS) $(FLUXIONS_SOURCES)
	$(MAKE) -C ../fluxions/

# $(OBJDIR)/%.o: $(CXXSOURCES)/%.cpp $(GCH)
# 	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(GCH)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(DEP_SRCDIR)/%.cpp $(GCH)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(GCH)
	$(CC) $(CCFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(DEP_SRCDIR)/%.c $(GCH)
	$(CC) $(CCFLAGS) -c $< -o $@

GTAGS: $(SOURCES) $(HEADERS)
	gtags

clean:
	$(RM) -f $(GCH)
	$(RM) -f $(OBJDIR)/*.o
	$(RM) -f $(TARGET)
	$(RM) -f GPATH GRTAGS GTAGS
