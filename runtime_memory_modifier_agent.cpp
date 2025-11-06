#include <jvmti.h>
#include <iostream>

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



// Agent attachment (for dynamic loading)
JNIEXPORT jint JNICALL Agent_OnAttach(JavaVM *vm, char *options, void *reserved) {
    std::cout << "\n🔧 RUNTIME MEMORY MODIFIER AGENT ATTACHING" << std::endl;
    std::cout << "===========================================" << std::endl;
    
    // Get JNI environment for direct memory operations
    JNIEnv *env;
    jint result = vm->GetEnv((void**)&env, JNI_VERSION_1_8);
    if (result != JNI_OK) {
        std::cout << "❌ Failed to get JNI environment: " << result << std::endl;
        return JNI_ERR;
    }
    
    // Get JVMTI environment (needed for ModifyStaticField function signature)
    jvmtiEnv *jvmti;
    result = vm->GetEnv((void **)&jvmti, JVMTI_VERSION);
    if (result != JNI_OK) {
        std::cout << "❌ Failed to get JVMTI environment: " << result << std::endl;
        return JNI_ERR;
    }
    
    // Find and modify the TestTarget class directly
    jclass target_class = env->FindClass("TestTarget");
    if (target_class != nullptr) {
        std::cout << "🎯 TestTarget class found, modifying static field..." << std::endl;
        
        if (ModifyStaticField(jvmti, env)) {
            std::cout << "✅ Static field modification successful!" << std::endl;
        } else {
            std::cout << "❌ Static field modification failed" << std::endl;
        }
    } else {
        std::cout << "⚠️ TestTarget class not found" << std::endl;
        return JNI_ERR;
    }
    
    std::cout << "\n🎯 RUNTIME MEMORY MODIFIER ACTIVE!" << std::endl;
    std::cout << "Ready to modify static field values at runtime." << std::endl;
    
    return JNI_OK;
}