#include <Vulkan-Device.hpp>

#include <string>
#include <Titanium/Log.hpp>
#include <Vulkan-Instance.hpp>
#include <Vulkan/Vulkan-RHI.hpp>
#include <Vulkan-Header.hpp>
#include <Vulkan/Vulkan-Surface.hpp>
#include <Vulkan/Vulkan-SwapChain.hpp>
#include <Vulkan/Vulkan-CommandList.hpp>

namespace TiRHI::Vulkan
{
    std::vector<const char*> getDeviceVkExtensionName(const std::span<const Adapter::Features>& features)
    {
        std::vector<const char*> out;

        const auto addUnique = [&out](const char* extension)
        {
            if (std::ranges::find(out, extension) == out.end())
                out.push_back(extension);
        };

        addUnique(VK_KHR_SWAPCHAIN_EXTENSION_NAME); // requires

        for (const Adapter::Features feature : features)
        {
            switch (feature)
            {
            case Adapter::Features::RayQuery:
                addUnique(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
                addUnique(VK_KHR_RAY_QUERY_EXTENSION_NAME);
                addUnique(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
                break;

            case Adapter::Features::RayTracingPipeline:
                addUnique(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
                addUnique(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
                addUnique(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
                break;

            case Adapter::Features::MeshShader:
                addUnique(VK_EXT_MESH_SHADER_EXTENSION_NAME);
                break;
            }
        }

        return out;
    }

    Device::Device(RHI& rhi)
        : BaseDevice<Device, RHI>(rhi)
    {
    }

    bool Device::build(RHI& rhi, Surface& surface, const std::span<const Adapter>& adapters,
                       std::optional<size_t> index)
    {
        const size_t adaptaterIndex = index ? *index : BaseDevice::getBestAdapter(adapters);

        const std::vector<vk::PhysicalDevice> nativePhysicalDevice = rhi.getNativeInstance().enumeratePhysicalDevices();
#undef min;
        if (adaptaterIndex >= nativePhysicalDevice.size())
            return false;

        assert(adapters.size() == nativePhysicalDevice.size());
        return createDevice(nativePhysicalDevice[adaptaterIndex], surface.getSurfaceNative(), adapters[adaptaterIndex]);
    }

    void Device::wait()
    {
        m_device->waitIdle();
    }

    void Device::submit(SwapChain& swapChain, CommandList& commandList)
    {
        vk::SubmitInfo submitInfo{};

        std::array waitSemaphore = {swapChain.getNativeImageAvailableSemaphore()};
        std::array waitStage = {static_cast<vk::PipelineStageFlags>(vk::PipelineStageFlagBits::eColorAttachmentOutput)};
        std::array commandBuffers = {commandList.getcurrentFrameCmb()};
        std::array signalSemaphore = {swapChain.getNativeRenderFinishedSemaphore()};

        // clang-format off
        submitInfo
            .setWaitSemaphoreCount(static_cast<uint32_t>(waitSemaphore.size()))
            .setPWaitSemaphores(waitSemaphore.data())
            .setWaitDstStageMask(waitStage)
            .setCommandBuffers(commandBuffers)
            .setSignalSemaphores(signalSemaphore);
        // clang-format on

        getNativeGraphicQueue().submit(submitInfo, swapChain.getNativeImageInFlightFence());
    }

    bool Device::createDevice(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, const Adapter& adapter)
    {
        m_physicalDevice = physicalDevice;
        m_queueProperties = Private::DeviceQueueProperties(physicalDevice, surface);
        auto properties = m_queueProperties.getQueuProperties();

        size_t allPropertiesQueuIndex = std::numeric_limits<size_t>::max();

        for (const auto& queueProp : properties)
        {
            if (queueProp.graphic && queueProp.compute && queueProp.present && queueProp.transfer)
            {
                allPropertiesQueuIndex = queueProp.index;
                break;
            }
        }

        if (allPropertiesQueuIndex == std::numeric_limits<size_t>::max())
        {
            RHI_LOG_ERROR(L"Failed to find an valid queue", RhiApi::Vulkan);
            return false;
        }

        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfo = {};
        std::array<float, 1> queuePriority = {1.f};
        queueCreateInfo.resize(1);

        for (uint32_t i = 0; i < 1; i++)
        {
            queueCreateInfo[i].sType = vk::StructureType::eDeviceQueueCreateInfo;
            queueCreateInfo[i].queueFamilyIndex = allPropertiesQueuIndex;
            queueCreateInfo[i].queueCount = 1;
            queueCreateInfo[i].pQueuePriorities = queuePriority.data();
        }

        const std::vector<const char*> getDeviceExtensionName = getDeviceVkExtensionName(adapter.getFeatures());

        vk::DeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = vk::StructureType::eDeviceCreateInfo;
        deviceCreateInfo.pNext = nullptr;
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfo.data();
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfo.size());
        deviceCreateInfo.pEnabledFeatures = nullptr;

        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(getDeviceExtensionName.size());
        deviceCreateInfo.ppEnabledExtensionNames = getDeviceExtensionName.data();

        m_device = physicalDevice.createDeviceUnique(deviceCreateInfo);
        RHI_LOG_INFO(std::format(L"Create Device success {}",
                                 [&]()
                                 {
                                     const std::string_view name = adapter.getName();
                                     return std::wstring(name.begin(), name.end());
                                 }()),
                     RhiApi::Vulkan);

        m_graphicQueue = m_device->getQueue(allPropertiesQueuIndex, 0);
        m_graphicQueueIndex = allPropertiesQueuIndex;
        m_presentQueue = m_graphicQueue;
        m_presentQueueIndex = m_graphicQueueIndex;

        return m_graphicQueue && m_presentQueue;
    }

} // namespace TiRHI::Vulkan
