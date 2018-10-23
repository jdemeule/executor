#ifndef STATIC_THREAD_POOL_HPP
#define STATIC_THREAD_POOL_HPP

#include "execution.hpp"

#include <algorithm>
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>

namespace execution {

class static_thread_pool {
public:
   using executor_type = executor<oneway_t>;

   class executor_impl;

   // construction/destruction
   explicit static_thread_pool(std::size_t num_threads) {
      //      for (std::size_t i = 0; i < num_threads; ++i) {
      //         m_threads.push_back(thread(worker(*this)));
      //      }
   }

   // nocopy
   static_thread_pool(const static_thread_pool&) = delete;
   static_thread_pool& operator=(const static_thread_pool&) = delete;

   // stop accepting incoming work and wait for work to drain
   ~static_thread_pool() {
      stop();
      wait();
   }

   // attach current thread to the thread pools list of worker threads
   // void attach();

   // signal all work to complete
   void stop() {
      //      shutdown(hard);
   }

   // wait for all threads in the thread pool to complete
   void wait() {
      //      join_all();
   }

   // placeholder for a general approach to getting executors from
   // standard contexts.
   executor_type executor() noexcept;

private:
   enum class shutdown_status { none, soft, hard };
};

class static_thread_pool::executor_impl {
   /*
    public:
    // types:

    typedef std::size_t shape_type;
    typedef std::size_t index_type;

    // construct / copy / destroy:

    C(const C& other) noexcept;
    C(C&& other) noexcept;

    C& operator=(const C& other) noexcept;
    C& operator=(C&& other) noexcept;

    // executor operations:

    see-below require(execution::blocking_t::never_t) const;
    see-below require(execution::blocking_t::possibly_t) const;
    see-below require(execution::blocking_t::always_t) const;
    see-below require(execution::relationship_t::continuation_t) const;
    see-below require(execution::relationship_t::fork_t) const;
    see-below require(execution::outstanding_work_t::tracked_t) const;
    see-below require(execution::outstanding_work_t::untracked_t) const;
    see-below require(const execution::allocator_t<void>& a) const;
    template<class ProtoAllocator>
    see-below require(const execution::allocator_t<ProtoAllocator>& a) const;

    static constexpr execution::bulk_guarantee_t
    query(execution::bulk_guarantee_t::parallel_t) const; static constexpr
    execution::mapping_t query(execution::mapping_t::thread_t) const;
    execution::blocking_t query(execution::blocking_t) const;
    execution::relationship_t query(execution::relationship_t) const;
    execution::outstanding_work_t query(execution::outstanding_work_t) const;
    see-below query(execution::context_t) const noexcept;
    see-below query(execution::allocator_t<void>) const noexcept;
    template<class ProtoAllocator>
    see-below query(execution::allocator_t<ProtoAllocator>) const noexcept;

    bool running_in_this_thread() const noexcept;
    */

   template <class Function>
   void execute(Function&& f) const;
};
}  // namespace execution

#endif
