#include "execution.hpp"
#include "static_thread_pool.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>


struct inline_executor {
   friend bool operator==(const inline_executor&, const inline_executor&) noexcept {
      return true;
   }

   friend bool operator!=(const inline_executor&, const inline_executor&) noexcept {
      return false;
   }

   template <class Function>
   void execute(Function f) const noexcept {
      f();
   }
};

struct not_inline_executor {
   friend bool operator==(const not_inline_executor&, const not_inline_executor&) noexcept {
      return true;
   }

   friend bool operator!=(const not_inline_executor&, const not_inline_executor&) noexcept {
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

//   static_assert(execution::is_executable_impl<inline_executor, nullary_fct>::value, "");
//   static_assert(execution::is_executable_impl<inline_executor, nullary_fct&>::value, "");
//   static const bool is_executable_cr = is_executable_impl<E, const nullary_fct&>::value;
//   static const bool is_executable_rr = is_executable_impl<E, nullary_fct&&>::value;
//
   static_assert(execution::is_oneway_executor<inline_executor>::value, "");
   static_assert(!execution::is_oneway_executor<other_kind_of_executor>::value, "");
   //   static_assert(!execution::is_oneway_executor<thrower_executor>::value, "");

   static_assert(
      std::is_same<inline_executor, decltype(execution::require(inline_executor{}, execution::oneway_t{}))>::value, "");

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
      static_assert(execution::is_oneway_executor<inline_executor>::value, "one way executor requirements not met");
      inline_executor ex;
      auto            ex2 = execution::require(ex, execution::oneway);
   }

   ex.execute([] { std::cout << "we made it\n"; });
}
