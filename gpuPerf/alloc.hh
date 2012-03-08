#ifndef gpuPerf_alloc_hh
#define gpuPerf_alloc_hh

#include <iostream>

namespace cu
{
  // ----------------- for host mem ----------------
  template <class T> class allocator_host
  {
  public:
    typedef size_t    size_type;
    typedef ptrdiff_t difference_type;
    typedef T*        pointer;
    typedef const T*  const_pointer;
    typedef T&        reference;
    typedef const T&  const_reference;
    typedef T         value_type;
    template <class U> struct rebind { typedef allocator_host<U> other; };

    allocator_host() throw() { }
    allocator_host(const allocator_host&) throw() { }
    template <class U> allocator_host(const allocator_host<U>&) throw() { }
    ~allocator_host() throw() { }

    pointer address(reference x) const { return &x; }
    const_pointer address(const_reference x) const { return &x; }

    pointer allocate(size_type n,
		     std::allocator<void>::const_pointer hint = 0)
    {
      void* ptr;
      cudaError_t err = cudaMallocHost(&ptr,n * sizeof(T));
      return err==cudaSuccess?(pointer)ptr : 0;
    }

    void deallocate(pointer p, size_type n) { cudaFreeHost(p); }
    size_type max_size() const throw() { return 1<<29; }
    void construct(pointer p, const T& val) { new (p) T(val);  }
    void destroy(pointer p) { p->~T(); }
  private:
  };

  template <class T1, class T2>
  inline bool operator==(const allocator_host<T1>&, const allocator_host<T2>&) throw()
  { return false; }

  template <class T1, class T2>
  inline bool operator!=(const allocator_host<T1>&, const allocator_host<T2>&) throw()
  { return true; }

}


#endif /* gpuPerf_alloc_hh */
