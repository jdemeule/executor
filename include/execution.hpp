#ifndef EXECUTION_HPP
#define EXECUTION_HPP

/*
 * execution
 *
 * Very partial implementation of p0443, executor, in its revision 9.
 * http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0443r9.html
 *
 * The proposal target oneway executor (fire and forget model) and define
 * several api to manipulate them:
 *   - queries api with require/prefer/query and customisation points,
 *   - default properties to queries,
 *   - allocator customization,
 *   - oneway model and a typed erased oneway polymorphic helper.
 *
 * For now, we target only the oneway (non bulk) model with the typed erased
 * helper but without any other properties nor customization point.
 */


#include <cstddef>
#include <memory>
#include <type_traits>
#include <typeinfo>

namespace execution {

// Customization points:

// namespace {
// constexpr unspecified require = unspecified;
// constexpr unspecified prefer  = unspecified;
// constexpr unspecified query   = unspecified;
//}  // namespace

// Customization point type traits:

// template <class Executor, class... Properties>
// struct can_require;
// template <class Executor, class... Properties>
// struct can_prefer;
// template <class Executor, class Property>
// struct can_query;
//
// template <class Executor, class... Properties>
// constexpr bool can_require_v = can_require<Executor, Properties...>::value;
// template <class Executor, class... Properties>
// constexpr bool can_prefer_v = can_prefer<Executor, Properties...>::value;
// template <class Executor, class Property>
// constexpr bool can_query_v = can_query<Executor, Property>::value;

// Associated execution context property:

struct context_t;

// constexpr context_t context;

// Interface-changing properties:

struct oneway_t;
// struct bulk_oneway_t;

// constexpr oneway_t oneway;
// constexpr bulk_oneway_t bulk_oneway;

// Blocking properties:

// struct blocking_t;

// constexpr blocking_t blocking;

// Properties to allow adaptation of blocking and directionality:

// struct blocking_adaptation_t;

// constexpr blocking_adaptation_t blocking_adaptation;

// Properties to indicate if submitted tasks represent continuations:

// struct relationship_t;

// constexpr relationship_t relationship;

// Properties to indicate likely task submission in the future:

// struct outstanding_work_t;

// constexpr outstanding_work_t outstanding_work;

// Properties for bulk execution guarantees:

// struct bulk_guarantee_t;

// constexpr bulk_guarantee_t bulk_guarantee;

// Properties for mapping of execution on to threads:

// struct mapping_t;

// constexpr mapping_t mapping;

// Memory allocation properties:

// template <typename ProtoAllocator>
// struct allocator_t;

// constexpr allocator_t<void> allocator;

// Executor type traits:

// template <class Executor, bool>
// struct is_oneway_executor;
// template <class Executor>
// struct is_bulk_oneway_executor;

// template <class Executor>
// constexpr bool is_oneway_executor_v = is_oneway_executor<Executor>::value;
// template <class Executor>
// constexpr bool is_bulk_oneway_executor_v =
// is_bulk_oneway_executor<Executor>::value;

// template <class Executor>
// struct executor_shape;
// template <class Executor>
// struct executor_index;
//
// template <class Executor>
// using executor_shape_t = typename executor_shape<Executor>::type;
// template <class Executor>
// using executor_index_t = typename executor_index<Executor>::type;

// Polymorphic executor support:

class bad_executor;

template <class InterfaceProperty, class... SupportableProperties>
using executor = typename InterfaceProperty::template polymorphic_executor_type<
   InterfaceProperty,
   SupportableProperties...>;

// template <class Property>
// struct prefer_only;



/////////////////////////////
// Implementation
/////////////////////////////

// Type traits for C++11 compilers
template <typename E, typename Fn>
struct is_oneway_execute_impl_type {
   template <typename F>
   static auto f(int)
      -> decltype(std::declval<const E&>().execute(std::declval<F>()));

   template <typename F>
   static auto f(...) -> std::false_type;

   using type = decltype(f<Fn>(0));
};

template <typename E, class F>
struct is_oneway_execute_impl {
   static const bool value =
      std::is_same<void,
                   typename is_oneway_execute_impl_type<E, F>::type>::value;
};

template <class E>
struct is_oneway_executor_impl {
   struct nullary_fct {
      void operator()() {}
   };

