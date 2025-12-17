/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** SafeQueue.hpp - Thread-safe queue template
*/

#ifndef SAFE_QUEUE_HPP_
#define SAFE_QUEUE_HPP_

#include <condition_variable>
#include <deque>
#include <mutex>

/**
 * @brief Thread-safe queue template class
 *
 * @tparam T Type of elements stored in the queue
 */
template <typename T>
class SafeQueue
{
public:
  /**
   * @brief Push an element to the queue
   *
   * @param value The value to push
   */
  void push(const T &value)
  {
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_queue.push_back(value);
    }
    m_conditionVariable.notify_one();
  }

  /**
   * @brief Try to pop an element from the queue
   *
   * @param value Variable to store the popped value
   * @return true if an element was popped, false if queue is empty
   */
  bool pop(T &value)
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_queue.empty())
      return false;
    value = std::move(m_queue.front());
    m_queue.pop_front();
    return true;
  }

  /**
   * @brief Pop an element, waiting if necessary
   *
   * @return The popped element
   */
  T popWait()
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_conditionVariable.wait(lock, [this] { return !m_queue.empty(); });
    T value = std::move(m_queue.front());
    m_queue.pop_front();
    return value;
  }

private:
  std::deque<T> m_queue;
  std::mutex m_mutex;
  std::condition_variable m_conditionVariable;
};

#endif // SAFE_QUEUE_HPP_
