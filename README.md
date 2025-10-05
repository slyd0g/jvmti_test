# JVMTI Agent with Jython REPL

## Overview
This JVMTI (JVM Tool Interface) agent provides runtime access to JVM processes through a Jython REPL. It dynamically loads into running JVMs using `jcmd` and embeds a Jython interpreter accessible via socket connection.

### Prerequisites
- macOS with Xcode command line tools
- Java Development Kit (JDK) with `jcmd` and `jps` tools
- Jython standalone JAR - Download from [jython.org](https://www.jython.org/download) and place as `jython-standalone.jar` in project directory
- netcat (`nc`) for REPL connection (pre-installed on macOS)

### Build, Run, Call Java APIs through Jython REPL
```bash
# Build everything
make all

# Launch Java app and connect to REPL
./run.sh
Cleaning up any existing TestTarget processes...
No existing TestTarget processes found
Starting TestTarget Java application...
Waiting for JVM to start...
TestTarget running - PID: 25442
Finding TestTarget process...
Found TestTarget process: 25442
Attaching JVMTI agent...
[JVMTI Agent] Agent_OnAttach called
...
...
...
[JVMTI Agent] Agent successfully attached
return code: 0

JVMTI agent attached successfully!
Waiting 8 seconds for Jython server to start...
[Jython Server] Listening on localhost:1337
Connecting to Jython REPL...
Process ID: 25442 (kill with: kill 25442)
Type 'exit' to disconnect from REPL
---
('[Jython Server] Connected from', ('127.0.0.1', 50347))
Jython REPL connected to JVM process
>>> import TestTarget

>>> target = TestTarget()

>>> target.GetSecret()
slyd0g
```