   static const bool is_executable =
      is_oneway_execute_impl<E, nullary_fct>::value;
   static const bool is_executable_r =
      is_oneway_execute_impl<E, nullary_fct&>::value;
   static const bool is_executable_cr =
      is_oneway_execute_impl<E, const nullary_fct&>::value;
   static const bool is_executable_rr =
      is_oneway_execute_impl<E, nullary_fct&&>::value;

   static const bool value =
      is_executable && is_executable_r && is_executable_cr && is_executable_rr;
};


template <class Executor, bool = is_oneway_executor_impl<Executor>::value>
struct is_oneway_executor : std::false_type {};

template <class Executor>
struct is_oneway_executor<Executor, true> : std::true_type {};

struct require_fn {
   template <class E, class P0>
   constexpr auto operator()(E&& e, P0&&) const
      -> std::enable_if_t<std::decay_t<P0>::is_requirable and
                             (std::decay_t<P0>::template static_query_v<
                                 std::decay_t<E>> == std::decay_t<P0>::value()),
                          E> {
      return std::forward<E>(e);
   }
};

constexpr require_fn require = {};


struct context_t {
   static constexpr bool is_requirable = false;
   static constexpr bool is_preferable = false;

   // using polymorphic_query_result_type = any;

   template <class Executor>
   static constexpr decltype(auto) static_query_v =
      Executor::query(*static_cast<context_t*>(nullptr));
};

struct oneway_t {
   static constexpr bool is_requirable = true;
   static constexpr bool is_preferable = false;

   using polymorphic_query_result_type = bool;

   template <class... SupportableProperties>
   class polymorphic_executor_type;

   template <class Executor>
   static constexpr bool static_query_v = is_oneway_executor<Executor>::value;

   static constexpr bool value() {
      return true;
   }
};

constexpr oneway_t oneway = {};


template <class... SupportableProperties>
class oneway_t::polymorphic_executor_type {
public:
   // construct / copy / destroy:

   polymorphic_executor_type() noexcept = default;

   polymorphic_executor_type(std::nullptr_t) noexcept
      : polymorphic_executor_type() {}

   polymorphic_executor_type(const polymorphic_executor_type& e) noexcept
      : m_vptr(e.m_vptr) {
      m_vptr->copy(&m_buffer, &e.m_buffer);
   }

   polymorphic_executor_type(polymorphic_executor_type&& e) noexcept
      : m_vptr(e.m_vptr) {
      m_vptr->move_ctor(&m_buffer, &e.m_buffer);
   }

   template <class... OtherSupportableProperties>
   polymorphic_executor_type(
      polymorphic_executor_type<OtherSupportableProperties...> e);

   //   template <class... OtherSupportableProperties>
   //   polymorphic_executor_type(polymorphic_executor_type<OtherSupportableProperties...>
   //   e) = delete;

   polymorphic_executor_type& operator=(
      const polymorphic_executor_type& e) noexcept {
      polymorphic_executor_type(e).swap(*this);
      return *this;
   }

   polymorphic_executor_type& operator=(
      polymorphic_executor_type&& e) noexcept {
      m_vptr->dtor(&m_buffer);
      m_vptr = e.m_vptr;
      m_vptr->move_ctor(&m_buffer, &e.m_buffer);
      return *this;
   }

   polymorphic_executor_type& operator=(std::nullptr_t) noexcept {
      polymorphic_executor_type().swap(*this);
      return *this;
   }

   ~polymorphic_executor_type() {
      m_vptr->dtor(&m_buffer);
   }

   // polymorphic_executor_type modifiers:

   void swap(polymorphic_executor_type& other) noexcept {
      std::swap(*this, other);
   }

   // polymorphic_executor_type operations:

   template <class Property>
   polymorphic_executor_type require(Property) const;

   template <class Property>
   typename Property::polymorphic_query_result_type query(Property) const;

   // polymorphic_executor_type capacity:

   explicit operator bool() const noexcept {
      return m_vptr != &empty;
   }

   // polymorphic_executor_type target access:

   const std::type_info& target_type() const noexcept {
      return m_vptr->target_type(&m_buffer);
   }

   template <class Executor>
   Executor* target() noexcept {
      return target_type() == typeid(Executor)
                ? static_cast<Executor*>(static_cast<void*>(&m_buffer))
                : nullptr;
   }

   template <class Executor>
   const Executor* target() const noexcept {
      return target_type() == typeid(Executor)
                ? static_cast<Executor*>(static_cast<void*>(&m_buffer))
                : nullptr;
   }

   // polymorphic_executor_type casts:

