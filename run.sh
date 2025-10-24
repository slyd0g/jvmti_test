#!/bin/bash

# Jython Loader Execution Script
# Assumes jython_loader.dylib and TestTarget.class are already built

echo "Cleaning up any existing TestTarget processes..."
pkill -f TestTarget 2>/dev/null && echo "Killed existing TestTarget processes" || echo "No existing TestTarget processes found"

echo "Starting TestTarget Java application..."
java TestTarget &

echo "Waiting for JVM to start..."
sleep 2

echo "Finding TestTarget process..."
PID=$(jps | grep TestTarget | cut -d' ' -f1)

if [ -n "$PID" ]; then
    echo "Found TestTarget process: $PID"
    echo "Attaching Jython loader..."
    
    # Use absolute path for jcmd
    AGENT_PATH="$(pwd)/jython_loader.dylib"
    
    # Check if agent file exists
    if [ ! -f "$AGENT_PATH" ]; then
        echo "Agent file not found: $AGENT_PATH"
        echo "Run 'make all' first"
        exit 1
    fi
    
    jcmd $PID JVMTI.agent_load "$AGENT_PATH"
    
    echo ""
    echo "Jython loader attached successfully!"
    echo "Waiting 8 seconds for Jython server to start..."
    sleep 8
    
    echo "Connecting to Jython REPL..."
    echo "Process ID: $PID (kill with: kill $PID)"
    echo "Type 'exit' to disconnect from REPL"
    echo "---"
    
    # Connect to Jython REPL
    nc localhost 1337
    
else
    echo "Could not find TestTarget process"
    echo "Make sure you built with: make all"
    exit 1
fi