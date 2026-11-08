#include <Vulkan-Device.hpp>

#include <string>
#include <Titanium/Log.hpp>
#include <Vulkan-Instance.hpp>
#include <Vulkan/Vulkan-RHI.hpp>
#include <Vulkan-Header.hpp>
#include <Vulkan/Vulkan-Surface.hpp>
#include <Vulkan/Vulkan-SwapChain.hpp>
#include <Vulkan/Vulkan-CommandList.hpp>
#include <vulkan/Vulkan-AcquiredFrame.hpp>

// #define VMA_STATIC_VULKAN_FUNCTIONS 0
// #define VMA_DYNAMIC_VULKAN_FUNCTIONS 1 for volk

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

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
        : BaseDevice(rhi)

    {
    }

    Device& Device::operator=(Device&& other) noexcept
    {
        if (this == &other)
            return *this;

        BaseDevice::operator=(std::move(other));
        // Release fences
        m_synchronisations.clear();
        m_device = std::move(other.m_device);
        m_physicalDevice = std::exchange(other.m_physicalDevice, vk::PhysicalDevice{});
        m_queueProperties = std::move(other.m_queueProperties);
        m_graphicQueue = std::exchange(other.m_graphicQueue, vk::Queue{});
        m_graphicQueueIndex = std::exchange(other.m_graphicQueueIndex, 0);
        m_presentQueue = std::exchange(other.m_presentQueue, vk::Queue{});
        m_presentQueueIndex = std::exchange(other.m_presentQueueIndex, 0);
        m_synchronisations = std::move(other.m_synchronisations);
        m_dispatch = std::exchange(other.m_dispatch, VolkDeviceTable{});
        return *this;
    }

    bool Device::build(RHI& rhi, Surface& surface, const std::span<const Adapter>& adapters,
                       std::optional<size_t> index)
    {
        onBuild(adapters, index);
        const std::vector<vk::PhysicalDevice> nativePhysicalDevice = rhi.getNativeInstance().enumeratePhysicalDevices();
#undef min;
        if (m_adapterIndex >= nativePhysicalDevice.size())
            return false;

        assert(adapters.size() == nativePhysicalDevice.size());
        std::vector<Adapter::Features> finalFeatures;

        // clang-format off
        return checkExtensionToEnableValid(adapters[m_adapterIndex], finalFeatures) &&
               createDevice(nativePhysicalDevice[m_adapterIndex], surface.getSurfaceNative(), adapters[m_adapterIndex], finalFeatures) &&
               initVolkTable() && 
               createSynchronisationPrimitives() && 
               initVmaAllocator(rhi.getInstance());
        // clang-format on
    }

    void Device::wait()
    {
        m_dispatch.vkDeviceWaitIdle(static_cast<VkDevice>(m_device.get()));
    }

    void Device::submitPresent(std::span<const SubmitPresentDesc> submitPresentDesc)
    {
        if (submitPresentDesc.empty())
            return;

        // TODO: Give each window its own frame slots and submission fences.
        // A single shared frame fence couples all windows' progress.
        // Schedule rendering and presentation independently so the debug window
        // can update without waiting for the main renderer's frame.

        struct SubmissionStorage
        {
            std::vector<vk::Semaphore> waitSemaphores;
            std::vector<vk::PipelineStageFlags> waitStages;
            std::vector<vk::CommandBuffer> commandBuffers;
            std::vector<vk::Semaphore> signalSemaphores;
        };

        std::vector<SubmissionStorage> storage(submitPresentDesc.size());
        std::vector<vk::SubmitInfo> submitInfos(submitPresentDesc.size());

        std::vector<vk::Semaphore> presentWaitSemaphores;
        std::vector<vk::SwapchainKHR> swapChains;
        std::vector<uint32_t> swapImageIndices;

        for (size_t i = 0; i < submitPresentDesc.size(); ++i)
        {
            const auto& desc = submitPresentDesc[i];
            auto& data = storage[i];

            if (desc.commandList)
                data.commandBuffers.push_back(desc.commandList->getcurrentFrameCmb());

            for (const auto& frame : desc.acquireFrames)
            {
                data.waitSemaphores.push_back(frame.getImageAvailableSemaphore());

                data.waitStages.push_back(vk::PipelineStageFlagBits::eColorAttachmentOutput);

                const auto renderFinished = frame.getRenderFinishSemaphore();
                data.signalSemaphores.push_back(renderFinished);

                presentWaitSemaphores.push_back(renderFinished);
                swapChains.push_back(frame.getSwapChain());
                swapImageIndices.push_back(frame.getSwapChainImageIndex());
            }

            submitInfos[i]
                .setWaitSemaphores(data.waitSemaphores)
                .setWaitDstStageMask(data.waitStages)
                .setCommandBuffers(data.commandBuffers)
                .setSignalSemaphores(data.signalSemaphores);
        }

        // https://vulkan.lunarg.com/doc/view/1.4.350.0/windows/antora/tutorial/latest/Synchronization/Frame_in_Flight/03_resource_lifetimes.html
        const uint64_t submissionValue = ++m_timeLine.nextSubmissionValue;
        const vk::Semaphore semaphore = m_timeLine.timeLineSemaphore.get();
        auto& last = storage.back();
        last.signalSemaphores.push_back(semaphore);

        // One value per signal semaphore; binary semaphore values are ignored.
        std::vector<uint64_t> signalValues(last.signalSemaphores.size(), 0);
        signalValues.back() = submissionValue;

        vk::TimelineSemaphoreSubmitInfo timelineInfo{};
        timelineInfo.setSignalSemaphoreValues(signalValues);
        submitInfos.back().setSignalSemaphores(last.signalSemaphores).setPNext(&timelineInfo);

        // The current frame fence must already have been waited on and reset.

        // wait fence
        {
            std::array fences = {getNativeInFlightFence()};
            const VkFence* vkFences = reinterpret_cast<const VkFence*>(fences.data());
            const uint32_t fencesCount = static_cast<uint32_t>(fences.size());
            m_dispatch.vkResetFences(m_device.get(), fencesCount, vkFences);
        }
        getNativeGraphicQueue().submit(submitInfos, getNativeInFlightFence());

        for (const auto& desc : submitPresentDesc)
        {
            if (desc.commandList)
            {
                CommandList::Private::setLastSubmissionValue(*desc.commandList, submissionValue);
                Vulkan::Private::UploadBatch& batch =
                    CommandList::Private::getUploadBatch(*desc.commandList)[getRHI().getCurrentFrame()];
                m_uploadManager.retire(std::move(batch), submissionValue);
            }
        }
        m_timeLine.nextSubmissionValueFrames[getRHI().getCurrentFrame()] = submissionValue;

        if (!swapChains.empty())
        {
            vk::PresentInfoKHR presentInfo{};
            presentInfo.setWaitSemaphores(presentWaitSemaphores)
                .setSwapchains(swapChains)
                .setImageIndices(swapImageIndices)
                .setPResults(nullptr);

            const vk::Result result = m_presentQueue.presentKHR(presentInfo);
            VulkanCheckErrorStatus(result);
        }
    }

    void Device::beginFrame()
    {
        std::array fences = {getNativeInFlightFence()};
        const VkFence* vkFences = reinterpret_cast<const VkFence*>(fences.data());
        const uint32_t fencesCount = static_cast<uint32_t>(fences.size());

        m_dispatch.vkWaitForFences(m_device.get(), fencesCount, vkFences, vk::True,
                                   std::numeric_limits<uint64_t>::max());

        // TODO COLLECT DATA
        const uint64_t completed = m_device->getSemaphoreCounterValue(m_timeLine.timeLineSemaphore.get());
        m_deferredDeletionQueue.collect(completed);
        m_uploadManager.collect(*this, completed);
    }

    bool Device::checkExtensionToEnableValid(const Adapter& adapter, std::vector<Adapter::Features>& finalFeatures)
    {
        const std::span<const Adapter::Features> requested = getFeaturesToEnable();
        const std::span<const Adapter::Features> supported = adapter.getFeatures();

        bool allSupported = true;

        for (const Adapter::Features feature : requested)
        {
            const bool isSupported = std::ranges::find(supported, feature) != supported.end();

            if (isSupported)
            {
                finalFeatures.push_back(feature);
            }
            else
            {
                allSupported = false;

                RHI_LOG_WARNING(std::format(L"Requested feature '{}' is not supported by adapter '{}'",
                                            Adapter::toWString(feature), adapter.getNameW()),
                                RhiApi::Vulkan);
            }
        }

        return allSupported;
    }

    bool Device::createDevice(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, const Adapter& adapter,
                              const std::vector<Adapter::Features>& featuresToEnable)
    {
        m_physicalDevice = physicalDevice;
        m_queueProperties = Vulkan::Private::DeviceQueueProperties(physicalDevice, surface);
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

        std::vector<const char*> getDeviceExtensionName = getDeviceVkExtensionName(featuresToEnable);

        vk::PhysicalDeviceTimelineSemaphoreFeatures enabledTimeline{};
        enabledTimeline.setTimelineSemaphore(vk::True);

        vk::DeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = vk::StructureType::eDeviceCreateInfo;
        deviceCreateInfo.pNext = &enabledTimeline;
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfo.data();
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfo.size());
        deviceCreateInfo.pEnabledFeatures = nullptr;

        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(getDeviceExtensionName.size());
        deviceCreateInfo.ppEnabledExtensionNames = getDeviceExtensionName.data();

        m_device = physicalDevice.createDeviceUnique(deviceCreateInfo);
        onCreateDeviceSuccess(adapter, m_device.get() != VK_NULL_HANDLE);
        m_graphicQueue = m_device->getQueue(allPropertiesQueuIndex, 0);
        m_graphicQueueIndex = allPropertiesQueuIndex;
        m_presentQueue = m_graphicQueue;
        m_presentQueueIndex = m_graphicQueueIndex;

        return m_device && m_graphicQueue && m_presentQueue;
    }

    bool Device::createSynchronisationPrimitives()
    {
        m_synchronisations.resize(getRHI().getFrameInFlight());
        vk::FenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;

        bool isOk = true;
        for (size_t i = 0; i < m_synchronisations.size(); i++)
        {
            Synchronisation& s = m_synchronisations[i];
            s.inFlightFence = m_device->createFenceUnique(fenceCreateInfo);
            isOk &= s.inFlightFence.get() != VK_NULL_HANDLE;
        }

        //
        vk::SemaphoreTypeCreateInfo typeInfo{};
        typeInfo.setSemaphoreType(vk::SemaphoreType::eTimeline).setInitialValue(0);

        vk::SemaphoreCreateInfo createInfo{};
        createInfo.setPNext(&typeInfo);
        m_timeLine.timeLineSemaphore = m_device->createSemaphoreUnique(createInfo);
        m_timeLine.nextSubmissionValueFrames.fill(0);

        return isOk && m_timeLine.timeLineSemaphore;
    }

    vk::Fence Device::getNativeInFlightFence() const
    {
        return *m_synchronisations[getRHI().getCurrentFrame()].inFlightFence;
    }

    bool Device::initVolkTable()
    {
        if (!m_device)
            return false;

        volkLoadDeviceTable(&m_dispatch, m_device.get());

        return true;
    }

    bool Device::initVmaAllocator(Vulkan::Instance& instance)
    {
        VmaAllocatorCreateInfo createInfo{};

        createInfo.instance = static_cast<VkInstance>(instance.getInstance());
        createInfo.physicalDevice = static_cast<VkPhysicalDevice>(m_physicalDevice);
        createInfo.device = static_cast<VkDevice>(m_device.get());

        // Must match your enabled/supported Vulkan API version.
        createInfo.vulkanApiVersion = instance.getApiVersion();

        VmaVulkanFunctions functions{};
        functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
        functions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

        createInfo.pVulkanFunctions = &functions;

        VkResult result = vmaCreateAllocator(&createInfo, &m_vmaAllocator.vmaAllocator);

        if (result != VK_SUCCESS)
        {
            RHI_LOG_FATAL(L"Failed to create vma allocator", RhiApi::Vulkan);
            return false;
        }

        return true;
    }

    uint64_t Device::Private::getCurrentFrameSubmissionValue(const Device& device)
    {
        const size_t currentFrame = device.getRHI().getCurrentFrame();

        return device.m_timeLine.nextSubmissionValueFrames[currentFrame];
    }

} // namespace TiRHI::Vulkan
