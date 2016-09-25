

#ifndef Platform_h
#define Platform_h

namespace blink {

class WebThread;

class Platform {
public:
    void shutdown();
    static Platform* current();

    WebThread* currentThread();
    WebThread* mainThread() const;

private:
    Platform();

    static Platform* m_inst;
    WebThread* m_mainThread;
    WebThread* m_thread;
};

}

#endif Platform_h