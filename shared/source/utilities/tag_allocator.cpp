/*
 * Copyright (C) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/utilities/tag_allocator.h"

namespace NEO {

TagAllocatorBase::TagAllocatorBase(const std::vector<uint32_t> &rootDeviceIndices, MemoryManager *memMngr, size_t tagCount, size_t tagAlignment, size_t tagSize, bool doNotReleaseNodes, DeviceBitfield deviceBitfield)
    : deviceBitfield(deviceBitfield), rootDeviceIndices(rootDeviceIndices), memoryManager(memMngr), tagCount(tagCount), tagSize(tagSize), doNotReleaseNodes(doNotReleaseNodes) {

    this->tagSize = alignUp(tagSize, tagAlignment);

    for (auto &index : rootDeviceIndices) {
        if (index > maxRootDeviceIndex) {
            maxRootDeviceIndex = index;
        }
    }
}

void TagAllocatorBase::cleanUpResources() {
    for (auto &multiGfxAllocation : gfxAllocations) {
        for (auto &allocation : multiGfxAllocation->getGraphicsAllocations()) {
            memoryManager->freeGraphicsMemory(allocation);
        }
    }

    gfxAllocations.clear();
}

MultiGraphicsAllocation *TagNodeBase::getBaseGraphicsAllocation() const {
    return gfxAllocation;
}

void TagNodeBase::returnTag() {
    allocator->returnTag(this);
}

bool TagNodeBase::canBeReleased() const {
    return (!doNotReleaseNodes) &&
           (isCompleted()) &&
           (getImplicitGpuDependenciesCount() == getImplicitCpuDependenciesCount());
}

} // namespace NEO
