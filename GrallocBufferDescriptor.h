/*
 * Copyright (C) 2019 GlobalLogic
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <android/hardware/graphics/mapper/3.0/IMapper.h>

namespace android {
namespace hardware {
namespace graphics {
namespace mapper {
namespace V3_0 {
namespace renesas {
namespace passthrough {

using android::hardware::graphics::common::V1_2::PixelFormat;

/**
 * BufferDescriptor is created by IMapper and consumed by IAllocator. It is
 * versioned so that IMapper and IAllocator can be updated independently.
 */
constexpr uint32_t grallocBufferDescriptorSize = 7;
constexpr uint32_t grallocBufferDescriptorMagicVersion = ((0x9487 << 16) | 0);

inline BufferDescriptor grallocEncodeBufferDescriptor(
    const IMapper::BufferDescriptorInfo& description) {
    BufferDescriptor descriptor;
    descriptor.resize(grallocBufferDescriptorSize);
    descriptor[0] = grallocBufferDescriptorMagicVersion;
    descriptor[1] = description.width;
    descriptor[2] = description.height;
    descriptor[3] = description.layerCount;
    descriptor[4] = static_cast<uint32_t>(description.format);
    descriptor[5] = static_cast<uint32_t>(description.usage);
    descriptor[6] = static_cast<uint32_t>(description.usage >> 32);

    return descriptor;
}

inline bool grallocDecodeBufferDescriptor(const BufferDescriptor& descriptor,
                                          IMapper::BufferDescriptorInfo* outDescriptorInfo) {
    if (descriptor.size() != grallocBufferDescriptorSize ||
        descriptor[0] != grallocBufferDescriptorMagicVersion) {
        return false;
    }

    *outDescriptorInfo = IMapper::BufferDescriptorInfo {
        descriptor[1],
        descriptor[2],
        descriptor[3],
        static_cast<PixelFormat>(descriptor[4]),
        (static_cast<uint64_t>(descriptor[6]) << 32) | descriptor[5],
    };

    return true;
}

}  // namespace passthrough
}  // namespace renesas
}  // namespace V3_0
}  // namespace mapper
}  // namespace graphics
}  // namespace hardware
}  // namespace android
