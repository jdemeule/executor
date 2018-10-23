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
#include <stdexcept>
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

// template <class Executor>
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

class bad_executor : public std::runtime_error {
public:
   bad_executor()
      : std::runtime_error("bad_executor") {}
};

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


// struct require_fn {
//   template <class E, class P0>
//   constexpr auto operator()(E&& e, P0&&) const -> std::enable_if_t<
//      std::decay_t<P0>::is_requirable &&
//         std::decay_t<P0>::template static_query_v<std::decay_t<E>>,
//      std::decay_t<E>> {
//      return std::forward<E>(e);
//   }
//   // The only supported query is on the main property (oneway_t /
//   bulkoneway_t)
//   // for now.
//};

// struct require_fn {
//   template <
//      class E,
//      class P0,
//      class E00 = std::decay_t<E>,
//      class P00 = std::decay_t<P0>,
//      class = std::enable_if<P00::is_requirable && P00::static_query_v<E00>>>
//   constexpr std::decay_t<E> operator()(E&& e, P0&&) const {
//      return std::forward<E>(e);
//   }
//};
//
// constexpr require_fn require = {};

template <class E, class P, class = void>
struct Require {
   static const bool value = false;
};

template <class E, class P>
struct Require<E, P, typename P::template static_query<E>> {
   static const bool value = P::is_requirable && P::template static_query<E>::value;
};

template <class E>
struct Require<E, oneway_t> {
   static const bool value = is_oneway_executor<E>::value;
};


// Case N == 0, P0::is_requirable is true and P0::static_query_v<E> is well form
// and valid
template <class E,
          class P0,
          class E00 = std::decay_t<E>,
          class P00 = std::decay_t<P0>>
std::enable_if_t<Require<E00, P00>::value, E00> require(E&& e, P0&&) {
   return std::forward<E>(e);
}

template <class T>
struct has_require_method {
   template <class C, class P>
   static auto f(P* p) -> decltype(std::declval<C>().require(*p));

   template <class, class>
   static auto f(...) -> std::false_type;

   struct anything {
      template <class U>
      operator U();
   };

   using type = decltype(f<T, anything>(nullptr));
   static const bool value = !std::is_same<std::false_type, type>::value;
};

// template <class E,
//          class P0,
//          class E00 = std::decay_t<E>,
//          class P00 = std::decay_t<P0>>
// std::enable_if_t<!Require<E00, P00>::value && has_require_method<E00>::value,
//                 typename has_require_method<E00>::type>
// require(E&& e, P0&& p0) {
//   return e.require(std::forward<P0>(p0));
//}

template <class T, class P>
struct has_require_template_method {
   template <class C>
   static auto f(P* p) -> decltype(std::declval<C>().require(*p));

   template <class>
   static auto f(...) -> std::false_type;

   using type = decltype(f<T>(nullptr));
   static const bool value = !std::is_same<std::false_type, type>::value;
};

template <class E,
          class P0,
          class E00 = std::decay_t<E>,
          class P00 = std::decay_t<P0>>
std::enable_if_t<!Require<E00, P00>::value &&
                    has_require_template_method<E00, P00>::value,
                 typename has_require_template_method<E00, P00>::type>
require(E&& e, P0&& p0) {
   return e.require(std::forward<P0>(p0));
}

struct context_t {
   static constexpr bool is_requirable = false;
   static constexpr bool is_preferable = false;

   // using polymorphic_query_result_type = any;

   // No queries on it for now.
   // template <class Executor>
   // static constexpr decltype(auto) static_query_v =
   //   Executor::query(*static_cast<context_t*>(nullptr));
};

struct oneway_t {
   static constexpr bool is_requirable = true;
   static constexpr bool is_preferable = false;

   using polymorphic_query_result_type = bool;

   // Not polymorphic on properties for now
   template <class... SupportableProperties>
   class polymorphic_executor_type;

   // Extension to allow implementation in C++11 compiler
   template <class Executor>
   using static_query = is_oneway_executor<Executor>;

   template <class Executor>
   static constexpr bool static_query_v = is_oneway_executor<Executor>::value;

   static constexpr bool value() {
      return true;
   }
};

constexpr oneway_t oneway = {};


template <class... Ts>
struct type_list {};


template <class P, class... Px>
struct find_property;

template <class P, class H, class... Tail>
struct find_property<P, H, Tail...> : find_property<P, Tail...> {};

template <class P, class... Tail>
struct find_property<P, P, Tail...> : P {};

template <class P>
struct find_property<P> {};





