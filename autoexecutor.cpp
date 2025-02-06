#include "autoexecutor.h"

AutoExecutor::AutoExecutor(QObject *parent)
    : QObject(parent), m_timer(new QTimer(this)), m_function(nullptr)
{
    // 连接QTimer的timeout信号到类的内部槽函数
    connect(m_timer, &QTimer::timeout, this, &AutoExecutor::onTimeout);
}

AutoExecutor::~AutoExecutor()
{
}

void AutoExecutor::setInterval(int interval)
{
    if (m_timer)
        m_timer->setInterval(interval);
}

void AutoExecutor::start()
{
    if (m_timer && m_function)
        m_timer->start();
}

void AutoExecutor::stop()
{
    if (m_timer)
        m_timer->stop();
}

void AutoExecutor::setFunction(const std::function<void()> &func)
{
    m_function = func;
}

bool AutoExecutor::isRunning() const
{
    return m_timer && m_timer->isActive();
}

void AutoExecutor::onTimeout()
{
    if (m_function)
        m_function(); // 调用用户指定的函数
}
