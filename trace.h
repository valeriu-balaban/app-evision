#undef TRACEPOINT_PROVIDER
#define TRACEPOINT_PROVIDER evision_processing

#undef TRACEPOINT_INCLUDE_FILE
#define TRACEPOINT_INCLUDE_FILE ./trace.h

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */


#if !defined(_TRACE_H) || defined(TRACEPOINT_HEADER_MULTI_READ)
#define _TRACE_H

#include <lttng/tracepoint.h>

TRACEPOINT_EVENT(
    evision_processing,
    camera_new_frame, // Comment
    TP_ARGS(),
    TP_FIELDS()
)

#endif /* _TRACE_H */

#include <lttng/tracepoint-event.h>

#ifdef __cplusplus
}
#endif /* __cplusplus */
