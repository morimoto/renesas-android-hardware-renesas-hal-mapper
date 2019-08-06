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

#define LOG_TAG "android.hardware.graphics.mapper@3.0-impl"

#include "Mapper.h"
#include "GrallocLoader.h"
#include "../hwcomposer/img_gralloc_common_public.h"

namespace android {
namespace hardware {
namespace graphics {
namespace mapper {
namespace V3_0 {
namespace renesas {
namespace hal {

namespace detail {

Return<void> Mapper::createDescriptor(const V3_0::IMapper::BufferDescriptorInfo& description,
                                  IMapper::createDescriptor_cb _hidl_cb) {
        BufferDescriptor descriptor;
        Error error = mHal->createDescriptor(description, &descriptor);
        _hidl_cb(error, descriptor);
        return Void();
    }

Return<void> Mapper::importBuffer(const hidl_handle& rawHandle,
                          IMapper::importBuffer_cb _hidl_cb) {
    if (!rawHandle.getNativeHandle()) {
        _hidl_cb(Error::BAD_BUFFER, nullptr);
        return Void();
    }

    native_handle_t* bufferHandle = nullptr;
    Error error = mHal->importBuffer(rawHandle.getNativeHandle(), &bufferHandle);
    if (error != Error::NONE) {
        _hidl_cb(error, nullptr);
        return Void();
    }

    void* buffer = addImportedBuffer(bufferHandle);
    if (!buffer) {
        mHal->freeBuffer(bufferHandle);
        _hidl_cb(Error::NO_RESOURCES, nullptr);
        return Void();
    }

    _hidl_cb(error, buffer);
    return Void();
}

Return<Error> Mapper::freeBuffer(void* buffer) {
    native_handle_t* bufferHandle = removeImportedBuffer(buffer);
    if (!bufferHandle) {
        return Error::BAD_BUFFER;
    }

    return mHal->freeBuffer(bufferHandle);
}

Return<Error> Mapper::validateBufferSize(void* buffer,
                                 const IMapper::BufferDescriptorInfo& description,
                                 uint32_t stride) {
    const native_handle_t* bufferHandle = getImportedBuffer(buffer);
    if (!bufferHandle) {
        return Error::BAD_BUFFER;
    }

    return mHal->validateBufferSize(bufferHandle, description, stride);
}

Return<void> Mapper::getTransportSize(void* buffer, IMapper::getTransportSize_cb _hidl_cb) {
    const native_handle_t* bufferHandle = getImportedBuffer(buffer);
    if (!bufferHandle) {
        _hidl_cb(Error::BAD_BUFFER, 0, 0);
        return Void();
    }

    uint32_t numFds = 0;
    uint32_t numInts = 0;
    Error error = mHal->getTransportSize(bufferHandle, &numFds, &numInts);
    _hidl_cb(error, numFds, numInts);
    return Void();
}

Return<void> Mapper::lock(void* buffer, uint64_t cpuUsage, const V3_0::IMapper::Rect& accessRegion,
                  const hidl_handle& acquireFence, IMapper::lock_cb _hidl_cb) {
    const native_handle_t* bufferHandle = getImportedBuffer(buffer);
    if (!bufferHandle) {
        _hidl_cb(Error::BAD_BUFFER, nullptr, -1, -1);
        return Void();
    }

    base::unique_fd fenceFd;
    Error error = getFenceFd(acquireFence, &fenceFd);
    if (error != Error::NONE) {
        _hidl_cb(error, nullptr, -1, -1);
        return Void();
    }

    void* data = nullptr;
    error = mHal->lock(bufferHandle, cpuUsage, accessRegion, std::move(fenceFd), &data);
    if (error == Error::NONE) {
        const IMG_native_handle_t* imgHnd2 = reinterpret_cast<const IMG_native_handle_t*>(bufferHandle);
        _hidl_cb(error, data, imgHnd2->uiBpp >> 3, imgHnd2->uiBpp >> 3);
    } else {
        _hidl_cb(error, data, -1, -1);
    }
    return Void();
}

Return<void> Mapper::lockYCbCr(void* buffer, uint64_t cpuUsage, const V3_0::IMapper::Rect& accessRegion,
                       const hidl_handle& acquireFence,
                       IMapper::lockYCbCr_cb _hidl_cb) {
    const native_handle_t* bufferHandle = getImportedBuffer(buffer);
    if (!bufferHandle) {
        _hidl_cb(Error::BAD_BUFFER, YCbCrLayout{});
        return Void();
    }

    base::unique_fd fenceFd;
    Error error = getFenceFd(acquireFence, &fenceFd);
    if (error != Error::NONE) {
        _hidl_cb(error, YCbCrLayout{});
        return Void();
    }

    YCbCrLayout layout{};
    error = mHal->lockYCbCr(bufferHandle, cpuUsage, accessRegion, std::move(fenceFd), &layout);
    _hidl_cb(error, layout);
    return Void();
}

Return<void> Mapper::unlock(void* buffer, IMapper::unlock_cb _hidl_cb) {
    const native_handle_t* bufferHandle = getImportedBuffer(buffer);
    if (!bufferHandle) {
        _hidl_cb(Error::BAD_BUFFER, nullptr);
        return Void();
    }

    base::unique_fd fenceFd;
    Error error = mHal->unlock(bufferHandle, &fenceFd);
    if (error != Error::NONE) {
        _hidl_cb(error, nullptr);
        return Void();
    }

    NATIVE_HANDLE_DECLARE_STORAGE(fenceStorage, 1, 0);
    _hidl_cb(error, getFenceHandle(fenceFd, fenceStorage));
    return Void();
}

Return<void> Mapper::isSupported(const ::android::hardware::graphics::mapper::V3_0::IMapper::BufferDescriptorInfo& description,
                                    isSupported_cb _hidl_cb) {
    _hidl_cb(Error::NONE, mHal->isSupported(description));
    return Void();
}

}  // namespace detail
extern "C" IMapper* HIDL_FETCH_IMapper(const char* /*name*/) {
      return passthrough::GrallocLoader::load();
}

}  // namespace hal
}  // namespace renesas
}  // namespace V3_0
}  // namespace mapper
}  // namespace graphics
}  // namespace hardware
}  // namespace android
