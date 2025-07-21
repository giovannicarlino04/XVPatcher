# XVPatcher Modern Makefile
# Optimized for performance and maintainability

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++20 -O3 -march=native -mtune=native -Wall -Wextra -Werror
CXXFLAGS += -fno-exceptions -fno-rtti -ffast-math -flto
CXXFLAGS += -DNDEBUG -D_WIN32_WINNT=0x600

# Debug flags (uncomment for debug build)
# CXXFLAGS = -std=c++20 -O0 -g -Wall -Wextra -DDEBUG -D_WIN32_WINNT=0x600

# Directories
SRCDIR = src
INCDIR = include
BUILDDIR = build
OBJDIR = $(BUILDDIR)/obj

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Target
TARGET = $(BUILDDIR)/xinput1_3.dll

# Libraries
LIBS = -lpsapi -lkernel32 -luser32

# Default target
all: $(TARGET)

# Create build directories
$(OBJDIR):
	@mkdir -p $(OBJDIR)

# Compile source files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	@echo "Compiling $<..."
	@$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

# Link target
$(TARGET): $(OBJECTS)
	@echo "Linking $@..."
	@$(CXX) $(CXXFLAGS) -shared -o $@ $^ $(LIBS)
	@echo "Build complete: $@"

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILDDIR)
	@echo "Clean complete"

# Install target (copy to game directory)
install: $(TARGET)
	@echo "Installing XVPatcher..."
	@cp $(TARGET) ./xinput1_3.dll
	@echo "Installation complete"

# Development targets
debug: CXXFLAGS = -std=c++20 -O0 -g -Wall -Wextra -DDEBUG -D_WIN32_WINNT=0x600
debug: $(TARGET)

release: CXXFLAGS = -std=c++20 -O3 -march=native -mtune=native -Wall -Wextra -DNDEBUG -D_WIN32_WINNT=0x600 -fno-exceptions -fno-rtti -ffast-math -flto
release: clean $(TARGET)

# Performance profiling
profile: CXXFLAGS += -pg
profile: $(TARGET)

# Static analysis
analyze: CXXFLAGS += -fsanitize=address -fsanitize=undefined
analyze: $(TARGET)

# Help target
help:
	@echo "XVPatcher Build System"
	@echo "======================"
	@echo "Available targets:"
	@echo "  all      - Build optimized release version (default)"
	@echo "  debug    - Build with debug symbols and no optimization"
	@echo "  release  - Build optimized release version"
	@echo "  profile  - Build with profiling support"
	@echo "  analyze  - Build with static analysis tools"
	@echo "  clean    - Remove all build artifacts"
	@echo "  install  - Copy DLL to current directory"
	@echo "  help     - Show this help message"
	@echo ""
	@echo "Build directory: $(BUILDDIR)"
	@echo "Target: $(TARGET)"

# Phony targets
.PHONY: all clean install debug release profile analyze help

# Dependencies
$(OBJDIR)/main.o: $(INCDIR)/patcher.hpp $(INCDIR)/config.hpp $(INCDIR)/logger.hpp $(INCDIR)/patch.hpp $(INCDIR)/memory.hpp $(INCDIR)/types.hpp
$(OBJDIR)/patcher.o: $(INCDIR)/patcher.hpp $(INCDIR)/config.hpp $(INCDIR)/logger.hpp $(INCDIR)/patch.hpp $(INCDIR)/memory.hpp $(INCDIR)/types.hpp
$(OBJDIR)/patch.o: $(INCDIR)/patch.hpp $(INCDIR)/logger.hpp $(INCDIR)/memory.hpp $(INCDIR)/types.hpp
$(OBJDIR)/logger.o: $(INCDIR)/logger.hpp $(INCDIR)/types.hpp