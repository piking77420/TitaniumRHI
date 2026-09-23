#ifndef TITANIUM_LOG_H
#define TITANIUM_LOG_H

#include <Titanium/RHITypes.hpp>

namespace TiRHI
{
    namespace Private
    {
        inline LogCallBackSignature logCallBack{nullptr};
    } // namespace Private

} // TiRHI

#define RHI_LOG(MESSAGE, LOCATION, API, SEVERITY)                                                                      \
    TiRHI::Private::logCallBack ? TiRHI::Private::logCallBack(MESSAGE, LOCATION, API, SEVERITY) : void(0)

#define RHI_LOG_VERBOSE(MESSAGE, LOCATION, API) RHI_LOG(MESSAGE, LOCATION, API, RhiMessageSeverity::Verbose)

#define RHI_LOG_INFO(MESSAGE, LOCATION, API) RHI_LOG(MESSAGE, LOCATION, API, RhiMessageSeverity::Info)

#define RHI_LOG_WARNING(MESSAGE, LOCATION, API) RHI_LOG(MESSAGE, LOCATION, API, RhiMessageSeverity::Warning)

#define RHI_LOG_ERROR(MESSAGE, LOCATION, API) RHI_LOG(MESSAGE, LOCATION, API, RhiMessageSeverity::Error)

#define RHI_LOG_FATAL(MESSAGE, LOCATION, API) RHI_LOG(MESSAGE, LOCATION, API, RhiMessageSeverity::Fatal)

#endif // TITANIUM_LOG_H
