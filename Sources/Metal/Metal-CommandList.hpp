#ifndef TITANIUM_METAL_COMMAND_LIST_H
#define TITANIUM_METAL_COMMAND_LIST_H

#include <Titanium/RHI-BaseCommandList.hpp>

namespace TiRHI::Metal
{
    class RHI;
    class RenderTarget;
    class RenderPassDescriptor;
    class GraphicPipeline;

    class CommandList : public BaseCommandList<CommandList, RHI, RenderTarget, RenderPassDescriptor, GraphicPipeline>
    {
    public:
        CommandList() = delete;
        ~CommandList();
        RHI_MOVE_ONLY(CommandList)
        CommandList(RHI& rhi);

        bool build();

    private:
    };
} // namespace TiRHI::Metal


#endif // TITANIUM_METAL_COMMAND_LIST_H