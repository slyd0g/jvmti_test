#include <jvmti.h>
#include <iostream>
#include <string>
#include <cstring>

// Function to modify the runtime static field directly
bool ModifyStaticField(jvmtiEnv* jvmti, JNIEnv* env) {
    std::cout << "🔧 RUNTIME MOD: Modifying static 'encoded' field directly in memory..." << std::endl;
    
    // Get the TestTarget class
    jclass target_class = env->FindClass("TestTarget");
    if (target_class == nullptr) {
        std::cout << "❌ Could not find TestTarget class" << std::endl;
        return false;
    }
    
    // Get the static field ID for 'encoded'
    jfieldID encoded_field = env->GetStaticFieldID(target_class, "encoded", "[I");
    if (encoded_field == nullptr) {
        std::cout << "❌ Could not find 'encoded' field" << std::endl;
        return false;
    }
    
    std::cout << "🎯 Found 'encoded' static field" << std::endl;
    
    // Get the current array
    jintArray current_array = (jintArray)env->GetStaticObjectField(target_class, encoded_field);
    if (current_array == nullptr) {
        std::cout << "❌ Could not get current array value" << std::endl;
        return false;
    }
    
    // Get array length
    jsize array_length = env->GetArrayLength(current_array);
    std::cout << "📊 Current array length: " << array_length << std::endl;
    
    // Get current values
    jint* current_values = env->GetIntArrayElements(current_array, nullptr);
    if (current_values != nullptr) {
        std::cout << "� Current array values: {";
        for (int i = 0; i < array_length; i++) {
            std::cout << current_values[i];
            if (i < array_length - 1) std::cout << ", ";
        }
        std::cout << "}" << std::endl;
        
        // Modify the values to produce "slyd0g [hooked by jvm ti]" when XORed with 42
        // Target: "slyd0g [hooked by jvm ti]" (25 characters)
        jint new_values[] = {89, 70, 83, 78, 26, 77, 10, 113, 66, 69, 69, 65, 79, 78, 10, 72, 83, 10, 64, 92, 71, 10, 94, 67, 119};
        int new_array_length = 25;
        
        std::cout << "🔧 Creating new array with " << new_array_length << " elements..." << std::endl;
        
        // Create a new array with the extended values
        jintArray new_array = env->NewIntArray(new_array_length);
        if (new_array == nullptr) {
            std::cout << "❌ Could not create new array" << std::endl;
            env->ReleaseIntArrayElements(current_array, current_values, JNI_ABORT);
            return false;
        }
        
        // Set the new values
        env->SetIntArrayRegion(new_array, 0, new_array_length, new_values);
        
        // Replace the static field with the new array
        env->SetStaticObjectField(target_class, encoded_field, new_array);
        
        std::cout << "✅ Replaced static array with new " << new_array_length << "-element array" << std::endl;
        std::cout << "   New values: {";
        for (int i = 0; i < new_array_length; i++) {
            std::cout << new_values[i];
            if (i < new_array_length - 1) std::cout << ", ";
        }
        std::cout << "}" << std::endl;
        
        // Commit the changes back to the array
        env->ReleaseIntArrayElements(current_array, current_values, JNI_ABORT); // ABORT since we're replacing the array
        
        std::cout << "🎉 Successfully replaced static array in memory!" << std::endl;
        std::cout << "   Result should be: 'slyd0g [hooked by jvm ti]' (when XORed with 42)" << std::endl;
        
        return true;
    }
    
    std::cout << "❌ Could not access array elements" << std::endl;
    return false;
}

// Function to modify bytecode - this will call the runtime field modification
bool ModifySecretString(unsigned char* class_data, jint class_data_len) {
    std::cout << "🔧 BYTECODE MOD: Bytecode modification not needed for runtime field access" << std::endl;
    // We'll do the real work in Agent_OnAttach using JNI
    return true; // Return true so retransformation succeeds
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
        std::cout << "\n🎯 RUNTIME MEMORY MODIFIER: TestTarget class detected!" << std::endl;
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
            std::cout << "✅ RUNTIME MEMORY MODIFIER: Field modification applied!" << std::endl;
        } else {
            std::cout << "⚠️ BYTECODE INTERCEPTOR: No modifications made" << std::endl;
        }
        
        std::cout << "📊 New class size: " << *new_class_data_len << " bytes" << std::endl;
    }
}

// Agent initialization
JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *vm, char *options, void *reserved) {
    std::cout << "\n🔧 RUNTIME MEMORY MODIFIER AGENT LOADING" << std::endl;
    std::cout << "==========================================" << std::endl;
    std::cout << "🎯 Goal: Modify static field values at runtime" << std::endl;
    std::cout << "🔧 Strategy: Direct memory manipulation via JNI" << std::endl;
    
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
    std::cout << "\n🔧 RUNTIME MEMORY MODIFIER AGENT ATTACHING" << std::endl;
    std::cout << "===========================================" << std::endl;
    
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
                
                // After retransformation, modify the static field directly
                std::cout << "\n🔧 Now modifying static field in memory..." << std::endl;
                if (ModifyStaticField(jvmti, env)) {
                    std::cout << "✅ Static field modification successful!" << std::endl;
                } else {
                    std::cout << "❌ Static field modification failed" << std::endl;
                }
            }
        } else {
            std::cout << "⚠️ TestTarget class not yet loaded - will intercept on first load" << std::endl;
        }
    }
    
    std::cout << "\n🎯 RUNTIME MEMORY MODIFIER ACTIVE!" << std::endl;
    std::cout << "Ready to modify static field values at runtime." << std::endl;
    
    return JNI_OK;
}