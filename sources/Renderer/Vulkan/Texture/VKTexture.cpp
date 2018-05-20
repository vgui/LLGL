/*
 * VKTexture.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKTexture.h"
#include "../Memory/VKDeviceMemory.h"
#include "../VKTypes.h"
#include "../VKCore.h"
#include <algorithm>


namespace LLGL
{


VKTexture::VKTexture(
    const VKPtr<VkDevice>& device, VKDeviceMemoryManager& deviceMemoryMngr, const TextureDescriptor& desc) :
        Texture       { desc.type                  },
        imageWrapper_ { device                     },
        imageView_    { device, vkDestroyImageView },
        format_       { VKTypes::Map(desc.format)  }
{
    /* Create Vulkan image and allocate memory region */
    CreateImage(device, desc);
    imageWrapper_.AllocateAndBindMemoryRegion(deviceMemoryMngr);
}

Extent3D VKTexture::QueryMipLevelSize(std::uint32_t mipLevel) const
{
    switch (GetType())
    {
        case TextureType::Texture1D:
        case TextureType::Texture1DArray:
            return
            {
                std::max(1u, extent_.width  >> mipLevel),
                numArrayLayers_,
                1u
            };
        case TextureType::Texture2D:
        case TextureType::Texture2DArray:
        case TextureType::TextureCube:
        case TextureType::TextureCubeArray:
        case TextureType::Texture2DMS:
        case TextureType::Texture2DMSArray:
            return
            {
                std::max(1u, extent_.width  >> mipLevel),
                std::max(1u, extent_.height >> mipLevel),
                numArrayLayers_
            };
        case TextureType::Texture3D:
            return
            {
                std::max(1u, extent_.width  >> mipLevel),
                std::max(1u, extent_.height >> mipLevel),
                std::max(1u, extent_.depth  >> mipLevel)
            };
    }
    return { 0u, 0u, 0u };
}

TextureDescriptor VKTexture::QueryDesc() const
{
    TextureDescriptor desc;

    desc.type   = GetType();
    //desc.format = ;

    //todo...

    return desc;
}

void VKTexture::CreateImageView(
    VkDevice device, std::uint32_t baseMipLevel, std::uint32_t numMipLevels,
    std::uint32_t baseArrayLayer, std::uint32_t numArrayLayers, VkImageView* imageViewRef)
{
    imageWrapper_.CreateVkImageView(
        device,
        VKTypes::Map(GetType()),
        format_,
        VK_IMAGE_ASPECT_COLOR_BIT,
        baseMipLevel,
        numMipLevels,
        baseArrayLayer,
        numArrayLayers,
        imageViewRef
    );
}

void VKTexture::CreateInternalImageView(VkDevice device)
{
    CreateImageView(device, 0, GetNumMipLevels(), 0, GetNumArrayLayers(), imageView_.ReleaseAndGetAddressOf());
}


/*
 * ======= Private: =======
 */

static VkImageCreateFlags GetVkImageCreateFlags(const TextureDescriptor& desc)
{
    VkImageCreateFlags flags = 0;

    if (IsCubeTexture(desc.type))
        flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    return flags;
}

static VkImageType GetVkImageType(const TextureType textureType)
{
    if (textureType == TextureType::Texture3D)
        return VK_IMAGE_TYPE_3D;
    if (textureType == TextureType::Texture1D || textureType == TextureType::Texture1DArray)
        return VK_IMAGE_TYPE_1D;
    return VK_IMAGE_TYPE_2D;
}

static VkExtent3D GetVkImageExtent3D(const TextureDescriptor& desc, const VkImageType imageType)
{
    VkExtent3D extent;

    switch (imageType)
    {
        case VK_IMAGE_TYPE_1D:
            extent.width    = std::max(1u, desc.texture1D.width);
            extent.height   = 1u;
            extent.depth    = 1u;
            break;

        case VK_IMAGE_TYPE_2D:
            if (IsCubeTexture(desc.type))
            {
                /* Width and height must be equal for cube textures in Vulkan */
                extent.width    = std::max(1u, std::max(desc.textureCube.width, desc.textureCube.height));
                extent.height   = extent.width;
            }
            else
            {
                extent.width    = std::max(1u, desc.texture2D.width);
                extent.height   = std::max(1u, desc.texture2D.height);
            }
            extent.depth    = 1u;
            break;

        case VK_IMAGE_TYPE_3D:
            extent.width    = std::max(1u, desc.texture3D.width);
            extent.height   = std::max(1u, desc.texture3D.height);
            extent.depth    = std::max(1u, desc.texture3D.depth);
            break;

        default:
            extent.width    = 1u;
            extent.height   = 1u;
            extent.depth    = 1u;
            break;
    }

    return extent;
}

static bool HasTextureMipMaps(const TextureDescriptor& desc)
{
    return (!IsMultiSampleTexture(desc.type) && (desc.flags & TextureFlags::GenerateMips) != 0);
}

static std::uint32_t GetVkImageMipLevels(const TextureDescriptor& desc, const VkExtent3D& extent)
{
    if (HasTextureMipMaps(desc))
        return NumMipLevels(extent.width, extent.height, extent.depth);
    else
        return 1u;
}

static std::uint32_t GetVkImageArrayLayers(const TextureDescriptor& desc, const VkImageType imageType)
{
    switch (imageType)
    {
        case VK_IMAGE_TYPE_1D:
            return std::max(1u, desc.texture1D.layers);

        case VK_IMAGE_TYPE_2D:
            if (IsCubeTexture(desc.type))
                return std::max(1u, desc.textureCube.layers) * 6;
            else
                return std::max(1u, desc.texture2D.layers);

        default:
            return 1u;
    }
}

static VkSampleCountFlagBits GetVkImageSampleCountFlags(const TextureDescriptor& desc)
{
    if (IsMultiSampleTexture(desc.type))
    {
        //TODO:
        //returned value must be a bit value from "VkImageFormatProperties::sampleCounts"
        //that was returned by "vkGetPhysicalDeviceImageFormatProperties"
    }
    return VK_SAMPLE_COUNT_1_BIT;
}

static VkImageUsageFlags GetVkImageUsageFlags(const TextureDescriptor& desc)
{
    VkImageUsageFlags flags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    /* Enable TRANSFER_SRC_BIT image usage when MIP-maps are enabled */
    if (HasTextureMipMaps(desc))
        flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    /* Enable either color or depth-stencil ATTACHMENT_BIT image usage when attachment usage is enabled */
    if ((desc.flags & TextureFlags::AttachmentUsage) != 0)
    {
        if (IsDepthStencilFormat(desc.format))
            flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        else
            flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }

    return flags;
}

void VKTexture::CreateImage(VkDevice device, const TextureDescriptor& desc)
{
    /* Setup texture parameters */
    auto imageType  = GetVkImageType(desc.type);

    extent_         = GetVkImageExtent3D(desc, imageType);
    numMipLevels_   = GetVkImageMipLevels(desc, extent_);
    numArrayLayers_ = GetVkImageArrayLayers(desc, imageType);

    /* Create image object */
    imageWrapper_.CreateVkImage(
        device,
        imageType,
        format_,
        extent_,
        numMipLevels_,
        numArrayLayers_,
        GetVkImageCreateFlags(desc),
        GetVkImageSampleCountFlags(desc),
        GetVkImageUsageFlags(desc)
    );
}


} // /namespace LLGL



// ================================================================================
