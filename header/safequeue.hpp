#pragma once
#include <queue>
#include <mutex>

/*Queue like container which is thread safe
@tparam Type Type of element.
@tparam Sequence  Type of underlying sequence, defaults to deque<Type>.
*/
template <typename Type, typename Sequence = std::deque<Type>>
class SafeQueue {

private:
    std::queue<Type, Sequence> _data;
    std::mutex _mut_lock;
    volatile bool _is_empty = true;
    volatile size_t _len = 0;

public:
    SafeQueue() = default;

    ~SafeQueue(){_mut_lock.unlock();}

    //Returns the number of elements in the %SafeQueue.
    size_t size() noexcept{
        return _len;
    }

    //Returns true if the %SafeQueue is empty.
    bool empty() noexcept {
        return _is_empty;
    }

    /*Add data to the end of the %queue.
    @param __x Data to be added.
    
    This is a typical %SafeQueue operation. The function creates an element at the end of the %queue and assigns the given data to it.
    The time complexity of the operation depends on the underlying sequence.*/
    void push(Type &__x) noexcept{
        std::lock_guard<std::mutex> lock(_mut_lock);
        _data.push(__x);
        _len += 1;
        _is_empty = false;
    }

    /*Add data to the end of the %queue.
    @param __x  Data to be added.

    This is a typical %SafeQueue operation. The function creates an element at the end of the %queue and assigns the given data to it.
    The time complexity of the operation depends on the underlying sequence.*/
    void push(Type &&__x)noexcept{
        std::lock_guard<std::mutex> lock(_mut_lock);
        _data.push(std::move(__x));
        _len += 1;
        _is_empty = false;
    }

    /*Removes first element.
    This is a typical %SafeQueue operation. It shrinks the %SafeQueue by one. The time complexity of the operation depends on the
    underlying sequence.

    @note No data is returned, and if the first element's data is needed, it should be retrieved before pop() is called.*/
    void pop(){
        std::lock_guard<std::mutex> lock(_mut_lock);
        _data.pop();
        _len -= 1;
        _is_empty = _data.empty();
    }

    /*Returns a read/write reference to the data at the first element of the %SafeQueue.*/
    Type &front(){
        std::lock_guard<std::mutex> lock(_mut_lock);
        return _data.front();
    }
};
