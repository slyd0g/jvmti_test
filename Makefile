# Simplified Makefile for JVMTI Agent (macOS only)

# JAVA_HOME detection
JAVA_HOME := $(shell /usr/libexec/java_home)

# macOS-specific settings
CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2 -arch x86_64 -arch arm64
INCLUDES = -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/darwin
SHARED_FLAGS = -dynamiclib

# Targets
JVMTI_TARGET = jvmti_agent.dylib
JVMTI_SOURCE = jvmti_agent.cpp
JAVA_TARGET = TestTarget.class
JAVA_SOURCE = TestTarget.java

# Build all targets
all: $(JVMTI_TARGET) $(JAVA_TARGET)

# Build JVMTI agent
$(JVMTI_TARGET): $(JVMTI_SOURCE)
	@echo "Building JVMTI agent for macOS..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SHARED_FLAGS) -o $(JVMTI_TARGET) $(JVMTI_SOURCE)
	@echo "Successfully built $(JVMTI_TARGET)"

# Build Java application
$(JAVA_TARGET): $(JAVA_SOURCE)
	@echo "Compiling Java application..."
	javac $(JAVA_SOURCE)
	@echo "Successfully compiled $(JAVA_TARGET)"

# Clean all build artifacts
clean:
	rm -f $(JVMTI_TARGET) $(JAVA_TARGET)
	@echo "Cleaned all build artifacts"