   template <class... OtherSupportableProperties>
   polymorphic_executor_type<OtherSupportableProperties...>
   static_executor_cast() const;



   template <class Executor,
             class = std::enable_if_t<
                !std::is_same<polymorphic_executor_type, Executor>::value>>
   polymorphic_executor_type(Executor e)
      : m_vptr(vtable_for<Executor>()) {
      new (&m_buffer) Executor(std::move(e));
   }

   template <class Executor>
   polymorphic_executor_type& operator=(Executor e) {
      polymorphic_executor_type(std::move(e)).swap(*this);
      return *this;
   }

   template <class Function>
   void execute(Function&& f) const {
      // std::function<void()> should be fine too.
      // However, this could limit the kind of function we are able to received.
      std::unique_ptr<func_base> fct =
         std::make_unique<func_impl<Function>>(std::forward<Function>(f));
      m_vptr->execute(&m_buffer, std::move(fct));
   }

   friend bool operator==(const polymorphic_executor_type& a,
                          const polymorphic_executor_type& b) noexcept {
      if (!a && !b)
         return true;
      if ((a.m_vptr) == (b.m_vptr))
         return true;
      return false;
   }

   friend bool operator==(const polymorphic_executor_type& e,
                          std::nullptr_t) noexcept {
      return !e;
   }

   friend bool operator==(std::nullptr_t,
                          const polymorphic_executor_type& e) noexcept {
      return !e;
   }

   friend bool operator!=(const polymorphic_executor_type& a,
                          const polymorphic_executor_type& b) noexcept {
      return !(a == b);
   }

   friend bool operator!=(const polymorphic_executor_type& e,
                          std::nullptr_t) noexcept {
      return !!e;
   }

   friend bool operator!=(std::nullptr_t,
                          const polymorphic_executor_type& e) noexcept {
      return !!e;
   }

private:
   // Stable interface to the notion of 'function' without arguments which
   // return void. Function is in its generalized form, can handle function
   // pointer or even lambda with capture.
   struct func_base {
      virtual ~func_base() = default;
      virtual void call()  = 0;
   };

   // Concrete form of `Function() -> void`.
   template <class Function>
   struct func_impl : func_base {
      template <class F>
      explicit func_impl(F&& f)
         : fct(std::forward<F>(f)) {}

      void call() override {
         fct();
      }

      Function fct;
   };

   using Buffer = typename std::aligned_storage<64>::type;

   struct vtable {
      void (*dtor)(void* this_);
      void (*ctor)(void* this_);
      void (*copy)(void* this_, const void* rhs);
      void (*move_ctor)(void* this_, void* rhs);
      const std::type_info& (*target_type)(const void* this_) noexcept;
      void (*execute)(const void* this_, std::unique_ptr<func_base> f);
   };
   template <class T>
   static const vtable* vtable_for() {
      // clang-format off
      static vtable vptr = {
         [](void* this_) {
            static_cast<T*>(this_)->~T();
         },
         [](void* this_) {
            new (this_) T();
         },
         [](void* this_, const void* rhs) {
            new (this_) T(*static_cast<const T*>(rhs));
         },
         [](void* this_, void* rhs) {},
         [](const void* this_) noexcept -> const std::type_info& {
            return typeid(T);
         },
         [](const void* this_, std::unique_ptr<func_base> f) {
            static_cast<const T*>(this_)->execute([f = std::move(f)]() mutable {
               f->call();
            });
         }
      };
// clang-format on
return &vptr;
}  // namespace execution

static void                  default_dtor(void*) {}
static void                  default_move_ctor(void*, void*) noexcept {}
static const std::type_info& default_target_type(const void*) noexcept {
   return typeid(void);
}

static const vtable empty;

vtable const* m_vptr = &empty;
Buffer        m_buffer;
}
;

template <class... SupportableProperties>
const typename oneway_t::polymorphic_executor_type<
   SupportableProperties...>::vtable
   oneway_t::polymorphic_executor_type<SupportableProperties...>::empty = {
      &oneway_t::polymorphic_executor_type<
         SupportableProperties...>::default_dtor,
      nullptr,  // ctor
      nullptr,  // copy
      &oneway_t::polymorphic_executor_type<
         SupportableProperties...>::default_move_ctor,
      &oneway_t::polymorphic_executor_type<
         SupportableProperties...>::default_target_type,
      nullptr  // execute
};



}  // namespace execution

#endif
