# Simplified Makefile for Jython Loader (macOS only)

# JAVA_HOME detection
JAVA_HOME := $(shell /usr/libexec/java_home)

# macOS-specific settings
CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2 -arch x86_64 -arch arm64
INCLUDES = -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/darwin
SHARED_FLAGS = -dynamiclib

# Targets
JVMTI_TARGET = jython_loader.dylib
JVMTI_SOURCE = jython_loader.cpp
CAPABILITY_TARGET = capability_enum_agent.dylib
CAPABILITY_SOURCE = capability_enum_agent.cpp
INTERCEPTOR_TARGET = runtime_memory_modifier_agent.dylib
INTERCEPTOR_SOURCE = runtime_memory_modifier_agent.cpp
JAVA_TARGET = TestTarget.class
JAVA_SOURCE = TestTarget.java

# Build all targets
all: $(JVMTI_TARGET) $(CAPABILITY_TARGET) $(INTERCEPTOR_TARGET) $(JAVA_TARGET)

# Build JVMTI agent
$(JVMTI_TARGET): $(JVMTI_SOURCE)
	@echo "Building Jython loader for macOS..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SHARED_FLAGS) -o $(JVMTI_TARGET) $(JVMTI_SOURCE)
	@echo "Successfully built $(JVMTI_TARGET)"

# Build capability enumeration agent
$(CAPABILITY_TARGET): $(CAPABILITY_SOURCE)
	@echo "Building capability enumeration agent for macOS..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SHARED_FLAGS) -o $(CAPABILITY_TARGET) $(CAPABILITY_SOURCE)
	@echo "Successfully built $(CAPABILITY_TARGET)"

# Build runtime memory modifier agent
$(INTERCEPTOR_TARGET): $(INTERCEPTOR_SOURCE)
	@echo "Building runtime memory modifier agent for macOS..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SHARED_FLAGS) -o $(INTERCEPTOR_TARGET) $(INTERCEPTOR_SOURCE)
	@echo "Successfully built $(INTERCEPTOR_TARGET)"

# Build Java application
$(JAVA_TARGET): $(JAVA_SOURCE)
	@echo "Compiling Java application..."
	javac $(JAVA_SOURCE)
	@echo "Successfully compiled $(JAVA_TARGET)"

# Clean all build artifacts
clean:
	rm -f $(JVMTI_TARGET) $(CAPABILITY_TARGET) $(INTERCEPTOR_TARGET) $(JAVA_TARGET)
	@echo "Cleaned all build artifacts"