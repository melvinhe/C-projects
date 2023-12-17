#include "synchronized_queue.hpp"

template <typename T>
size_t synchronized_queue<T>::size() {
  // TODO (Part A, Step 3): IMPLEMENT
  std::unique_lock<std::mutex> lock(mtx);
  return this->q.size();
}

template <typename T>
bool synchronized_queue<T>::pop(T* elt) {
  // TODO (Part A, Step 3): IMPLEMENT
  std::unique_lock<std::mutex> lock(mtx);
  // wait if empty with condition variable
  cv.wait(lock, [this]() { return !q.empty() || is_stopped; });
  if (is_stopped) {
    return true;
  } else {
    *elt = std::move(q.front());
    q.pop();
    return false;
  }
}

template <typename T>
void synchronized_queue<T>::push(T elt) {
  // TODO (Part A, Step 3): IMPLEMENT
  std::unique_lock<std::mutex> lock(mtx);
  this->q.push(elt);
  this->cv.notify_all();
}

template <typename T>
std::vector<T> synchronized_queue<T>::flush() {
  // TODO (Part A, Step 3): IMPLEMENT
  std::unique_lock<std::mutex> lock(mtx);
  std::vector<T> vec;
  while (!this->q.empty()) {
    vec.push_back(this->q.front());
    this->q.pop();
  }
  return vec;
}

template <typename T>
void synchronized_queue<T>::stop() {
  // TODO (Part A, Step 3): IMPLEMENT
  std::unique_lock<std::mutex> lock(mtx);
  is_stopped = true;
  this->cv.notify_all();
}

// NOTE: DO NOT TOUCH! Why is this necessary? Because C++ is a beautiful
// language:
// https://isocpp.org/wiki/faq/templates#separate-template-fn-defn-from-decl
template class synchronized_queue<int>;
template class synchronized_queue<std::shared_ptr<ClientConn>>;
