#include <jvmti.h>
#include <iostream>
#include <string>
#include <cstring>

// Function to modify bytecode - replace the entire GetSecret method with a simple string return
bool ModifySecretString(unsigned char* class_data, jint class_data_len) {
    std::cout << "🔧 BYTECODE MOD: Looking for GetSecret method to replace with 'HOOKED'..." << std::endl;
    
    // Instead of modifying the array, let's replace the method body entirely
    // Look for the pattern in GetSecret method that we can replace
    
    // First, let's try the simpler approach: find and change the static field reference
    // Look for the getstatic instruction that loads the encoded array
    for (int i = 0; i < class_data_len - 2; i++) {
        if (class_data[i] == 0xB2) {  // getstatic opcode
            // Check if this references our encoded field
            uint16_t field_ref = (class_data[i+1] << 8) | class_data[i+2];
            
            std::cout << "🔍 Found getstatic instruction at offset " << i 
                      << " referencing constant pool entry " << field_ref << std::endl;
            
            // If this is referencing constant pool entry #10 (our encoded field)
            if (field_ref == 10) {
                std::cout << "🎯 Found getstatic reference to encoded field!" << std::endl;
                
                // This is more complex - we'd need to create a new array in constant pool
                // Let's try a different approach instead
                
                // Look for the bipush 42 (XOR key) in the same method and replace the whole operation
                // Search ahead for the ixor instruction
                for (int j = i; j < std::min(i + 50, class_data_len - 1); j++) {
                    if (class_data[j] == 0x10 && class_data[j+1] == 42) {  // bipush 42
                        std::cout << "🎯 Found XOR operation at offset " << j << std::endl;
                        
                        // Replace the bipush 42 with bipush 0 to change the XOR behavior
                        class_data[j+1] = 0;
                        std::cout << "✅ Changed XOR key from 42 to 0" << std::endl;
                        
                        return true;
                    }
                }
            }
        }
    }
    
    // Alternative: Force array re-initialization by clearing the static field
    // Look for putstatic instruction that sets the encoded field
    for (int i = 0; i < class_data_len - 2; i++) {
        if (class_data[i] == 0xB3) {  // putstatic opcode
            uint16_t field_ref = (class_data[i+1] << 8) | class_data[i+2];
            
            if (field_ref == 10) {  // encoded field reference
                std::cout << "🎯 Found putstatic to encoded field at offset " << i << std::endl;
                
                // Try to modify the array values before they're stored
                // Look backwards for the array initialization
                for (int j = i - 50; j < i; j++) {
                    if (j >= 0 && class_data[j] == 0x10) {  // bipush
                        unsigned char value = class_data[j+1];
                        
                        // Check if this is one of our array values
                        if (value == 89) {
                            class_data[j+1] = 98;  // H
                            std::cout << "✅ Changed array value 89 -> 98 (H)" << std::endl;
                        } else if (value == 70) {
                            class_data[j+1] = 101; // O
                            std::cout << "✅ Changed array value 70 -> 101 (O)" << std::endl;
                        } else if (value == 83) {
                            class_data[j+1] = 101; // O
                            std::cout << "✅ Changed array value 83 -> 101 (O)" << std::endl;
                        } else if (value == 78) {
                            class_data[j+1] = 97;  // K
                            std::cout << "✅ Changed array value 78 -> 97 (K)" << std::endl;
                        } else if (value == 26) {
                            class_data[j+1] = 111; // E
                            std::cout << "✅ Changed array value 26 -> 111 (E)" << std::endl;
                        } else if (value == 77) {
                            class_data[j+1] = 110; // D
                            std::cout << "✅ Changed array value 77 -> 110 (D)" << std::endl;
                        }
                    }
                }
                return true;
            }
        }
    }
    
    std::cout << "❌ Could not find suitable modification point" << std::endl;
    return false;
}

// JVMTI Callback for class file load hook
void JNICALL ClassFileLoadHook(jvmtiEnv *jvmti,
                               JNIEnv* jni,
                               jclass class_being_redefined,
                               jobject loader,
                               const char* name,
                               jobject protection_domain,
                               jint class_data_len,
                               const unsigned char* class_data,
                               jint* new_class_data_len,
                               unsigned char** new_class_data) {
    
    // Only process TestTarget class
    if (name != nullptr && strcmp(name, "TestTarget") == 0) {
        std::cout << "\n🎯 BYTECODE INTERCEPTOR: TestTarget class detected!" << std::endl;
        std::cout << "📊 Original class size: " << class_data_len << " bytes" << std::endl;
        
        // Allocate new buffer for modified class
        jvmtiError err = jvmti->Allocate(class_data_len, new_class_data);
        if (err != JVMTI_ERROR_NONE) {
            std::cout << "❌ Failed to allocate memory for modified class: " << err << std::endl;
            return;
        }
        
        // Copy original class data
        memcpy(*new_class_data, class_data, class_data_len);
        *new_class_data_len = class_data_len;
        
        // Attempt to modify the secret string
        bool modified = ModifySecretString(*new_class_data, class_data_len);
        
        if (modified) {
            std::cout << "✅ BYTECODE INTERCEPTOR: Class modification applied!" << std::endl;
        } else {
            std::cout << "⚠️ BYTECODE INTERCEPTOR: No modifications made" << std::endl;
        }
        
        std::cout << "📊 New class size: " << *new_class_data_len << " bytes" << std::endl;
    }
}

