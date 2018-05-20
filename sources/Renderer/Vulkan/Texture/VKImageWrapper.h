/*
 * VKImageWrapper.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_IMAGE_WRAPPER_H
#define LLGL_VK_IMAGE_WRAPPER_H


#include <LLGL/Texture.h>
#include <vulkan/vulkan.h>
#include "../VKPtr.h"
#include <cstdint>


namespace LLGL
{


class VKDeviceMemoryRegion;
class VKDeviceMemoryManager;

class VKImageWrapper
{

    public:

        VKImageWrapper(const VKPtr<VkDevice>& device);

        void AllocateAndBindMemoryRegion(VKDeviceMemoryManager& deviceMemoryMngr);

        void BindMemoryRegion(VkDevice device, VKDeviceMemoryRegion* memoryRegion);

        void CreateVkImage(
            VkDevice device,
            VkImageType imageType,
            VkFormat format,
            const VkExtent3D& extent,
            std::uint32_t numMipLevels,
            std::uint32_t numArrayLayers,
            VkImageCreateFlags createFlags,
            VkSampleCountFlagBits samplesFlags,
            VkImageUsageFlags usageFlags
        );

        void CreateVkImageView(
            VkDevice device,
            VkImageViewType viewType,
            VkFormat format,
            VkImageAspectFlags aspectFlags,
            std::uint32_t baseMipLevel,
            std::uint32_t numMipLevels,
            std::uint32_t baseArrayLayer,
            std::uint32_t numArrayLayers,
            VkImageView* imageViewRef
        );

        // Returns the Vulkan image object.
        inline VkImage GetVkImage() const
        {
            return image_;
        }

        // Returns the region of the hardware device memory.
        inline VKDeviceMemoryRegion* GetMemoryRegion() const
        {
            return memoryRegion_;
        }

    private:

        VKPtr<VkImage>          image_;
        VKDeviceMemoryRegion*   memoryRegion_   = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
