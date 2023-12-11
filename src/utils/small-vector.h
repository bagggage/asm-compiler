#ifndef __ASM_SMALL_VECTOR_H
#define __ASM_SMALL_VECTOR_H

#include <array>
#include <initializer_list>
#include <cstddef>
#include <stdexcept>

namespace ASM
{
    template <typename T, uint8_t N>
    class SmallVector
    {
    public:
      using iterator       = typename std::array<T,N>::iterator;
      using const_iterator = typename std::array<T,N>::const_iterator;

      SmallVector(uint8_t n = 0) : _size(n) 
      {
        if(_size > N)
            throw std::overflow_error("SmallVector overflow");
      }

      SmallVector(const SmallVector& other) = default;
      SmallVector(SmallVector&& other) = default;

      constexpr SmallVector(const std::initializer_list<T> init)
      {
        _size = init.size();

        for (unsigned int i = 0; i < _size; i++)
            storage[i] = *(init.begin() + i);
      }

      void push_back(T val)
      {
        storage[_size++] = val;

        if (_size > N)
            throw std::overflow_error("SmallVector overflow");
      }

      void pop_back()
      {
        if (_size == 0)
            throw std::underflow_error("SmallVector underflow");

        back().~T();
        _size--;
      }

      inline size_t size() const { return _size; }

      void clear() { while(_size > 0) { pop_back(); } }

      inline T& front() { return storage.front(); }
      inline const T& front() const { return storage.front(); }

      inline T& back() { return storage[_size - 1]; }
      inline const T& back() const { return storage[_size-1]; }

      inline iterator begin() { return storage.begin(); }
      inline const_iterator begin() const { return storage.begin(); }

      inline iterator end() { return storage.end(); }
      inline const_iterator end() const { return storage.end(); }

      inline T& operator[](uint8_t index) { return storage[index]; }
      inline const T& operator[](uint8_t index) const { return storage[index]; }

      inline T* data() { return storage.data(); }
      inline const T* data() const { return storage.data(); }

    private:
      std::array<T,N> storage;

      uint8_t _size = 0;
    };
}

#endif