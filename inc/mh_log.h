#ifndef _MH_LOG_H_
#define _MH_LOG_H_

/*
 * Logging functions. Currently they are just stubs
 * which will be replaced with something more useful
 * in the future
 */

#ifdef MH_DEBUG
#define MH_LOGD(...) MH_LOG(...)

#else
#define MH_LOGD(...)
#endif

#if defined(MH_DEBUG) && defined(MH_DEBUG_INTERRUPT)
#define MH_LOGD_ISR(...) MH_LOG(...)
#else
#define MH_LOGD_ISR(...)
#endif

#define MH_LOGI MH_LOG
#define MH_LOGE MH_LOG

#define MH_LOG(...)

#endif /* _MH_LOG_H_ */
