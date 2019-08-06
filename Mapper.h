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

#ifndef LOG_TAG
#warning "Mapper.h included without LOG_TAG"
#endif

#include <memory>

#include <android/hardware/graphics/mapper/3.0/IMapper.h>
#include <log/log.h>
#include "MapperHal.h"

#define HAL_PIXEL_FORMAT_VENDOR_EXT(fmt) (0x100 | (fmt & 0xF))

/*      Reserved ** DO NOT USE **    HAL_PIXEL_FORMAT_VENDOR_EXT(0) */
#define HAL_PIXEL_FORMAT_BGRX_8888   HAL_PIXEL_FORMAT_VENDOR_EXT(1)
#define HAL_PIXEL_FORMAT_sBGR_A_8888 HAL_PIXEL_FORMAT_VENDOR_EXT(2)
#define HAL_PIXEL_FORMAT_sBGR_X_8888 HAL_PIXEL_FORMAT_VENDOR_EXT(3)
/*      HAL_PIXEL_FORMAT_RGB_565     HAL_PIXEL_FORMAT_VENDOR_EXT(4) */
/*      HAL_PIXEL_FORMAT_BGRA_8888   HAL_PIXEL_FORMAT_VENDOR_EXT(5) */
#define HAL_PIXEL_FORMAT_NV12        HAL_PIXEL_FORMAT_VENDOR_EXT(6)
#define HAL_PIXEL_FORMAT_sRGB_A_8888 HAL_PIXEL_FORMAT_VENDOR_EXT(7)
#define HAL_PIXEL_FORMAT_sRGB_X_8888 HAL_PIXEL_FORMAT_VENDOR_EXT(8)
#define HAL_PIXEL_FORMAT_NV12_CUSTOM HAL_PIXEL_FORMAT_VENDOR_EXT(9)
#define HAL_PIXEL_FORMAT_NV21_CUSTOM HAL_PIXEL_FORMAT_VENDOR_EXT(10)
#define HAL_PIXEL_FORMAT_UYVY        HAL_PIXEL_FORMAT_VENDOR_EXT(11)
/*      Free for customer use        HAL_PIXEL_FORMAT_VENDOR_EXT(12) */
/*      Free for customer use        HAL_PIXEL_FORMAT_VENDOR_EXT(13) */
/*      Free for customer use        HAL_PIXEL_FORMAT_VENDOR_EXT(14) */
/*      Free for customer use        HAL_PIXEL_FORMAT_VENDOR_EXT(15) */

#define HAL_PIXEL_FORMAT_NV21        (HAL_PIXEL_FORMAT_YCrCb_420_SP)

namespace android {
namespace hardware {
namespace graphics {
namespace mapper {
namespace V3_0 {
namespace renesas {
namespace hal {

namespace detail {

// MapperImpl implements V3_0::IMapper on top of V3_0::hal::MapperHal
class MapperImpl : public V3_0::IMapper {
public:
    bool init(std::unique_ptr<hal::MapperHal> hal) {
        mHal = std::move(hal);
        return true;
    }

    // IMapper 3.0 interface

    Return<void> createDescriptor(const V3_0::IMapper::BufferDescriptorInfo& description,
                                  IMapper::createDescriptor_cb _hidl_cb) override;

    Return<void> importBuffer(const hidl_handle& rawHandle,
                              IMapper::importBuffer_cb _hidl_cb) override;

    Return<Error> freeBuffer(void* buffer) override;

    Return<Error> validateBufferSize(void* buffer,
                                     const IMapper::BufferDescriptorInfo& description,
                                     uint32_t stride);

    Return<void> getTransportSize(void* buffer, IMapper::getTransportSize_cb _hidl_cb);

    Return<void> lock(void* buffer, uint64_t cpuUsage, const V3_0::IMapper::Rect& accessRegion,
                      const hidl_handle& acquireFence, IMapper::lock_cb _hidl_cb) override;

    Return<void> lockYCbCr(void* buffer, uint64_t cpuUsage, const V3_0::IMapper::Rect& accessRegion,
                           const hidl_handle& acquireFence,
                           IMapper::lockYCbCr_cb _hidl_cb) override;

    Return<void> unlock(void* buffer, IMapper::unlock_cb _hidl_cb) override;

    Return<void> isSupported(const ::android::hardware::graphics::mapper::V3_0::IMapper::BufferDescriptorInfo& description,
                            isSupported_cb _hidl_cb) override;

protected:
    // these functions can be overriden to do true imported buffer management
    virtual void* addImportedBuffer(native_handle_t* bufferHandle) {
        return static_cast<void*>(bufferHandle);
    }

    virtual native_handle_t* removeImportedBuffer(void* buffer) {
        return static_cast<native_handle_t*>(buffer);
    }

    virtual const native_handle_t* getImportedBuffer(void* buffer) const {
        return static_cast<const native_handle_t*>(buffer);
    }

    // convert fenceFd to or from hidl_handle
    static Error getFenceFd(const hidl_handle& fenceHandle, base::unique_fd* outFenceFd) {
        auto handle = fenceHandle.getNativeHandle();
        if (handle && handle->numFds > 1) {
            ALOGE("invalid fence handle with %d fds", handle->numFds);
            return Error::BAD_VALUE;
        }

        int fenceFd = (handle && handle->numFds == 1) ? handle->data[0] : -1;
        if (fenceFd >= 0) {
            fenceFd = dup(fenceFd);
            if (fenceFd < 0) {
                return Error::NO_RESOURCES;
            }
        }

        outFenceFd->reset(fenceFd);

        return Error::NONE;
    }

    static hidl_handle getFenceHandle(const base::unique_fd& fenceFd, char* handleStorage) {
        native_handle_t* handle = nullptr;
        if (fenceFd >= 0) {
            handle = native_handle_init(handleStorage, 1, 0);
            handle->data[0] = fenceFd;
        }

        return hidl_handle(handle);
    }

    std::unique_ptr<hal::MapperHal> mHal;
};

}  // namespace detail

using Mapper = detail::MapperImpl;

extern "C" IMapper* HIDL_FETCH_IMapper(const char* name);

}  // namespace hal
}  // namespace renesas
}  // namespace V3_0
}  // namespace mapper
}  // namespace graphics
}  // namespace hardware
}  // namespace android
