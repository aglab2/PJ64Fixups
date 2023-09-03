#pragma once

#include <vector>

template<typename T, size_t Capacity>
class CircularBuffer
{
private:
    std::vector<T> buffer_;
    size_t head_ = 0;
    size_t tail_ = 0;

public:
    CircularBuffer()
    {
        buffer_.resize(Capacity);
    }

    void push(T value)
    {
        buffer_[head_] = value;
        head_ = (head_ + 1) % Capacity;
        if (head_ == tail_)
        {
            tail_ = (tail_ + 1) % Capacity;
        }
    }

    class iterator
    {
    private:
        T* buf_;
        size_t tail_;
        size_t index_;

    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::random_access_iterator_tag;

        iterator() = default;
        iterator(CircularBuffer& buf, size_t idx) : buf_(buf.buffer_.data()), tail_(buf.tail_), index_(idx % Capacity) {}
        iterator(T* buf, size_t tail, size_t idx) : buf_(buf)               , tail_(tail)     , index_(idx % Capacity) {}

        T& operator*() const
        {
            return buf_[index_];
        }

        T* operator->() const
        {
            return &buf_[index_];
        }

        T operator[](difference_type n) const
        {
            return buf_[(index_ + n) % Capacity];
        }

        iterator& operator++()
        {
            index_ = (index_ + 1) % Capacity;
            return *this;
        }

        iterator& operator--()
        {
            index_ = (index_ - 1 + Capacity) % Capacity;
            return *this;
        }

        iterator& operator++(int amt)
        {
            index_ = (index_ + amt) % Capacity;
            return *this;
        }

        iterator& operator--(int amt)
        {
            index_ = (index_ - amt + Capacity) % Capacity;
            return *this;
        }

        iterator& operator+=(difference_type amt)
        {
            index_ = (index_ + amt) % Capacity;
            return *this;
        }
        iterator& operator-=(difference_type amt)
        {
            index_ = (index_ - amt + Capacity) % Capacity;
            return *this;
        }

        iterator operator-(int amt) const
        {
            size_t index = (index_ - amt + Capacity) % Capacity;
            return iterator{ buf_, tail_, index };
        }

        iterator operator+(int amt) const
        {
            size_t index = (index_ + amt) % Capacity;
            return iterator{ buf_, tail_, index };
        }

        bool operator!=(const iterator& other) const
        {
            return index_ != other.index_;
        }

        bool operator==(const iterator& other) const
        {
            return index_ == other.index_;
        }

        friend iterator operator+(difference_type n, const iterator& it)
        {
            return it + n;
        }

        friend iterator operator-(difference_type n, const iterator& it)
        {
            return it - n;
        }

        difference_type operator-(const iterator& rhs) const
        {
            int realIdx0 = index_ >= tail_ ? index_ - tail_ : Capacity - tail_ + index_;
            int realIdx1 = rhs.index_ >= rhs.tail_ ? rhs.index_ - rhs.tail_ : Capacity - rhs.tail_ + rhs.index_;
            return realIdx0 - realIdx1;
        }

    };

    iterator begin()
    {
        return iterator(*this, tail_);
    }

    iterator end()
    {
        return iterator(*this, head_);
    }

    bool empty() const
    {
        return head_ == tail_;
    }

    bool full() const
    {
        return tail_ == (head_ + 1) % Capacity;
    }

    int head() const
    {
        return head_;
    }

    const std::vector<T>& buffer() const
    {
        return buffer_;
    }
};