// Not polymorphic on properties for now
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

   // Not Implemented for now.
   // template <class... OtherSupportableProperties>
   // polymorphic_executor_type(
   //   polymorphic_executor_type<OtherSupportableProperties...> e);

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

   // Limitation: for now only the exact match instead of convertible one is
   // handle.
   template <class Property,
             class Found = find_property<Property, SupportableProperties...>>
   std::enable_if_t<Found::is_requirable, polymorphic_executor_type> require(
      const Property& p) const {
      if (!*this)
         throw bad_executor();
      return m_vptr->require(&m_buffer, typeid(Property), &p);
   }

   // Not implemented for now.
   // template <class Property>
   // typename Property::polymorphic_query_result_type query(Property) const;

   // polymorphic_executor_type capacity:

   explicit operator bool() const noexcept {
      return m_vptr != empty;
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

   // Not implemented for now.
   // template <class... OtherSupportableProperties>
   // polymorphic_executor_type<OtherSupportableProperties...>
   // static_executor_cast() const;



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
         return a.m_vptr->eq(&a.m_buffer, &b.m_buffer);
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
      virtual void call() = 0;
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
      void (*ctor)(void* this_);
      void (*copy)(void* this_, const void* rhs);
      void (*move_ctor)(void* this_, void* rhs);
      void (*dtor)(void* this_);
      bool (*eq)(const void* lhs, const void* rhs);
      const std::type_info& (*target_type)(const void* this_);
      void (*execute)(const void* this_, std::unique_ptr<func_base> f);
      polymorphic_executor_type (*require)(const void* this_,
                                           const std::type_info& tp,
                                           const void* p);
   };

   template <class T>
   struct vtable_model {
      static void ctor(void* this_) {
         new (this_) T();
      }
      static void copy_ctor(void* this_, const void* rhs) {
         new (this_) T(*static_cast<const T*>(rhs));
      }
      static void move_ctor(void* this_, void* rhs) {
         new (this_) T(std::move(*static_cast<T*>(rhs)));
      }
      static void dtor(void* this_) {
         static_cast<T*>(this_)->~T();
      }
      static bool eq(const void* lhs, const void* rhs) {
         return *static_cast<const T*>(lhs) == *static_cast<const T*>(rhs);
      }
      static const std::type_info& target_type(const void*) {
         return typeid(T);
      }
      static void execute(const void* this_, std::unique_ptr<func_base> f) {
         static_cast<const T*>(this_)->execute(
            [f = std::move(f)]() mutable { f->call(); });
      }
      static polymorphic_executor_type require(const void* this_,
                                               const std::type_info& tp,
                                               const void* p) {
         return require_(
            static_cast<const T*>(this_), tp, p,
            type_list<SupportableProperties...>{});
      }

      template <class Head, class... Tail>
      static polymorphic_executor_type require_(const T* this_,
                                                const std::type_info& tp,
                                                const void* p,
                                                type_list<Head, Tail...>) {
         if (typeid(Head) == tp)
            return execution::require(*this_, *static_cast<const Head*>(p));
         return require_(this_, tp, p, type_list<Tail...>{});
      }

      static polymorphic_executor_type require_(const T*, const std::type_info&,
                                                const void*, type_list<>) {
         return {};
      }
   };

   template <class T>
   static const vtable* vtable_for() {
      static vtable vptr = {
         &vtable_model<T>::ctor,      &vtable_model<T>::copy_ctor,
         &vtable_model<T>::move_ctor, &vtable_model<T>::dtor,
         &vtable_model<T>::eq,        &vtable_model<T>::target_type,
         &vtable_model<T>::execute,   &vtable_model<T>::require };
      return &vptr;
   }

   struct default_vtable_model {
      static void ctor(void*) {}
      static void copy_ctor(void*, const void*) {}
      static void move_ctor(void*, void*) {}
      static void dtor(void*) {}
      static bool eq(const void*, const void*) {
         return true;
      }
      static const std::type_info& target_type(const void*) {
         return typeid(void);
      }
      static void execute(const void*, std::unique_ptr<func_base>) {}
      static polymorphic_executor_type require(const void*,
                                               const std::type_info&,
                                               const void*) {
         return {};
      }
   };


   static const vtable* default_vtable() {
      static vtable vptr = {
         &default_vtable_model::ctor,      &default_vtable_model::copy_ctor,
         &default_vtable_model::move_ctor, &default_vtable_model::dtor,
         &default_vtable_model::eq,        &default_vtable_model::target_type,
         &default_vtable_model::execute,   &default_vtable_model::require };
      return &vptr;
   }


   static const vtable* empty;

   vtable const* m_vptr = empty;
   Buffer m_buffer;
};


// Variant when will be polymorphic on properties.
template <class... SupportableProperties>
const typename oneway_t::polymorphic_executor_type<
   SupportableProperties...>::vtable*
   oneway_t::polymorphic_executor_type<SupportableProperties...>::empty =
      default_vtable();



}  // namespace execution

#endif
