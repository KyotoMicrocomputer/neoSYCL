#ifndef SYCL_INCLUDE_CL_SYCL_BUFFER_DATA_CONTAINER_ND_HPP_
#define SYCL_INCLUDE_CL_SYCL_BUFFER_DATA_CONTAINER_ND_HPP_

#include <shared_mutex>
#include <cstring>
#include "neoSYCL/sycl/detail/container/array_nd.hpp"

namespace neosycl::sycl::detail::container {

/**
 * Data container for SYCL buffer
 *
 * @tparam T DataType
 * @tparam dimensions Buffer dimensions
 * @tparam AllocatorT Buffer Allocator
 */
template <typename T, int dimensions, typename AllocatorT>
class DataContainerD : public DataContainer {
public:
  explicit DataContainerD(ArrayND<dimensions> r) : range(r) {
    allocate_ptr = shared_ptr_class<T>(alloc.allocate(r.get_liner()));
    ptr          = allocate_ptr.get();
  }

  DataContainerD(ArrayND<dimensions> r, AllocatorT allocatorT)
      : alloc(allocatorT), range(r) {
    allocate_ptr = shared_ptr_class<T>(alloc.allocate(r.get_liner()));
    ptr          = allocate_ptr.get();
  }

  DataContainerD(T* data, ArrayND<dimensions> r)
      : range(r), ptr(data), allocate_ptr(nullptr) {}

  DataContainerD(T* data, ArrayND<dimensions> r, AllocatorT allocatorT)
      : alloc(allocatorT), range(r), ptr(data), allocate_ptr(nullptr) {}

  size_t get_size() override {
    return sizeof(T) * range.get_liner();
  }

  size_t get_count() override {
    return range.get_liner();
  }

  T* get_ptr() const {
    return ptr;
  }

  void* get_raw_ptr() override {
    return (void*)get_ptr();
  }

  T* begin() const {
    return ptr;
  }

  T* end() const {
    return ptr + range.get_liner();
  }

  T& get(size_t x) const {
    return ptr[x];
  }

  AllocatorT get_allocator() {
    return alloc;
  }

  ArrayND<dimensions> get_range() const {
    return range;
  }

  DataContainerD(const DataContainerD& rhs)
      : alloc(rhs.alloc), range(rhs.range) {
    allocate_ptr = shared_ptr_class<T>(alloc.allocate(range.get_liner()));
    ptr          = allocate_ptr.get();
    std::memcpy(ptr, rhs.ptr, sizeof(T) * range.get_liner());
  }

  DataContainerD(DataContainerD&& rhs)
      : alloc(rhs.alloc), range(rhs.range), ptr(rhs.ptr),
        allocate_ptr(rhs.allocate_ptr) {}

  DataContainerD& operator=(const DataContainerD& rhs) {
    range        = rhs.range;
    alloc        = rhs.alloc;
    ptr          = rhs.ptr;
    allocate_ptr = rhs.allocate_ptr;
  }

  DataContainerD& operator=(DataContainerD&& rhs) {
    range        = rhs.range;
    alloc        = rhs.alloc;
    ptr          = rhs.ptr;
    allocate_ptr = rhs.allocate_ptr;
  }

  void set_final_data(std::nullptr_t) {
    ptr = nullptr;
  }

  void set_final_data(T* FinalData) {
    ptr = FinalData;
  }

