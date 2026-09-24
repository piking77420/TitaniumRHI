#ifndef TITANIUM_LOG_H
#define TITANIUM_LOG_H

#include <Titanium/RHITypes.hpp>
#include <Titanium/RHI.hpp>

#define RHI_LOG(MESSAGE, API, SEVERITY)                                                                                \
    TiRHI::RHIGlobalState::logCallBack ? TiRHI::RHIGlobalState::logCallBack(MESSAGE, API, SEVERITY) : void(0)

#define RHI_LOG_VERBOSE(MESSAGE, API) RHI_LOG(MESSAGE, API, RhiMessageSeverity::Verbose)

#define RHI_LOG_INFO(MESSAGE, API) RHI_LOG(MESSAGE, API, RhiMessageSeverity::Info)

#define RHI_LOG_WARNING(MESSAGE, API) RHI_LOG(MESSAGE, API, RhiMessageSeverity::Warning)

#define RHI_LOG_ERROR(MESSAGE, API) RHI_LOG(MESSAGE, API, RhiMessageSeverity::Error)

#define RHI_LOG_FATAL(MESSAGE, API) RHI_LOG(MESSAGE, API, RhiMessageSeverity::Fatal)

#endif // TITANIUM_LOG_H
