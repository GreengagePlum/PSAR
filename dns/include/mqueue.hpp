#pragma once

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

#include "common.hpp"
#include "message.hpp"

namespace microdns
{

class MessageQueue
{
    using QueryEntry = std::pair<QueryID, std::shared_ptr<dns::Message>>;

    mutable std::mutex mtx;
    mutable std::condition_variable writeReady;
    mutable std::condition_variable readReady;

    std::queue<QueryEntry> queue;
    const int limit;

    int pipefd[2]; // For `poll()` integration, notification mechanism
    char notification = 0;

  public:
    explicit MessageQueue(int limit = -1);

    ~MessageQueue();

    QueryEntry pop();

    void push(QueryEntry queryEntry);

    std::optional<QueryEntry> try_pop(const std::chrono::milliseconds &timeout);

    int getReadfd() const;
};

} // namespace microdns