  void set_final_data(std::weak_ptr<T> FinalData) {
    if (std::shared_ptr<T> p = FinalData.lock())
      ptr = p.get();
  }

private:
  AllocatorT alloc;
  ArrayND<dimensions> range;
  T* ptr;
  shared_ptr_class<T> allocate_ptr;
};

template <typename T, int dimensions, typename AllocatorT = buffer_allocator<T>>
class DataContainerND {};

template <typename T, typename AllocatorT>
class DataContainerND<T, 1, AllocatorT>
    : public DataContainerD<T, 1, AllocatorT> {
public:
  DataContainerND(const ArrayND<1>& r) : DataContainerD<T, 1, AllocatorT>(r) {}

  DataContainerND(const ArrayND<1>& r, AllocatorT alloc)
      : DataContainerD<T, 1, AllocatorT>(r, alloc) {}

  DataContainerND(T* data, const ArrayND<1>& r)
      : DataContainerD<T, 1, AllocatorT>(data, r) {}

  DataContainerND(T* data, const ArrayND<1>& r, AllocatorT alloc)
      : DataContainerD<T, 1, AllocatorT>(data, r, alloc) {}

  DataContainerND(const DataContainerD<T, 1, AllocatorT>& rhs)
      : DataContainerD<T, 1, AllocatorT>(rhs) {}

  DataContainerND(DataContainerD<T, 1, AllocatorT>&& rhs)
      : DataContainerD<T, 1, AllocatorT>(rhs) {}

  T& operator[](size_t x) const {
    return this->get_ptr()[x];
  }
};

template <typename T, typename AllocatorT>
class DataContainerND<T, 2, AllocatorT>
    : public DataContainerD<T, 2, AllocatorT> {
public:
  DataContainerND(const ArrayND<2>& r) : DataContainerD<T, 2, AllocatorT>(r) {}

  DataContainerND(const ArrayND<2>& r, AllocatorT alloc)
      : DataContainerD<T, 2, AllocatorT>(r, alloc) {}

  DataContainerND(T* data, const ArrayND<2>& r)
      : DataContainerD<T, 2, AllocatorT>(data, r) {}

  DataContainerND(T* data, const ArrayND<2>& r, AllocatorT alloc)
      : DataContainerD<T, 2, AllocatorT>(data, r, alloc) {}

  DataContainerND(const DataContainerD<T, 2, AllocatorT>& rhs)
      : DataContainerD<T, 2, AllocatorT>(rhs) {}

  DataContainerND(DataContainerD<T, 2, AllocatorT>&& rhs)
      : DataContainerD<T, 2, AllocatorT>(rhs) {}

  T* operator[](size_t i) const {
    size_t y = this->get_range()[1];
    return this->get_ptr() + (y * i);
  }
};

/**
 * Container access proxy, performs as 2-D array
 * Only use in 3-D buffer
 *
 * @tparam T DataType
 * @tparam dimensions Buffer dimensions
 */
template <typename T, int dimensions>
struct AccessProxyND {};

template <typename T>
struct AccessProxyND<T, 3> {
  AccessProxyND(const ArrayND<3>& r, T* ptr) : range(r), base_ptr(ptr) {}

  T* operator[](size_t i) const {
    size_t z = range[2];
    return base_ptr + i * z;
  }

  ArrayND<3> range;
  T* base_ptr;
};

template <typename T, typename AllocatorT>
class DataContainerND<T, 3, AllocatorT>
    : public DataContainerD<T, 3, AllocatorT> {
public:
  DataContainerND(const ArrayND<3>& r) : DataContainerD<T, 3, AllocatorT>(r) {}

  DataContainerND(const ArrayND<3>& r, AllocatorT alloc)
      : DataContainerD<T, 3, AllocatorT>(r, alloc) {}

  DataContainerND(T* data, const ArrayND<3>& r)
      : DataContainerD<T, 3, AllocatorT>(data, r) {}

  DataContainerND(T* data, const ArrayND<3>& r, AllocatorT alloc)
      : DataContainerD<T, 3, AllocatorT>(data, r, alloc) {}

  DataContainerND(const DataContainerD<T, 3, AllocatorT>& rhs)
      : DataContainerD<T, 3, AllocatorT>(rhs) {}

  DataContainerND(DataContainerD<T, 3, AllocatorT>&& rhs)
      : DataContainerD<T, 3, AllocatorT>(rhs) {}

  AccessProxyND<T, 3> operator[](size_t i) const {
    size_t y    = this->get_range()[1];
    size_t z    = this->get_range()[2];
    T* base_ptr = this->get_ptr() + i * y * z;
    return AccessProxyND<T, 3>(this->get_range(), base_ptr);
  }
};

} // namespace neosycl::sycl::detail::container

#endif // SYCL_INCLUDE_CL_SYCL_BUFFER_DATA_CONTAINER_ND_HPP_
