#include <jvmti.h>
#include <jni.h>
#include <iostream>

static jvmtiEnv* jvmti = nullptr;

void PrintCapability(const char* name, jint capability, bool available) {
    const char* status = available ? "✅ AVAILABLE" : "❌ NOT AVAILABLE";
    std::cout << "  " << name << ": " << status << std::endl;
}

void EnumerateCapabilities() {
    std::cout << "\n🔍 JVMTI CAPABILITY ENUMERATION" << std::endl;
    std::cout << "=================================" << std::endl;
    
    // Get potential capabilities (what the JVM supports)
    jvmtiCapabilities potential_caps = {0};
    jvmtiError error = jvmti->GetPotentialCapabilities(&potential_caps);
    
    if (error != JVMTI_ERROR_NONE) {
        std::cout << "❌ Failed to get potential capabilities: " << error << std::endl;
        return;
    }
    
    std::cout << "\n📋 MONITORING CAPABILITIES:" << std::endl;
    PrintCapability("can_tag_objects", 0, potential_caps.can_tag_objects);
    PrintCapability("can_generate_field_modification_events", 0, potential_caps.can_generate_field_modification_events);
    PrintCapability("can_generate_field_access_events", 0, potential_caps.can_generate_field_access_events);
    PrintCapability("can_get_bytecodes", 0, potential_caps.can_get_bytecodes);
    PrintCapability("can_get_synthetic_attribute", 0, potential_caps.can_get_synthetic_attribute);
    PrintCapability("can_get_owned_monitor_info", 0, potential_caps.can_get_owned_monitor_info);
    PrintCapability("can_get_current_contended_monitor", 0, potential_caps.can_get_current_contended_monitor);
    PrintCapability("can_get_monitor_info", 0, potential_caps.can_get_monitor_info);
    
    std::cout << "\n🎯 METHOD & BREAKPOINT CAPABILITIES:" << std::endl;
    PrintCapability("can_pop_frame", 0, potential_caps.can_pop_frame);
    PrintCapability("can_redefine_classes", 0, potential_caps.can_redefine_classes);
    PrintCapability("can_signal_thread", 0, potential_caps.can_signal_thread);
    PrintCapability("can_get_source_file_name", 0, potential_caps.can_get_source_file_name);
    PrintCapability("can_get_line_numbers", 0, potential_caps.can_get_line_numbers);
    PrintCapability("can_get_source_debug_extension", 0, potential_caps.can_get_source_debug_extension);
    PrintCapability("can_access_local_variables", 0, potential_caps.can_access_local_variables);
    PrintCapability("can_maintain_original_method_order", 0, potential_caps.can_maintain_original_method_order);
    
    std::cout << "\n🔄 EVENT GENERATION CAPABILITIES:" << std::endl;
    PrintCapability("can_generate_single_step_events", 0, potential_caps.can_generate_single_step_events);
    PrintCapability("can_generate_exception_events", 0, potential_caps.can_generate_exception_events);
    PrintCapability("can_generate_frame_pop_events", 0, potential_caps.can_generate_frame_pop_events);
    PrintCapability("can_generate_breakpoint_events", 0, potential_caps.can_generate_breakpoint_events);
    PrintCapability("can_suspend", 0, potential_caps.can_suspend);
    PrintCapability("can_redefine_any_class", 0, potential_caps.can_redefine_any_class);
    PrintCapability("can_get_current_thread_cpu_time", 0, potential_caps.can_get_current_thread_cpu_time);
    PrintCapability("can_get_thread_cpu_time", 0, potential_caps.can_get_thread_cpu_time);
    
    std::cout << "\n🚀 ADVANCED CAPABILITIES:" << std::endl;
    PrintCapability("can_generate_method_entry_events", 0, potential_caps.can_generate_method_entry_events);
    PrintCapability("can_generate_method_exit_events", 0, potential_caps.can_generate_method_exit_events);
    PrintCapability("can_generate_all_class_hook_events", 0, potential_caps.can_generate_all_class_hook_events);
    PrintCapability("can_generate_compiled_method_load_events", 0, potential_caps.can_generate_compiled_method_load_events);
    PrintCapability("can_generate_monitor_events", 0, potential_caps.can_generate_monitor_events);
    PrintCapability("can_generate_vm_object_alloc_events", 0, potential_caps.can_generate_vm_object_alloc_events);
    PrintCapability("can_generate_native_method_bind_events", 0, potential_caps.can_generate_native_method_bind_events);
    PrintCapability("can_generate_garbage_collection_events", 0, potential_caps.can_generate_garbage_collection_events);
    PrintCapability("can_generate_object_free_events", 0, potential_caps.can_generate_object_free_events);
    
    std::cout << "\n🔧 CLASS MODIFICATION CAPABILITIES:" << std::endl;
    PrintCapability("can_force_early_return", 0, potential_caps.can_force_early_return);
    PrintCapability("can_retransform_classes", 0, potential_caps.can_retransform_classes);
    PrintCapability("can_retransform_any_class", 0, potential_caps.can_retransform_any_class);
    PrintCapability("can_generate_resource_exhaustion_heap_events", 0, potential_caps.can_generate_resource_exhaustion_heap_events);
    PrintCapability("can_generate_resource_exhaustion_threads_events", 0, potential_caps.can_generate_resource_exhaustion_threads_events);
    
    // Now check what capabilities we currently have
    std::cout << "\n📊 CURRENTLY ENABLED CAPABILITIES:" << std::endl;
    jvmtiCapabilities current_caps = {0};
    error = jvmti->GetCapabilities(&current_caps);
    
    if (error == JVMTI_ERROR_NONE) {
        bool has_any = false;
        
        // Check a few key capabilities
        if (current_caps.can_retransform_classes) {
            std::cout << "  ✅ can_retransform_classes: ENABLED" << std::endl;
            has_any = true;
        }
        if (current_caps.can_redefine_classes) {
            std::cout << "  ✅ can_redefine_classes: ENABLED" << std::endl;
            has_any = true;
        }
        if (current_caps.can_tag_objects) {
            std::cout << "  ✅ can_tag_objects: ENABLED" << std::endl;
            has_any = true;
        }
        if (current_caps.can_generate_method_entry_events) {
            std::cout << "  ✅ can_generate_method_entry_events: ENABLED" << std::endl;
            has_any = true;
        }
        if (current_caps.can_generate_method_exit_events) {
            std::cout << "  ✅ can_generate_method_exit_events: ENABLED" << std::endl;
            has_any = true;
        }
        
        if (!has_any) {
            std::cout << "  🔒 No capabilities currently enabled" << std::endl;
        }
    } else {
        std::cout << "  ❌ Failed to get current capabilities: " << error << std::endl;
    }
    
    std::cout << "\n💡 CAPABILITY TESTING:" << std::endl;
    std::cout << "Testing if we can add some key capabilities..." << std::endl;
    
    // Test adding some capabilities
    jvmtiCapabilities test_caps = {0};
    test_caps.can_retransform_classes = 1;
    test_caps.can_tag_objects = 1;
    
    error = jvmti->AddCapabilities(&test_caps);
    if (error == JVMTI_ERROR_NONE) {
        std::cout << "  ✅ Successfully added retransform and tag capabilities" << std::endl;
    } else {
        std::cout << "  ⚠️  Failed to add test capabilities (error " << error << ")" << std::endl;
        if (error == JVMTI_ERROR_NOT_AVAILABLE) {
            std::cout << "     Reason: Capabilities not available on this JVM" << std::endl;
        } else if (error == JVMTI_ERROR_MUST_POSSESS_CAPABILITY) {
            std::cout << "     Reason: Must possess capability to perform this operation" << std::endl;
        }
    }
    
    std::cout << "\n🎯 CAPABILITY ENUMERATION COMPLETE!" << std::endl;
    std::cout << "Use this information to understand what JVMTI features are available." << std::endl;
}

extern "C" JNIEXPORT jint JNICALL Agent_OnAttach(JavaVM* vm, char* options, void* reserved) {
    std::cout << "\n🔍 JVMTI CAPABILITY ENUMERATION AGENT" << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cout << "This agent will enumerate all available JVMTI capabilities" << std::endl;
    
    // Get JVMTI environment
    jint result = vm->GetEnv((void**)&jvmti, JVMTI_VERSION_1_0);
    if (result != JNI_OK || jvmti == nullptr) {
        std::cout << "❌ Failed to get JVMTI environment: " << result << std::endl;
        return JNI_ERR;
    }
    
    std::cout << "✅ JVMTI environment obtained successfully" << std::endl;
    
    // Enumerate capabilities
    EnumerateCapabilities();
    
    return JNI_OK;
}

extern "C" JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM* vm, char* options, void* reserved) {
    return Agent_OnAttach(vm, options, reserved);
}

extern "C" JNIEXPORT void JNICALL Agent_OnUnload(JavaVM* vm) {
    std::cout << "🔄 Capability enumeration agent unloading..." << std::endl;
}