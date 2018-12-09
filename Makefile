# requires the following packages
# libczmq-dev libzmq3-dev libcurl4-gnutls-dev libsodium-dev zlib1g-dev python3-dev freeglut3-dev libglew-dev
# development packages
# global

DEP_INCDIR = dep/include
DEP_SRCDIR = dep/src
SRCDIR = src
INCDIR = include
OBJDIR = build
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
TARGET = build/libfluxions.a
GCH = $(SRCDIR)/stdafx.h.gch

CC = gcc
CCFLAGS = -Wall -I$(INCDIR) -I$(DEP_INCDIR) `python3-config --includes`
CXX = g++
CXXFLAGS = -std=c++14 -g -Wall -I$(INCDIR) -I$(DEP_INCDIR) `python3-config --includes`
LDFLAGS = -LGLEW -LGL -LGLU -Lglut

.PHONY: all clean precompiled

all: GTAGS $(GCH) $(TARGET)

precompiled: $(GCH)
	echo $(SRCOBJECTS)
	echo DEPOBJECTS =======================
	echo $(DEPOBJECTS)

cobjects: $(DEPCOBJECTS)
	echo $(DEPCOBJECTS)
	echo
	echo $(DEPOBJECTS)

$(TARGET): $(OBJECTS)
	$(AR) cr $@ $(OBJECTS) 

$(GCH): $(SRCDIR)/stdafx.h $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

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
