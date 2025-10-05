#include <jvmti.h>
#include <jni.h>
#include <iostream>
#include <string>
#include <cstring>

// Global JVMTI environment
static jvmtiEnv* g_jvmti = nullptr;
static JNIEnv* g_jni = nullptr;
static bool g_jython_server_started = false;

// Forward declaration
void start_jython_socket_server();

// Start Jython socket server on port 1337
void start_jython_socket_server() {
    if (!g_jni) {
        std::cout << "[JVMTI Agent] JNI environment not available for Jython server" << std::endl;
        return;
    }
    
    std::cout << "[JVMTI Agent] Starting Jython socket server on port 1337..." << std::endl;
    
    try {
        // Try to load Jython classes using JNI
        jclass pythonInterpreterClass = g_jni->FindClass("org/python/util/PythonInterpreter");
        if (g_jni->ExceptionCheck()) {
            g_jni->ExceptionClear();
            pythonInterpreterClass = nullptr;
        }
        
        if (!pythonInterpreterClass) {
            std::cout << "[JVMTI Agent] Jython not found in classpath - loading from jython-standalone.jar" << std::endl;
            
            // Load Jython from the jar file on disk
            jclass urlClassLoaderClass = g_jni->FindClass("java/net/URLClassLoader");
            jclass urlClass = g_jni->FindClass("java/net/URL");
            jclass fileClass = g_jni->FindClass("java/io/File");
            
            if (g_jni->ExceptionCheck()) {
                std::cout << "[JVMTI Agent] Exception finding basic Java classes" << std::endl;
                g_jni->ExceptionDescribe();
                g_jni->ExceptionClear();
                return;
            }
            
            if (urlClassLoaderClass && urlClass && fileClass) {
                // Create File object for jython-standalone.jar in current directory
                jmethodID fileConstructor = g_jni->GetMethodID(fileClass, "<init>", "(Ljava/lang/String;)V");
                jstring jythonPath = g_jni->NewStringUTF("./jython-standalone.jar");
                jobject jythonFile = g_jni->NewObject(fileClass, fileConstructor, jythonPath);
                
                if (g_jni->ExceptionCheck()) {
                    std::cout << "[JVMTI Agent] Exception creating File object" << std::endl;
                    g_jni->ExceptionDescribe();
                    g_jni->ExceptionClear();
                    return;
                }
                
                // Convert File to URL
                jmethodID toURIMethod = g_jni->GetMethodID(fileClass, "toURI", "()Ljava/net/URI;");
                jobject jythonURI = g_jni->CallObjectMethod(jythonFile, toURIMethod);
                
                if (g_jni->ExceptionCheck()) {
                    std::cout << "[JVMTI Agent] Exception converting File to URI" << std::endl;
                    g_jni->ExceptionDescribe();
                    g_jni->ExceptionClear();
                    return;
                }
                
                jclass uriClass = g_jni->FindClass("java/net/URI");
                jmethodID toURLMethod = g_jni->GetMethodID(uriClass, "toURL", "()Ljava/net/URL;");
                jobject jythonURL = g_jni->CallObjectMethod(jythonURI, toURLMethod);
                
                if (g_jni->ExceptionCheck()) {
                    std::cout << "[JVMTI Agent] Exception converting URI to URL" << std::endl;
                    g_jni->ExceptionDescribe();
                    g_jni->ExceptionClear();
                    return;
                }
                
                // Create URL array
                jobjectArray urlArray = g_jni->NewObjectArray(1, urlClass, jythonURL);
                
                // Create URLClassLoader
                jmethodID urlClassLoaderConstructor = g_jni->GetMethodID(urlClassLoaderClass, "<init>", "([Ljava/net/URL;)V");
                jobject jythonClassLoader = g_jni->NewObject(urlClassLoaderClass, urlClassLoaderConstructor, urlArray);
                
                if (g_jni->ExceptionCheck()) {
                    std::cout << "[JVMTI Agent] Exception creating URLClassLoader" << std::endl;
                    g_jni->ExceptionDescribe();
                    g_jni->ExceptionClear();
                    return;
                }
                
                // Load PythonInterpreter class through the new classloader
                jmethodID loadClassMethod = g_jni->GetMethodID(urlClassLoaderClass, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
                jstring pythonInterpreterName = g_jni->NewStringUTF("org.python.util.PythonInterpreter");
                pythonInterpreterClass = (jclass)g_jni->CallObjectMethod(jythonClassLoader, loadClassMethod, pythonInterpreterName);
                
                if (g_jni->ExceptionCheck()) {
                    std::cout << "[JVMTI Agent] Exception loading PythonInterpreter class" << std::endl;
                    g_jni->ExceptionDescribe();
                    g_jni->ExceptionClear();
                    return;
                }
                
                g_jni->DeleteLocalRef(jythonPath);
                g_jni->DeleteLocalRef(jythonFile);
                g_jni->DeleteLocalRef(jythonURI);
                g_jni->DeleteLocalRef(jythonURL);
                g_jni->DeleteLocalRef(urlArray);
                g_jni->DeleteLocalRef(pythonInterpreterName);
            }
        }
        
        if (pythonInterpreterClass) {
            std::cout << "[JVMTI Agent] Found Jython PythonInterpreter class" << std::endl;
            
            // Create PythonInterpreter instance
            jmethodID pythonConstructor = g_jni->GetMethodID(pythonInterpreterClass, "<init>", "()V");
            jobject pythonInterpreter = g_jni->NewObject(pythonInterpreterClass, pythonConstructor);
            
            if (g_jni->ExceptionCheck()) {
                std::cout << "[JVMTI Agent] Exception creating PythonInterpreter instance" << std::endl;
                g_jni->ExceptionDescribe();
                g_jni->ExceptionClear();
                return;
            }
            
            if (pythonInterpreter) {
                std::cout << "[JVMTI Agent] Created PythonInterpreter instance" << std::endl;
                
                // Set up Python code to create a socket server
                jmethodID execMethod = g_jni->GetMethodID(pythonInterpreterClass, "exec", "(Ljava/lang/String;)V");
                
                std::string pythonCode = 
                    "import socket\n"
                    "import threading\n"
                    "\n"
                    "def handle_client(conn, addr):\n"
                    "    print('[Jython Server] Connected from', addr)\n"
                    "    conn.send(b'Jython REPL connected to JVM process\\n>>> ')\n"
                    "    \n"
                    "    while True:\n"
                    "        try:\n"
                    "            data = conn.recv(1024).decode('utf-8').strip()\n"
                    "            if not data or data == 'exit':\n"
                    "                break\n"
                    "            \n"
                    "            try:\n"
                    "                # Execute Python code and capture output\n"
                    "                result = eval(data)\n"
                    "                conn.send((str(result) + '\\n>>> ').encode('utf-8'))\n"
                    "            except:\n"
                    "                try:\n"
                    "                    exec(data)\n"
                    "                    conn.send(b'\\n>>> ')\n"
                    "                except Exception as e:\n"
                    "                    conn.send(('Error: ' + str(e) + '\\n>>> ').encode('utf-8'))\n"
                    "        except:\n"
                    "            break\n"
                    "    \n"
                    "    conn.close()\n"
                    "    print('[Jython Server] Disconnected from', addr)\n"
                    "\n"
                    "def start_server():\n"
                    "    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)\n"
                    "    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)\n"
                    "    server.bind(('localhost', 1337))\n"
                    "    server.listen(5)\n"
                    "    print('[Jython Server] Listening on localhost:1337')\n"
                    "    \n"
                    "    while True:\n"
                    "        conn, addr = server.accept()\n"
                    "        client_thread = threading.Thread(target=handle_client, args=(conn, addr))\n"
                    "        client_thread.daemon = True\n"
                    "        client_thread.start()\n"
                    "\n"
                    "# Start server in background thread\n"
                    "server_thread = threading.Thread(target=start_server)\n"
                    "server_thread.daemon = True\n"
                    "server_thread.start()\n"
                    "print('[Jython Server] Started successfully!')\n";
                
                jstring pythonCodeStr = g_jni->NewStringUTF(pythonCode.c_str());
                g_jni->CallVoidMethod(pythonInterpreter, execMethod, pythonCodeStr);
                
                // Check for exceptions
                if (g_jni->ExceptionCheck()) {
                    std::cout << "[JVMTI Agent] Exception occurred while starting Jython server" << std::endl;
                    g_jni->ExceptionDescribe();
                    g_jni->ExceptionClear();
                } else {
                    std::cout << "[JVMTI Agent] Jython socket server started on localhost:1337!" << std::endl;
                    std::cout << "[JVMTI Agent] Connect with: nc localhost 1337 or telnet localhost 1337" << std::endl;
                }
                
                g_jni->DeleteLocalRef(pythonCodeStr);
            }
        } else {
            std::cout << "[JVMTI Agent] Could not load Jython from jython-standalone.jar" << std::endl;
            std::cout << "[JVMTI Agent] Make sure jython-standalone.jar exists in current directory" << std::endl;
            std::cout << "[JVMTI Agent] Download from: https://www.jython.org/downloads.html" << std::endl;
        }
        
    } catch (...) {
        std::cout << "[JVMTI Agent] Exception occurred while setting up Jython server" << std::endl;
    }
}

// Print information about loaded classes
void print_loaded_classes() {
    jint class_count;
    jclass* classes;
    
    jvmtiError err = g_jvmti->GetLoadedClasses(&class_count, &classes);
    if (err == JVMTI_ERROR_NONE) {
        std::cout << "[JVMTI Agent] Found " << class_count << " loaded classes" << std::endl;
        
        // Print first 10 loaded classes
        int found_count = 0;
        for (int i = 0; i < class_count && found_count < 10; i++) {
            char* class_name = nullptr;
            err = g_jvmti->GetClassSignature(classes[i], &class_name, nullptr);
            if (err == JVMTI_ERROR_NONE && class_name) {
                std::cout << "[JVMTI Agent]   Class: " << class_name << std::endl;
                found_count++;
                g_jvmti->Deallocate((unsigned char*)class_name);
            }
        }
        
        g_jvmti->Deallocate((unsigned char*)classes);
    }
}

// Agent initialization function called when the agent is attached to a running VM
JNIEXPORT jint JNICALL Agent_OnAttach(JavaVM* vm, char* options, void* reserved) {
    std::cout << "[JVMTI Agent] Agent_OnAttach called" << std::endl;
    std::cout << "[JVMTI Agent] Options: " << (options ? options : "(none)") << std::endl;
    
    // Get JVMTI environment
    jint result = vm->GetEnv((void**)&g_jvmti, JVMTI_VERSION_1_2);
    if (result != JNI_OK || g_jvmti == nullptr) {
        std::cerr << "[JVMTI Agent] Failed to get JVMTI environment" << std::endl;
        return JNI_ERR;
    }
    
    // Get JNI environment directly from JavaVM
    result = vm->GetEnv((void**)&g_jni, JNI_VERSION_1_8);
    if (result != JNI_OK || g_jni == nullptr) {
        std::cerr << "[JVMTI Agent] Failed to get JNI environment directly" << std::endl;
        return JNI_ERR;
    }
    
    std::cout << "[JVMTI Agent] Got JNI environment directly from JavaVM!" << std::endl;
    
    // Start Jython socket server immediately since we have JNI (run on main thread)
    if (!g_jython_server_started) {
        start_jython_socket_server();
        g_jython_server_started = true;
    }
    
    // Print loaded classes
    print_loaded_classes();
    
    std::cout << "[JVMTI Agent] Agent successfully attached" << std::endl;
    return JNI_OK;
}

// Agent cleanup function called when the agent is unloaded
JNIEXPORT void JNICALL Agent_OnUnload(JavaVM* vm) {
    std::cout << "[JVMTI Agent] Agent_OnUnload called" << std::endl;
    std::cout << "[JVMTI Agent] Agent cleanup completed" << std::endl;
}