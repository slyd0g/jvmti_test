#!/bin/bash

# Generic JVMTI Agent Execution Script
# Usage: ./run.sh [agent_name]
# Example: ./run.sh jython_loader
# If no agent specified, defaults to jython_loader

# Handle help
if [ "$1" = "--help" ] || [ "$1" = "-h" ] || [ "$1" = "help" ]; then
    echo "Usage: $0 [agent_name]"
    echo ""
    echo "Loads a JVMTI agent into a running TestTarget Java process."
    echo ""
    echo "Arguments:"
    echo "  agent_name    Name of the agent (without .dylib extension)"
    echo "                Default: jython_loader"
    echo ""
    echo "Examples:"
    echo "  $0                    # Load jython_loader.dylib"
    echo "  $0 jython_loader      # Load jython_loader.dylib"
    echo "  $0 my_custom_agent    # Load my_custom_agent.dylib"
    echo ""
    echo "Available agents:"
    ls -1 *.dylib 2>/dev/null | sed 's/\.dylib$//' | sed 's/^/  /' || echo "  No .dylib files found"
    exit 0
fi

# Get agent name from command line argument, default to jython_loader
AGENT_NAME=${1:-jython_loader}
AGENT_PATH="$(pwd)/${AGENT_NAME}.dylib"

echo "Using agent: ${AGENT_NAME}"
echo "Agent path: ${AGENT_PATH}"

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
    echo "Attaching agent: ${AGENT_NAME}..."
    
    # Check if agent file exists
    if [ ! -f "$AGENT_PATH" ]; then
        echo "Agent file not found: $AGENT_PATH"
        echo "Available agents:"
        ls -1 *.dylib 2>/dev/null || echo "  No .dylib files found"
        echo "Run 'make all' or build your specific agent first"
        exit 1
    fi
    
    jcmd $PID JVMTI.agent_load "$AGENT_PATH"
    
    echo ""
    echo "Agent '${AGENT_NAME}' attached successfully!"
    
    # Special handling for jython_loader - connect to REPL
    if [ "$AGENT_NAME" = "jython_loader" ]; then
        echo "Waiting 8 seconds for Jython server to start..."
        sleep 8
        
        echo "Connecting to Jython REPL..."
        echo "Process ID: $PID (kill with: kill $PID)"
        echo "Type 'exit' to disconnect from REPL"
        echo "---"
        
        # Connect to Jython REPL
        nc localhost 1337
    else
        echo "Agent loaded. TestTarget is running with PID: $PID"
        echo "Kill with: kill $PID"
        echo "Monitor output or attach debugger as needed."
    fi
    
else
    echo "Could not find TestTarget process"
    echo "Make sure TestTarget is running and you built the required files"
    echo "Usage: $0 [agent_name]"
    echo "Example: $0 jython_loader"
    exit 1
fi