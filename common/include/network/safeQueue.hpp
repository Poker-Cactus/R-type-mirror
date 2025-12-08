/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** safeQueue.hpp
*/

#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>

template <typename T> class SafeQueue
{
  public:
    void push(const T &value)
    {
        {
            std::lock_guard<std::mutex> lock(mtx);
            q.push_back(value);
        }
        cv.notify_one();
    }

    bool pop(T &value)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (q.empty())
            return false;
        value = std::move(q.front());
        q.pop_front();
        return true;
    }

    T pop_wait()
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !q.empty(); });
        T val = std::move(q.front());
        q.pop_front();
        return val;
    }

  private:
    std::deque<T> q;
    std::mutex mtx;
    std::condition_variable cv;
};
