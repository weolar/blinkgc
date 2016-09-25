
#include "config.h"
#include "platform/Platform.h"
#include "public/platform/WebScheduler.h"
#include "public/platform/WebTraceLocation.h"
#include "WebSchedulerImpl.h"
#include "WebThreadImpl.h"

namespace blink {

WebSchedulerImpl::WebSchedulerImpl(WebThreadImpl* thread)
    : m_thread(thread)
{

}

WebSchedulerImpl::~WebSchedulerImpl() { }

void WebSchedulerImpl::shutdown() 
{
}

bool WebSchedulerImpl::shouldYieldForHighPriorityWork()
{
    return false; 
}

bool WebSchedulerImpl::canExceedIdleDeadlineIfRequired() { DebugBreak(); return false; }

void WebSchedulerImpl::postIdleTask(const blink::WebTraceLocation&, blink::WebThread::IdleTask*) { DebugBreak(); }

void WebSchedulerImpl::postNonNestableIdleTask(const blink::WebTraceLocation&, blink::WebThread::IdleTask*) { DebugBreak(); }

void WebSchedulerImpl::postIdleTaskAfterWakeup(const blink::WebTraceLocation&, blink::WebThread::IdleTask*) { DebugBreak(); }

void WebSchedulerImpl::postLoadingTask(const blink::WebTraceLocation& location, blink::WebThread::Task* task) { m_thread->postDelayedTask(location, task, 0); }

void WebSchedulerImpl::postTimerTask(const blink::WebTraceLocation& location, blink::WebThread::Task* task, long long delayMs)
{
    m_thread->postDelayedTask(location, task, delayMs);
}

void WebSchedulerImpl::postTimerTaskAt(const blink::WebTraceLocation&, blink::WebThread::Task*, double monotonicTime) { DebugBreak(); }

void WebSchedulerImpl::suspendTimerQueue() 
{
    m_thread->suspendTimerQueue();
}

void WebSchedulerImpl::resumeTimerQueue() 
{
    m_thread->resumeTimerQueue();
}

void WebSchedulerImpl::postIdleTask(const blink::WebTraceLocation&, PassOwnPtr<IdleTask>) { DebugBreak(); }
void WebSchedulerImpl::postNonNestableIdleTask(const blink::WebTraceLocation&, PassOwnPtr<IdleTask>) { DebugBreak(); }
void WebSchedulerImpl::postIdleTaskAfterWakeup(const blink::WebTraceLocation&, PassOwnPtr<IdleTask>) { DebugBreak(); }
void WebSchedulerImpl::postLoadingTask(const blink::WebTraceLocation&, PassOwnPtr<Task>) { DebugBreak(); }

} // content