#ifndef TITANIUM_COMMAND_LIST_H
#define TITANIUM_COMMAND_LIST_H

#include <Titanium/RHI-Object.hpp>
#include <Titanium/Log.hpp>

namespace TiRHI
{
    template<typename T, typename TRHI>
    class BaseCommandList : public Object<T, TRHI>
    {
    public:
        BaseCommandList() = delete;
        ~BaseCommandList() = default;
        RHI_MOVE_CONSTRUCT_ONLY(BaseCommandList)
        BaseCommandList(TRHI& rhi)
            : Object<T, TRHI>(rhi)
        {
            static_assert(std::derived_from<T, BaseCommandList<T, TRHI>>);
        }

    protected:
        struct RecordState
        {
            bool isRecording = false;

            void reset()
            {
                isRecording = false;
            }
        } m_recordState;

        bool onBeginRecord();

        void onEndRecord();
    };

    template<typename T, typename TRHI>
    inline bool BaseCommandList<T, TRHI>::onBeginRecord()
    {
        if (m_recordState.isRecording)
        {
            RHI_LOG_ERROR(L"Try to record commandList while the command list is already in record state",
                          RhiApi::DirectX12);
            return false;
        }

        m_recordState.isRecording = true;

        return true;
    }

    template<typename T, typename TRHI>
    inline void BaseCommandList<T, TRHI>::onEndRecord()
    {
        if (!m_recordState.isRecording)
        {
            RHI_LOG_ERROR(L"Try to edning record commandList while the command list was not in record state",
                          RhiApi::DirectX12);
            return;
        }

        m_recordState.reset();
    }
} // namespace TiRHI

#endif // TITANIUM_COMMAND_LIST_H
