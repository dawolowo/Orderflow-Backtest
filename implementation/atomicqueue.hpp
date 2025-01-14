#pragma once
#include <queue>
#include <mutex>

/*Queue like container which is thread safe
@tparam Type Type of element.
@tparam Sequence  Type of underlying sequence, defaults to deque<Type>.
*/
template <typename Type, typename Sequence = std::deque<Type>>
class AtomicQueue {

private:
    std::queue<Type, Sequence> _data;
    std::mutex _mut_lock;
    
public:
    AtomicQueue() = default;

    ~AtomicQueue(){_mut_lock.unlock();}

    //Returns the number of elements in the %AtomicQueue.
    size_t size() noexcept{
        std::lock_guard<std::mutex> lock(_mut_lock);
        return _data.size();
    }

    //Returns true if the %AtomicQueue is empty.
    bool empty() noexcept {
        std::lock_guard<std::mutex> lock(_mut_lock);
        return _data.empty();
    }

    /*Add data to the end of the %queue.
    @param __x Data to be added.
    
    This is a typical %AtomicQueue operation. The function creates an element at the end of the %queue and assigns the given data to it.
    The time complexity of the operation depends on the underlying sequence.*/
    void push(Type &__x) noexcept{
        std::lock_guard<std::mutex> lock(_mut_lock);
        _data.push(__x);
    }

    /*Add data to the end of the %queue.
    @param __x  Data to be added.

    This is a typical %AtomicQueue operation. The function creates an element at the end of the %queue and assigns the given data to it.
    The time complexity of the operation depends on the underlying sequence.*/
    void push(Type &&__x)noexcept{
        std::lock_guard<std::mutex> lock(_mut_lock);
        _data.push(std::move(__x));
    }

    /*Removes first element.

    This is a typical %AtomicQueue operation. It shrinks the %AtomicQueue by one. The time complexity of the operation depends on the
     underlying sequence.

    @note No data is returned, and if the first element's data is needed, it should be retrieved before pop() is called.*/
    void pop(){
        std::lock_guard<std::mutex> lock(_mut_lock);
        _data.pop();
    }

    /*Returns a read/write reference to the data at the first element of the %AtomicQueue.*/
    Type &front(){
        std::lock_guard<std::mutex> lock(_mut_lock);
        return _data.front();
    }
};
