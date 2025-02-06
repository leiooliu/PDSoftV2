#ifndef AUTOEXECUTOR_H
#define AUTOEXECUTOR_H

#include <QObject>
#include <QTimer>
#include <functional>

class AutoExecutor : public QObject
{
    Q_OBJECT
public:
    explicit AutoExecutor(QObject *parent = nullptr);
    ~AutoExecutor();

    void setInterval(int interval);
    void start();
    void stop();
    void setFunction(const std::function<void()> &func);

    bool isRunning() const;

private slots:
    void onTimeout();

private:
    QTimer *m_timer;
    std::function<void()> m_function;

};

#endif // AUTOEXECUTOR_H
