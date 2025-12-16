// Copyright 2024 NVIDIA Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "buffer_backend_loader.hpp"

#include <iostream>

#include "rosidl_buffer_registry/buffer_backend_registry.hpp"
#include "rosidl_typesupport_fastrtps_cpp/buffer_serialization.hpp"

namespace rmw_fastrtps_cpp
{

// Function pointer type for descriptor registration functions
using RegisterDescriptorFunc = void (*)();

void initialize_buffer_backends()
{
  std::cerr << "[RMW FastRTPS] Initializing buffer backends...\n";
  
  // Note: CPU backend doesn't need registration - it's handled directly
  // in buffer_serialization.hpp by serializing as std::vector<T>
  
  // Load all available buffer backends via pluginlib into the generic registry
  // Each backend is completely serialization-independent
  try {
    std::cerr << "[RMW FastRTPS] Loading buffer backend plugins via pluginlib...\n";
    rosidl_buffer_registry::BufferBackendRegistry::get_instance().load_plugins();
  } catch (const std::exception & e) {
    std::cerr << "[RMW FastRTPS] Plugin loading exception: " << e.what() << "\n";
    // Non-fatal: No buffer backend plugins found
    // This is expected on systems without vendor buffer support (CPU-only systems)
  }

  // Populate global maps in rosidl_typesupport_fastrtps_cpp
  // Map 1: Backend descriptor operations (technology-independent)
  // Map 2: FastCDR descriptor serializers (technology-specific)
  auto & generic_registry = rosidl_buffer_registry::BufferBackendRegistry::get_instance();
  std::cerr << "[RMW FastRTPS] Generic registry instance at: " << &generic_registry << "\n";

  auto & backend_ops = rosidl_typesupport_fastrtps_cpp::get_backend_descriptor_ops();
  std::cerr << "[RMW FastRTPS] Backend ops map at: " << &backend_ops << "\n";

  auto backend_names = generic_registry.get_backend_names();
  std::cerr << "[RMW FastRTPS] Found " << backend_names.size() << " backend(s)\n";
  
  for (const auto & backend_name : backend_names) {
    std::cerr << "[RMW FastRTPS] Processing backend: " << backend_name << "\n";
    
    auto backend = generic_registry.get_backend(backend_name);
    if (!backend) {
      std::cerr << "[RMW FastRTPS]   ERROR: Backend pointer is null!\n";
      continue;
    }

    std::string backend_type = backend->get_backend_type();
    std::cerr << "[RMW FastRTPS]   Backend type: " << backend_type << "\n";
    std::cerr << "[RMW FastRTPS]   Descriptor type: " << backend->get_descriptor_type_name() << "\n";
    
    // Populate backend descriptor operations map
    rosidl_typesupport_fastrtps_cpp::BackendDescriptorOps ops;
    ops.descriptor_type_name = backend->get_descriptor_type_name();
    
    auto backend_ptr = backend;  // Capture for lambdas
    ops.create_descriptor = [backend_ptr](const std::shared_ptr<void> & impl) -> std::shared_ptr<void> {
      return backend_ptr->create_descriptor(impl);
    };
    ops.from_descriptor = [backend_ptr](const std::shared_ptr<void> & descriptor) -> std::shared_ptr<void> {
      return backend_ptr->from_descriptor(descriptor);
    };
    
    backend_ops[backend_type] = ops;
    std::cerr << "[RMW FastRTPS]   ✓ Registered backend ops for: " << backend_type << "\n";
    
    // Call FastCDR registration function to populate serializers map
    std::cerr << "[RMW FastRTPS]   Getting FastCDR registration function...\n";
    void * reg_func_ptr = backend->get_descriptor_registration_function();
    
    if (reg_func_ptr) {
      std::cerr << "[RMW FastRTPS]   Found registration function at " << reg_func_ptr << ", calling it...\n";
      auto register_func = reinterpret_cast<RegisterDescriptorFunc>(reg_func_ptr);
      register_func();
      std::cerr << "[RMW FastRTPS]   ✓ Successfully called FastCDR registration\n";
    } else {
      std::cerr << "[RMW FastRTPS]   ✗ Backend does not provide FastCDR registration function\n";
    }
  }
  
  std::cerr << "[RMW FastRTPS] Buffer backend initialization complete\n";
  std::cerr << "[RMW FastRTPS] Total backends registered: " << backend_ops.size() << "\n";
}

}  // namespace rmw_fastrtps_cpp
