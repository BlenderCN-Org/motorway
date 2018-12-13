#include <Shared.h>

#if FLAN_VULKAN
#include "Buffer.h"
#include "CommandList.h"

NativeBufferObject* flan::rendering::CreateBufferImpl( NativeRenderContext* nativeRenderContext, const BufferDesc& description, void* initialData )
{
    VkBuffer nativeBuffer = nullptr;
       
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.flags = 0;
    bufferInfo.size = description.Size;
    bufferInfo.usage = 0;

    if ( description.Type == BufferDesc::CONSTANT_BUFFER ) {
        bufferInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    } else if ( description.Type == BufferDesc::DYNAMIC_VERTEX_BUFFER ) {
        bufferInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    } else if ( description.Type == BufferDesc::VERTEX_BUFFER ) {
        bufferInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    } else if ( description.Type == BufferDesc::DYNAMIC_INDICE_BUFFER ) {
        bufferInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    } else if ( description.Type == BufferDesc::INDICE_BUFFER ) {
        bufferInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    } else if ( description.Type == BufferDesc::UNORDERED_ACCESS_VIEW_BUFFER ) {
        bufferInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    } else if ( description.Type == BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_1D 
        || description.Type == BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_2D
        || description.Type == BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_3D ) {
        bufferInfo.usage |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    }

    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    auto bufferCreationResult = vkCreateBuffer( nativeRenderContext->device, &bufferInfo, nullptr, &nativeBuffer );
    if ( bufferCreationResult != VK_SUCCESS ) {
        FLAN_CERR << "Failed to create buffer description! (error code: " << bufferCreationResult << ")" << std::endl;
        return nullptr;
    }

    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;

    NativeBufferObject* nativeBufferObject = new NativeBufferObject();
    nativeBufferObject->bufferObject = nativeBuffer;
    nativeBufferObject->bufferType = description.Type;
    nativeBufferObject->stride = description.Stride;

    return nativeBufferObject;
}

void flan::rendering::DestroyBufferImpl( NativeRenderContext* nativeRenderContext, NativeBufferObject* bufferObject )
{
    vkDestroyBuffer( nativeRenderContext->device, bufferObject->bufferObject, nullptr );
}

void flan::rendering::UpdateBufferImpl( NativeRenderContext* nativeRenderContext, NativeBufferObject* bufferObject, const void* dataToUpload, const std::size_t dataToUploadSize )
{

}

void flan::rendering::UpdateBufferAsynchronousImpl( NativeCommandList* nativeCmdList, NativeBufferObject* bufferObject, const void* dataToUpload, const std::size_t dataToUploadSize )
{
}

void flan::rendering::UpdateBufferRangeImpl( NativeRenderContext* nativeRenderContext, NativeBufferObject* bufferObject, const void* dataToUpload, const std::size_t dataToUploadSize, const int32_t bufferOffset )
{
}

void flan::rendering::FlushAndUpdateBufferRangeImpl( NativeRenderContext* nativeRenderContext, NativeBufferObject* bufferObject, const void* dataToUpload, const std::size_t dataToUploadSize, const int32_t bufferOffset )
{
}

void flan::rendering::BindBufferCmdImpl( NativeCommandList* nativeCmdList, NativeBufferObject* bufferObject, const uint32_t shaderStagesToBindTo, const uint32_t bindingIndex )
{
    switch ( bufferObject->bufferType ) {
    case BufferDesc::BufferType::CONSTANT_BUFFER:
    {
    } break;
    //    if ( ( shaderStagesToBindTo & SHADER_STAGE_VERTEX ) == SHADER_STAGE_VERTEX ) {
    //        nativeDeviceContext->VSSetConstantBuffers( bindingIndex, 1, &bufferObject->bufferObject );
    //    }

    //    if ( ( shaderStagesToBindTo & SHADER_STAGE_PIXEL ) == SHADER_STAGE_PIXEL ) {
    //        nativeDeviceContext->PSSetConstantBuffers( bindingIndex, 1, &bufferObject->bufferObject );
    //    }

    //    if ( ( shaderStagesToBindTo & SHADER_STAGE_TESSELATION_CONTROL ) == SHADER_STAGE_TESSELATION_CONTROL ) {
    //        nativeDeviceContext->DSSetConstantBuffers( bindingIndex, 1, &bufferObject->bufferObject );
    //    }

    //    if ( ( shaderStagesToBindTo & SHADER_STAGE_TESSELATION_EVALUATION ) == SHADER_STAGE_TESSELATION_EVALUATION ) {
    //        nativeDeviceContext->HSSetConstantBuffers( bindingIndex, 1, &bufferObject->bufferObject );
    //    }

    //    if ( ( shaderStagesToBindTo & SHADER_STAGE_COMPUTE ) == SHADER_STAGE_COMPUTE ) {
    //        nativeDeviceContext->CSSetConstantBuffers( bindingIndex, 1, &bufferObject->bufferObject );
    //    }
    //} break;

    case BufferDesc::BufferType::DYNAMIC_INDICE_BUFFER:
    case BufferDesc::BufferType::INDICE_BUFFER:
    {
        vkCmdBindIndexBuffer( nativeCmdList->cmdBuffer, bufferObject->bufferObject, 0, ( bufferObject->stride == 4 ) ? VkIndexType::VK_INDEX_TYPE_UINT32 : VkIndexType::VK_INDEX_TYPE_UINT16 );
    } break;

    case BufferDesc::BufferType::DYNAMIC_VERTEX_BUFFER:
    case BufferDesc::BufferType::VERTEX_BUFFER:
    {
        constexpr VkDeviceSize OFFSETS = 0ull;
        vkCmdBindVertexBuffers( nativeCmdList->cmdBuffer, 0, 1, &bufferObject->bufferObject, &OFFSETS );
    } break;

    //case BufferDesc::BufferType::UNORDERED_ACCESS_VIEW_BUFFER:
    //{
    //    nativeDeviceContext->CSSetUnorderedAccessViews( bindingIndex, 1, &bufferObject->bufferUAVObject, nullptr );
    //} break;

    default:
        break;
    }
}

void flan::rendering::BindBufferReadOnlyCmdImpl( NativeCommandList* nativeCmdList, NativeBufferObject* bufferObject, const uint32_t shaderStagesToBindTo, const uint32_t bindingIndex )
{
}

void flan::rendering::UnbindBufferCmdImpl( NativeCommandList* nativeCmdList, NativeBufferObject* bufferObject, const uint32_t shaderStagesToBindTo, const uint32_t bindingIndex, const Buffer::BindMode bindMode )
{
}

void flan::rendering::CopyStructureCountImpl( NativeCommandList* nativeCmdList, NativeBufferObject* sourceBufferObject, NativeBufferObject* destinationBufferObject, const uint32_t byteOffset )
{
    VkBufferCopy regionCpy;
    regionCpy.srcOffset = byteOffset;
    regionCpy.dstOffset = 0;
    regionCpy.size = sizeof( uint32_t );

    vkCmdCopyBuffer( nativeCmdList->cmdBuffer, sourceBufferObject->bufferObject, destinationBufferObject->bufferObject, 1, &regionCpy );
}
#endif
