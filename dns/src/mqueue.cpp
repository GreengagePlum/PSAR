#include <optional>
#include <stdio.h>
#include <unistd.h>

#include <mutex>

#include "mqueue.hpp"

namespace microdns
{

MessageQueue::MessageQueue(int limit) : limit(limit)
{
    if (pipe(pipefd) == -1)
    {
        const char *msg = "MessageQueue couldn't create pipe";
        perror(msg);
        throw msg;
    }
}

MessageQueue::~MessageQueue()
{
    if (close(pipefd[1]) == -1)
        perror("MessageQueue pipefd[1] close failed");
    if (close(pipefd[0]) == -1)
        perror("MessageQueue pipefd[1] close failed");
}

int MessageQueue::getReadfd() const
{
    return pipefd[0];
}

MessageQueue::QueryEntry MessageQueue::pop()
{
    std::unique_lock<std::mutex> lk(mtx);
    readReady.wait(lk, [&]() -> bool { return queue.size() > 0; });
    auto queryEntry = queue.front();
    queue.pop();
    lk.unlock();

    // TODO: error check this read!
    read(pipefd[0], &notification, sizeof(notification));
    if (limit >= 0)
        writeReady.notify_one();
    return queryEntry;
}

void MessageQueue::push(MessageQueue::QueryEntry queryEntry)
{
    std::unique_lock<std::mutex> lk(mtx);
    if (limit >= 0)
        writeReady.wait(lk, [&]() -> bool { return queue.size() < (unsigned long)limit; });
    queue.push(queryEntry);
    lk.unlock();

    // TODO: error check this write!
    write(pipefd[1], &notification, sizeof(notification));
    readReady.notify_one(); // TODO: optimize away unnecessary notifications (only if someone is actually waiting)
}

std::optional<MessageQueue::QueryEntry> MessageQueue::try_pop(const std::chrono::milliseconds &timeout)
{
    std::unique_lock<std::mutex> lk(mtx);
    if (!readReady.wait_for(lk, timeout, [&]() -> bool { return queue.size() > 0; }))
    {
        return std::nullopt;
    }
    auto queryEntry = queue.front();
    queue.pop();
    lk.unlock();

    // TODO: error check this read!
    read(pipefd[0], &notification, sizeof(notification));
    if (limit >= 0)
        writeReady.notify_one(); // TODO: optimize away unnecessary notifications (only if someone is actually waiting)
    return queryEntry;
}

} // namespace microdns
