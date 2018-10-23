#include "execution.hpp"
//#include "static_thread_pool.hpp"

#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>


// struct vtable {
//   void (*dtor)(void* this_);
//   void (*ctor)(void* this_);
//   void (*copy)(void* this_, const void* rhs);
//   void (*move_ctor)(void* this_, void* rhs);
//   const std::type_info& (*target_type)(const void* this_);
//};
//
// template <class T>
// struct vtable_model {
//   static void ctor(void* this_) {
//      new (this_) T();
//   }
//   static void copy_ctor(void* this_, const void* rhs) {
//      new (this_) T(*static_cast<const T*>(rhs));
//   }
//   static void move_ctor(void* this_, void* rhs) {
//      new (this_) T(std::move(*static_cast<T*>(rhs)));
//   }
//   static void dtor(void* this_) {
//      static_cast<T*>(this_)->~T();
//   }
//   static const std::type_info& target_type(const void*) {
//      return typeid(T);
//   }
//
//   static const vtable vptr;
//};
//
// template <class T>
// const vtable vtable_model<T>::vptr = {&dtor, &ctor, &copy_ctor, &move_ctor,
//                                      &target_type};
//
// template <>
// struct vtable_model<void> {
//   static void                  ctor(void*) {}
//   static void                  copy_ctor(void*, const void*) {}
//   static void                  move_ctor(void*, void*) {}
//   static void                  dtor(void*) {}
//   static const std::type_info& target_type(const void*) {
//      return typeid(void);
//   }
//
//   static const vtable vptr;
//};
//
// template <>
// const vtable vtable_model<void>::vptr = {&dtor, &ctor, &copy_ctor,
// &move_ctor,
//                                         &target_type};
//
// template <class T>
// const vtable* vtable_for() {
//   static vtable vptr = {&vtable_model<T>::dtor, &vtable_model<T>::ctor,
//                         &vtable_model<T>::copy_ctor,
//                         &vtable_model<T>::move_ctor,
//                         &vtable_model<T>::target_type};
//   return &vptr;
//   // return &vtable_model<T>::vptr;
//}
//
// struct inline_executor {
//   friend bool operator==(const inline_executor&,
//                          const inline_executor&) noexcept {
//      return true;
//   }
//
//   friend bool operator!=(const inline_executor&,
//                          const inline_executor&) noexcept {
//      return false;
//   }
//
//   template <class Function>
//   void execute(Function f) const noexcept {
//      f();
//   }
//};
//
//
// TEST(Foo, Bar) {
//   const vtable* vtrp = vtable_for<inline_executor>();
//}


struct inline_executor {
   friend bool operator==(const inline_executor&,
                          const inline_executor&) noexcept {
      return true;
   }

   friend bool operator!=(const inline_executor&,
                          const inline_executor&) noexcept {
      return false;
   }

   template <class Function>
   void execute(Function f) const noexcept {
      f();
   }
};

struct not_inline_executor {
   friend bool operator==(const not_inline_executor&,
                          const not_inline_executor&) noexcept {
      return true;
   }

   friend bool operator!=(const not_inline_executor&,
                          const not_inline_executor&) noexcept {
      return false;
   }

   template <class Function>
   void execute(Function f) const noexcept {
      f();
   }
};

struct other_kind_of_executor {
   template <class Function>
   void execute(Function f, bool) const noexcept {
      f();
   }
};

struct thrower_executor {
   thrower_executor(const thrower_executor&) noexcept(false);
   template <class Function>
   void execute(Function f) const noexcept {}
};

struct custom_prop {
   static constexpr bool is_requirable = true;
   static constexpr bool is_preferable = true;
   using polymorphic_executor_type = bool;

   bool value = false;
};

struct custom_executor {
   friend bool operator==(const custom_executor&,
                          const custom_executor&) noexcept {
      return true;
   }

   friend bool operator!=(const custom_executor&,
                          const custom_executor&) noexcept {
      return false;
   }

   custom_executor require(custom_prop p) const {
      return {p.value};
   }

   template <class Function>
   void execute(Function f) const noexcept {
      f();
   }

   bool prop = false;
};

// template <class T>
// struct Foo {
//   struct vtable {
//      void (*dtor)(void* this_);
//   };
//   static const vtable empty;
//};

// template <class T>
// const typename Foo<T>::vtable Foo<T>::empty = ;

