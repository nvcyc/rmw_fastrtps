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

#ifndef BUFFER_BACKEND_LOADER_HPP_
#define BUFFER_BACKEND_LOADER_HPP_

namespace rmw_fastrtps_cpp
{

/// Initialize buffer backends for serialization.
/// This loads and registers all available buffer backends (CPU, CUDA, etc.).
/// Called once during RMW initialization.
void initialize_buffer_backends();

}  // namespace rmw_fastrtps_cpp

#endif  // BUFFER_BACKEND_LOADER_HPP_

