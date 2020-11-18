/*
 * Copyright (C) 2019-2020 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "shared/source/kernel/dispatch_kernel_encoder_interface.h"
#include "shared/source/unified_memory/unified_memory.h"

#include "level_zero/core/source/kernel/kernel.h"

#include <memory>

namespace L0 {

struct GraphicsAllocation;

struct KernelImp : Kernel {
    KernelImp(Module *module);

    ~KernelImp() override;

    ze_result_t destroy() override {
        delete this;
        return ZE_RESULT_SUCCESS;
    }

    ze_result_t setIndirectAccess(ze_kernel_indirect_access_flags_t flags) override;
    ze_result_t getIndirectAccess(ze_kernel_indirect_access_flags_t *flags) override;
    ze_result_t getSourceAttributes(uint32_t *pSize, char **pString) override;

    ze_result_t getProperties(ze_kernel_properties_t *pKernelProperties) override;

    ze_result_t setArgumentValue(uint32_t argIndex, size_t argSize, const void *pArgValue) override;

    void setGroupCount(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;

    ze_result_t setGroupSize(uint32_t groupSizeX, uint32_t groupSizeY,
                             uint32_t groupSizeZ) override;

    ze_result_t suggestGroupSize(uint32_t globalSizeX, uint32_t globalSizeY, uint32_t globalSizeZ,
                                 uint32_t *groupSizeX, uint32_t *groupSizeY,
                                 uint32_t *groupSizeZ) override;

    ze_result_t getKernelName(size_t *pSize, char *pName) override;

    ze_result_t suggestMaxCooperativeGroupCount(uint32_t *totalGroupCount) override;

    const uint8_t *getCrossThreadData() const override { return crossThreadData.get(); }
    uint32_t getCrossThreadDataSize() const override { return crossThreadDataSize; }

    const std::vector<NEO::GraphicsAllocation *> &getResidencyContainer() const override {
        return residencyContainer;
    }

    ze_result_t setArgImmediate(uint32_t argIndex, size_t argSize, const void *argVal);

    ze_result_t setArgBuffer(uint32_t argIndex, size_t argSize, const void *argVal);

    ze_result_t setArgRedescribedImage(uint32_t argIndex, ze_image_handle_t argVal) override;

    ze_result_t setArgBufferWithAlloc(uint32_t argIndex, uintptr_t argVal, NEO::GraphicsAllocation *allocation) override;

    ze_result_t setArgImage(uint32_t argIndex, size_t argSize, const void *argVal);

    ze_result_t setArgSampler(uint32_t argIndex, size_t argSize, const void *argVal);

    virtual void setBufferSurfaceState(uint32_t argIndex, void *address, NEO::GraphicsAllocation *alloc) = 0;

    ze_result_t initialize(const ze_kernel_desc_t *desc);

    const uint8_t *getPerThreadData() const override { return perThreadDataForWholeThreadGroup; }
    uint32_t getPerThreadDataSizeForWholeThreadGroup() const override { return perThreadDataSizeForWholeThreadGroup; }

    uint32_t getPerThreadDataSize() const override { return perThreadDataSize; }
    uint32_t getNumThreadsPerThreadGroup() const override { return numThreadsPerThreadGroup; }
    uint32_t getThreadExecutionMask() const override { return threadExecutionMask; }

    NEO::GraphicsAllocation *getPrintfBufferAllocation() override { return this->printfBuffer; }
    void printPrintfOutput() override;

    const uint8_t *getSurfaceStateHeapData() const override { return surfaceStateHeapData.get(); }
    uint32_t getSurfaceStateHeapDataSize() const override { return surfaceStateHeapDataSize; }

    const uint8_t *getDynamicStateHeapData() const override { return dynamicStateHeapData.get(); }

    const KernelImmutableData *getImmutableData() const override { return kernelImmData; }

    UnifiedMemoryControls getUnifiedMemoryControls() const override { return unifiedMemoryControls; }
    bool hasIndirectAllocationsAllowed() const override;

    const NEO::KernelDescriptor &getKernelDescriptor() const override {
        return kernelImmData->getDescriptor();
    }
    const uint32_t *getGroupSize() const override {
        return groupSize;
    }
    uint32_t getSlmTotalSize() const override;
    NEO::GraphicsAllocation *getIsaAllocation() const override;

    uint32_t getRequiredWorkgroupOrder() const override { return requiredWorkgroupOrder; }
    bool requiresGenerationOfLocalIdsByRuntime() const override { return kernelRequiresGenerationOfLocalIdsByRuntime; }

  protected:
    KernelImp() = default;

    void patchWorkgroupSizeInCrossThreadData(uint32_t x, uint32_t y, uint32_t z);

    void createPrintfBuffer();
    void setDebugSurface();
    virtual void evaluateIfRequiresGenerationOfLocalIdsByRuntime(const NEO::KernelDescriptor &kernelDescriptor) = 0;
    void *patchBindlessSurfaceState(NEO::GraphicsAllocation *alloc, uint32_t bindless);

    const KernelImmutableData *kernelImmData = nullptr;
    Module *module = nullptr;

    typedef ze_result_t (KernelImp::*KernelArgHandler)(uint32_t argIndex, size_t argSize, const void *argVal);
    std::vector<KernelImp::KernelArgHandler> kernelArgHandlers;
    std::vector<NEO::GraphicsAllocation *> residencyContainer;

    NEO::GraphicsAllocation *printfBuffer = nullptr;

    uint32_t groupSize[3] = {0u, 0u, 0u};
    uint32_t numThreadsPerThreadGroup = 1u;
    uint32_t threadExecutionMask = 0u;

    std::unique_ptr<uint8_t[]> crossThreadData = 0;
    uint32_t crossThreadDataSize = 0;

    std::unique_ptr<uint8_t[]> surfaceStateHeapData = nullptr;
    uint32_t surfaceStateHeapDataSize = 0;

    std::unique_ptr<uint8_t[]> dynamicStateHeapData = nullptr;
    uint32_t dynamicStateHeapDataSize = 0;

    uint8_t *perThreadDataForWholeThreadGroup = nullptr;
    uint32_t perThreadDataSizeForWholeThreadGroupAllocated = 0;
    uint32_t perThreadDataSizeForWholeThreadGroup = 0u;
    uint32_t perThreadDataSize = 0u;

    UnifiedMemoryControls unifiedMemoryControls;
    std::vector<uint32_t> slmArgSizes;
    uint32_t slmArgsTotalSize = 0U;
    uint32_t requiredWorkgroupOrder = 0u;

    bool kernelRequiresGenerationOfLocalIdsByRuntime = true;
};

} // namespace L0