TEST(Execution, 0) {

   //   using Bar = Foo<int>;
   //
   //   Bar b;
   //   std::cout << &Bar::empty << "\n";

   struct nullary_fct {
      void operator()() {}
   };

   // static_assert(
   //   execution::is_executable_impl<inline_executor, nullary_fct>::value, "");
   // static_assert(
   //   execution::is_executable_impl<inline_executor, nullary_fct&>::value,
   //   "");
   // static const bool is_executable_cr =
   //   is_executable_impl<E, const nullary_fct&>::value;
   // static const bool is_executable_rr =
   //   is_executable_impl<E, nullary_fct&&>::value;

   static_assert(execution::is_oneway_executor<inline_executor>::value, "");
   static_assert(!execution::is_oneway_executor<other_kind_of_executor>::value,
                 "");
   // static_assert(!execution::is_oneway_executor<thrower_executor>::value,
   // "");

   static_assert(
      std::is_same<inline_executor,
                   decltype(execution::require(inline_executor{},
                                               execution::oneway_t{}))>::value,
      "");

   using executor = execution::executor<execution::oneway_t>;

   executor ex = inline_executor{};

   EXPECT_EQ(ex.target_type(), typeid(inline_executor));
   EXPECT_TRUE(static_cast<bool>(ex));
   EXPECT_FALSE(static_cast<bool>(executor{}));

   EXPECT_TRUE(ex.target<inline_executor>() != nullptr);
   EXPECT_TRUE(ex.target<not_inline_executor>() == nullptr);
   EXPECT_TRUE(executor{}.target<inline_executor>() == nullptr);

   EXPECT_TRUE(ex == ex);
   EXPECT_TRUE(ex != executor{});
   EXPECT_TRUE(ex == executor(inline_executor{}));
   EXPECT_TRUE(nullptr == executor{});

   {
      executor ex2 = ex;
      EXPECT_EQ(ex2.target_type(), typeid(inline_executor));
      EXPECT_TRUE(static_cast<bool>(ex2));
      EXPECT_FALSE(static_cast<bool>(executor{}));

      EXPECT_TRUE(ex2.target<inline_executor>() != nullptr);
      EXPECT_TRUE(ex2.target<not_inline_executor>() == nullptr);

      EXPECT_TRUE(ex2 == ex2);
      EXPECT_TRUE(ex == ex2);
      EXPECT_TRUE(ex2 != executor{});
      EXPECT_TRUE(ex2 == executor(inline_executor{}));
   }
   {
      executor tmp = ex;
      executor ex2 = std::move(tmp);
      EXPECT_EQ(ex2.target_type(), typeid(inline_executor));
      EXPECT_TRUE(static_cast<bool>(ex2));
      EXPECT_FALSE(static_cast<bool>(executor{}));

      EXPECT_TRUE(ex2.target<inline_executor>() != nullptr);
      EXPECT_TRUE(ex2.target<not_inline_executor>() == nullptr);

      EXPECT_TRUE(ex2 == ex2);
      EXPECT_TRUE(ex == ex2);
      EXPECT_TRUE(ex2 != executor{});
      EXPECT_TRUE(ex2 == executor(inline_executor{}));
   }
   {
      executor tmp = ex;
      executor ex2;
      ex2.swap(tmp);

      EXPECT_EQ(ex2.target_type(), typeid(inline_executor));
      EXPECT_TRUE(static_cast<bool>(ex2));
      EXPECT_FALSE(static_cast<bool>(executor{}));

      EXPECT_TRUE(ex2.target<inline_executor>() != nullptr);
      EXPECT_TRUE(ex2.target<not_inline_executor>() == nullptr);

      EXPECT_TRUE(ex2 == ex2);
      EXPECT_TRUE(ex == ex2);
      EXPECT_TRUE(ex2 != executor{});
      EXPECT_TRUE(ex2 == executor(inline_executor{}));
   }

   {
      static_assert(execution::is_oneway_executor<inline_executor>::value,
                    "one way executor requirements not met");
      static_assert(execution::oneway_t::static_query<inline_executor>::value,
                    "");
      inline_executor ex;
      auto ex2 = execution::require(ex, execution::oneway);
      ex2.execute([] { std::cout << "hello\n"; });
   }

   {
      static_assert(
         !execution::is_oneway_executor<other_kind_of_executor>::value, "");
      static_assert(
         !execution::oneway_t::static_query<other_kind_of_executor>::value, "");
      // auto nop =
      //   execution::require(other_kind_of_executor{}, execution::oneway);
      // nop.execute([] {});
   }

   {
      static_assert(execution::is_oneway_executor<custom_executor>::value, "");
      static_assert(execution::has_require_method<custom_executor>::value, "");
      auto ex2 = execution::require(custom_executor(), custom_prop{true});
      EXPECT_TRUE(ex2.prop);
   }

   {
      using EX = execution::executor<execution::oneway_t, custom_prop>;
      EX ex5 = custom_executor{};
      static_assert(
         execution::has_require_template_method<EX, custom_prop>::value, "");
      // auto ex6 = ex5.require(custom_prop{ true });
      auto ex6 = execution::require(ex5, custom_prop{true});
      const custom_executor* ex7 = ex6.target<custom_executor>();
      EXPECT_TRUE(ex7->prop);
   }

   ex.execute([] { std::cout << "we made it\n"; });
}
