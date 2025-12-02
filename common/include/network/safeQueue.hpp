/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** safeQueue.hpp
*/

#include <deque>
#include <mutex>
#pragma once

template <typename T> class SafeQueue
{
  public:
    void push(const T &value)
    {
        std::lock_guard<std::mutex> lock(mtx);
        q.push_back(value);
    }

    bool pop(T &value)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (q.empty())
            return false;
        value = q.front();
        q.pop_front();
        return true;
    }

  private:
    std::deque<T> q;
    std::mutex mtx;
};