// Agent initialization
JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *vm, char *options, void *reserved) {
    std::cout << "\n🔧 BYTECODE INTERCEPTOR AGENT LOADING" << std::endl;
    std::cout << "====================================" << std::endl;
    std::cout << "🎯 Goal: Intercept and modify GetSecret() method" << std::endl;
    std::cout << "🔧 Strategy: Direct bytecode manipulation via ClassFileLoadHook" << std::endl;
    
    jvmtiEnv *jvmti;
    jint result = vm->GetEnv((void **)&jvmti, JVMTI_VERSION);
    if (result != JNI_OK) {
        std::cout << "❌ Failed to get JVMTI environment: " << result << std::endl;
        return JNI_ERR;
    }
    
    // Set required capabilities
    jvmtiCapabilities capabilities = {0};
    capabilities.can_retransform_classes = 1;
    capabilities.can_generate_all_class_hook_events = 1;
    
    jvmtiError error = jvmti->AddCapabilities(&capabilities);
    if (error != JVMTI_ERROR_NONE) {
        std::cout << "❌ Failed to add capabilities: " << error << std::endl;
        return JNI_ERR;
    }
    
    std::cout << "✅ Retransformation capabilities acquired" << std::endl;
    
    // Set callback for class file load hook
    jvmtiEventCallbacks callbacks = {0};
    callbacks.ClassFileLoadHook = ClassFileLoadHook;
    
    error = jvmti->SetEventCallbacks(&callbacks, sizeof(callbacks));
    if (error != JVMTI_ERROR_NONE) {
        std::cout << "❌ Failed to set event callbacks: " << error << std::endl;
        return JNI_ERR;
    }
    
    // Enable ClassFileLoadHook event
    error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_CLASS_FILE_LOAD_HOOK, nullptr);
    if (error != JVMTI_ERROR_NONE) {
        std::cout << "❌ Failed to enable ClassFileLoadHook: " << error << std::endl;
        return JNI_ERR;
    }
    
    std::cout << "✅ ClassFileLoadHook enabled for bytecode interception" << std::endl;
    
    return JNI_OK;
}

// Agent attachment (for dynamic loading)
JNIEXPORT jint JNICALL Agent_OnAttach(JavaVM *vm, char *options, void *reserved) {
    std::cout << "\n🔧 BYTECODE INTERCEPTOR AGENT ATTACHING" << std::endl;
    std::cout << "=======================================" << std::endl;
    
    jvmtiEnv *jvmti;
    jint result = vm->GetEnv((void **)&jvmti, JVMTI_VERSION);
    if (result != JNI_OK) {
        std::cout << "❌ Failed to get JVMTI environment: " << result << std::endl;
        return JNI_ERR;
    }
    
    // Set required capabilities
    jvmtiCapabilities capabilities = {0};
    capabilities.can_retransform_classes = 1;
    capabilities.can_generate_all_class_hook_events = 1;
    
    jvmtiError error = jvmti->AddCapabilities(&capabilities);
    if (error != JVMTI_ERROR_NONE) {
        std::cout << "❌ Failed to add capabilities: " << error << std::endl;
        return JNI_ERR;
    }
    
    std::cout << "✅ Retransformation capabilities acquired" << std::endl;
    
    // Set callback for class file load hook
    jvmtiEventCallbacks callbacks = {0};
    callbacks.ClassFileLoadHook = ClassFileLoadHook;
    
    error = jvmti->SetEventCallbacks(&callbacks, sizeof(callbacks));
    if (error != JVMTI_ERROR_NONE) {
        std::cout << "❌ Failed to set event callbacks: " << error << std::endl;
        return JNI_ERR;
    }
    
    // Enable ClassFileLoadHook event
    error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_CLASS_FILE_LOAD_HOOK, nullptr);
    if (error != JVMTI_ERROR_NONE) {
        std::cout << "❌ Failed to enable ClassFileLoadHook: " << error << std::endl;
        return JNI_ERR;
    }
    
    std::cout << "✅ ClassFileLoadHook enabled for bytecode interception" << std::endl;
    
    // Force retransformation of TestTarget class if already loaded
    JNIEnv *env;
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_8) == JNI_OK) {
        jclass target_class = env->FindClass("TestTarget");
        if (target_class != nullptr) {
            std::cout << "🎯 TestTarget class found, triggering retransformation..." << std::endl;
            
            jclass classes[] = { target_class };
            error = jvmti->RetransformClasses(1, classes);
            if (error != JVMTI_ERROR_NONE) {
                std::cout << "❌ Retransformation failed: " << error << std::endl;
            } else {
                std::cout << "✅ TestTarget class retransformed successfully!" << std::endl;
            }
        } else {
            std::cout << "⚠️ TestTarget class not yet loaded - will intercept on first load" << std::endl;
        }
    }
    
    std::cout << "\n🎯 BYTECODE INTERCEPTOR ACTIVE!" << std::endl;
    std::cout << "Ready to intercept and modify GetSecret() method." << std::endl;
    
    return JNI_OK;
}