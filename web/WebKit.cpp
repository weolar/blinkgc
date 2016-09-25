#include "config.h"

#include "public/platform/WebThread.h"
#include "platform/Platform.h"
#include "platform/heap/heap.h"
#include "web/CurrentTimeImpl.h"
#include "public/platform/WebTraceLocation.h"
#include "wtf/MainThread.h"

namespace blink {

static WebThread::TaskObserver* s_endOfTaskRunner = 0;
static WebThread::TaskObserver* s_pendingGCRunner = 0;
static ThreadState::Interruptor* s_messageLoopInterruptor = 0;
static ThreadState::Interruptor* s_isolateInterruptor = 0;

// Make sure we are not re-initialized in the same address space.
// Doing so may cause hard to reproduce crashes.
static bool s_webKitInitialized = false;

namespace {

    class EndOfTaskRunner : public WebThread::TaskObserver {
    public:
        void willProcessTask() override
        {

        }

        void didProcessTask() override
        {

        }
    };

    class MainThreadTaskRunner : public WebThread::Task {
        WTF_MAKE_NONCOPYABLE(MainThreadTaskRunner);
    public:
        MainThreadTaskRunner(WTF::MainThreadFunction* function, void* context)
            : m_function(function)
            , m_context(context) { }

        void run() override
        {
            m_function(m_context);
        }
    private:
        WTF::MainThreadFunction* m_function;
        void* m_context;
    };

} // namespace

class PendingGCRunner : public WebThread::TaskObserver {
public:
    PendingGCRunner() : m_nesting(0) { }

    ~PendingGCRunner()
    {
        // m_nesting can be 1 if this was unregistered in a task and
        // didProcessTask was not called.
        ASSERT(!m_nesting || m_nesting == 1);
    }

    virtual void willProcessTask()
    {
        m_nesting++;
    }

    virtual void didProcessTask()
    {
        // In the production code WebKit::initialize is called from inside the
        // message loop so we can get didProcessTask() without corresponding
        // willProcessTask once. This is benign.
        if (m_nesting)
            m_nesting--;

        ThreadState* state = ThreadState::current();
        state->safePoint(m_nesting ? ThreadState::HeapPointersOnStack : ThreadState::NoHeapPointersOnStack);
    }

private:
    int m_nesting;
};

class MessageLoopInterruptor : public ThreadState::Interruptor {
public:
    explicit MessageLoopInterruptor(WebThread* thread) : m_thread(thread) { }

    void requestInterrupt() override
    {
        // GCTask has an empty run() method. Its only purpose is to guarantee
        // that MessageLoop will have a task to process which will result
        // in PendingGCRunner::didProcessTask being executed.
        m_thread->postTask(FROM_HERE, new GCTask);
    }

private:
    class GCTask : public WebThread::Task {
    public:
        virtual ~GCTask() { }

        void run() override
        {
            // Don't do anything here because we don't know if this is
            // a nested event loop or not. PendingGCRunner::didProcessTask
            // will enter correct safepoint for us.
            // We are not calling onInterrupted() because that always
            // conservatively enters safepoint with pointers on stack.
        }
    };

    WebThread* m_thread;
};

static double currentTimeFunction()
{
    return currentTimeImpl();
}

double g_firstMonotonicallyIncreasingTime = 0;

static double monotonicallyIncreasingTimeFunction()
{
    double timeInDouble = (double)currentTimeImpl();
    return timeInDouble - g_firstMonotonicallyIncreasingTime;
}

static double systemTraceTimeFunction()
{
    DebugBreak();
    return 0;
}

static void histogramEnumerationFunction(const char* name, int sample, int boundaryValue)
{
    //Platform::current()->histogramEnumeration(name, sample, boundaryValue);
}

static void cryptographicallyRandomValues(unsigned char* buffer, size_t length)
{
    //Platform::current()->cryptographicallyRandomValues(buffer, length);
    DebugBreak();
}

static void callOnMainThreadFunction(WTF::MainThreadFunction function, void* context)
{
    Platform::current()->mainThread()->postTask(FROM_HERE, new MainThreadTaskRunner(function, context));
}

static void adjustAmountOfExternalAllocatedMemory(int size)
{
    //v8::Isolate::GetCurrent()->AdjustAmountOfExternalAllocatedMemory(size);
}

static void initializeWithoutV8(Platform* platform);

void initialize()
{
    Platform* platform = Platform::current();
    initializeWithoutV8(platform);
    ThreadState::current()->addInterruptor(s_isolateInterruptor);

    // currentThread is null if we are running on a thread without a message loop.
    if (WebThread* currentThread = platform->currentThread()) {
        ASSERT(!s_endOfTaskRunner);
        s_endOfTaskRunner = new EndOfTaskRunner;
        currentThread->addTaskObserver(s_endOfTaskRunner);
    }
}

static void initializeWithoutV8(Platform* platform)
{
    ASSERT(!s_webKitInitialized);
    s_webKitInitialized = true;

    ASSERT(platform);

//    WTF::setRandomSource(cryptographicallyRandomValues);
    WTF::initialize(currentTimeFunction, monotonicallyIncreasingTimeFunction, systemTraceTimeFunction, histogramEnumerationFunction, adjustAmountOfExternalAllocatedMemory);
    WTF::initializeMainThread(callOnMainThreadFunction);
    //Platform::initialize(platform);
    Heap::init();

    ThreadState::attachMainThread();
    // currentThread() is null if we are running on a thread without a message loop.
    if (WebThread* currentThread = platform->currentThread()) {
        ASSERT(!s_pendingGCRunner);
        s_pendingGCRunner = new PendingGCRunner;
        currentThread->addTaskObserver(s_pendingGCRunner);

        ASSERT(!s_messageLoopInterruptor);
        s_messageLoopInterruptor = new MessageLoopInterruptor(currentThread);
        ThreadState::current()->addInterruptor(s_messageLoopInterruptor);
    }
}

}