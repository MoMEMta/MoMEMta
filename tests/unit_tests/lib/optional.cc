/* Copyright 2017 The TensorFlow Authors. All Rights Reserved.

Modifications copyright 2017 Universite catholique de Louvain (UCL), Belgium

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include <lib/optional.h>

#include <catch.hpp>

#include <string>
#include <utility>

#include <momemta/Logging.h>

#include <platform/macros.h>

namespace momemta {
namespace {

using momemta::gtl::optional;
using momemta::gtl::nullopt;
using momemta::gtl::nullopt_t;
using momemta::gtl::in_place;
using momemta::gtl::in_place_t;
using momemta::gtl::make_optional;

template <typename T> std::string TypeQuals(T&) { return "&"; }
template <typename T> std::string TypeQuals(T&&) { return "&&"; }
template <typename T> std::string TypeQuals(const T&) { return "c&"; }
template <typename T> std::string TypeQuals(const T&&) { return "c&&"; }

struct StructorListener {
  int construct0 = 0;
  int construct1 = 0;
  int construct2 = 0;
  int listinit = 0;
  int copy = 0;
  int move = 0;
  int copy_assign = 0;
  int move_assign = 0;
  int destruct = 0;
};

struct Listenable {
  static StructorListener* listener;

  Listenable() { ++listener->construct0; }
  Listenable(int /*unused*/) { ++listener->construct1; }  // NOLINT
  Listenable(int /*unused*/, int /*unused*/) { ++listener->construct2; }
  Listenable(std::initializer_list<int> /*unused*/) { ++listener->listinit; }
  Listenable(const Listenable& /*unused*/) { ++listener->copy; }
  Listenable(Listenable&& /*unused*/) { ++listener->move; }  // NOLINT
  Listenable& operator=(const Listenable& /*unused*/) {
    ++listener->copy_assign;
    return *this;
  }
  Listenable& operator=(Listenable&& /*unused*/) {  // NOLINT
    ++listener->move_assign;
    return *this;
  }
  ~Listenable() { ++listener->destruct; }
};

StructorListener* Listenable::listener = nullptr;

// clang on macos -- even the latest major version at time of writing (8.x) --
// does not like much of our constexpr business.  clang < 3.0 also has trouble.
#if defined(__clang__) && defined(__APPLE__)
#define SKIP_CONSTEXPR_TEST_DUE_TO_CLANG_BUG
#endif

struct ConstexprType {
  constexpr ConstexprType() : x(0) {}
  constexpr explicit ConstexprType(int i) : x(i) {}
#ifndef SKIP_CONSTEXPR_TEST_DUE_TO_CLANG_BUG
  constexpr ConstexprType(std::initializer_list<int> il) : x(il.size()) {}
#endif
  constexpr ConstexprType(const char* s ATTRIBUTE_UNUSED) : x(-1) {}  // NOLINT
  int x;
};

struct Copyable {
  Copyable() {}
  Copyable(const Copyable&) {}
  Copyable& operator=(const Copyable&) { return *this; }
};

struct MoveableThrow {
  MoveableThrow() {}
  MoveableThrow(MoveableThrow&&) {}
  MoveableThrow& operator=(MoveableThrow&&) { return *this; }
};

struct MoveableNoThrow {
  MoveableNoThrow() {}
  MoveableNoThrow(MoveableNoThrow&&) noexcept {}
  MoveableNoThrow& operator=(MoveableNoThrow&&) noexcept { return *this; }
};

struct NonMovable {
  NonMovable() {}
  NonMovable(const NonMovable&) = delete;
  NonMovable& operator=(const NonMovable&) = delete;
  NonMovable(NonMovable&&) = delete;
  NonMovable& operator=(NonMovable&&) = delete;
};

TEST_CASE("optional", "[core/lib]") {
  SECTION("DefaultConstructor")
  {
    optional<int> empty;
    REQUIRE_FALSE(!!empty);
    constexpr optional<int> cempty;
    static_assert(!cempty.has_value(), "");
    REQUIRE(std::is_nothrow_default_constructible<optional<int>>::value);
  }

  SECTION("NullOptConstructor")
  {
    optional<int> empty(nullopt);
    REQUIRE_FALSE(!!empty);
    // Creating a temporary nullopt_t object instead of using nullopt because
    // nullopt cannot be constexpr and have external linkage at the same time.
    constexpr optional<int> cempty{nullopt_t(nullopt_t::init)};
    static_assert(!cempty.has_value(), "");
    REQUIRE((std::is_nothrow_constructible<optional<int>, nullopt_t>::value));
  }

  SECTION("CopyConstructor")
  {
    optional<int> empty, opt42 = 42;
    optional<int> empty_copy(empty);
    REQUIRE_FALSE(!!empty_copy);
    optional<int> opt42_copy(opt42);
    REQUIRE(!!opt42_copy);
    REQUIRE(42 == opt42_copy);
    // test copyablility
    REQUIRE(std::is_copy_constructible<optional<int>>::value);
    REQUIRE(std::is_copy_constructible<optional<Copyable>>::value);
    REQUIRE_FALSE(std::is_copy_constructible<optional<MoveableThrow>>::value);
    REQUIRE_FALSE(std::is_copy_constructible<optional<MoveableNoThrow>>::value);
    REQUIRE_FALSE(std::is_copy_constructible<optional<NonMovable>>::value);
  }

  SECTION("MoveConstructor")
  {
    optional<int> empty, opt42 = 42;
    optional<int> empty_move(std::move(empty));
    REQUIRE_FALSE(!!empty_move);
    optional<int> opt42_move(std::move(opt42));
    REQUIRE(!!opt42_move);
    REQUIRE(42 == opt42_move);
    // test movability
    REQUIRE(std::is_move_constructible<optional<int>>::value);
    REQUIRE(std::is_move_constructible<optional<Copyable>>::value);
    REQUIRE(std::is_move_constructible<optional<MoveableThrow>>::value);
    REQUIRE(std::is_move_constructible<optional<MoveableNoThrow>>::value);
    REQUIRE_FALSE(std::is_move_constructible<optional<NonMovable>>::value);
    // test noexcept
    REQUIRE(std::is_nothrow_move_constructible<optional<int>>::value);
    REQUIRE_FALSE(
            std::is_nothrow_move_constructible<optional<MoveableThrow>>::value);
    REQUIRE(
            std::is_nothrow_move_constructible<optional<MoveableNoThrow>>::value);
  }

  SECTION("Destructor")
  {
    struct Trivial {};

    struct NonTrivial {
        ~NonTrivial() {}
    };

    REQUIRE(std::is_trivially_destructible<optional<int>>::value);
    REQUIRE(std::is_trivially_destructible<optional<Trivial>>::value);
    REQUIRE_FALSE(std::is_trivially_destructible<optional<NonTrivial>>::value);
  }

  SECTION("InPlaceConstructor")
  {
    constexpr optional<ConstexprType> opt0{in_place_t()};
    static_assert(opt0, "");
    static_assert(opt0->x == 0, "");
    constexpr optional<ConstexprType> opt1{in_place_t(), 1};
    static_assert(opt1, "");
    static_assert(opt1->x == 1, "");
#ifndef SKIP_CONSTEXPR_TEST_DUE_TO_CLANG_BUG
    constexpr optional<ConstexprType> opt2{in_place_t(), {1, 2}};
    static_assert(opt2, "");
    static_assert(opt2->x == 2, "");
#endif

    // TODO(b/34201852): uncomment these when std::is_constructible<T, Args&&...>
    // SFINAE is added to optional::optional(in_place_t, Args&&...).
    // struct I {
    //   I(in_place_t);
    // };

    // REQUIRE_FALSE((std::is_constructible<optional<I>, in_place_t>::value));
    // REQUIRE_FALSE((std::is_constructible<optional<I>, const
    // in_place_t&>::value));
  }

// template<U=T> optional(U&&);
  SECTION("ValueConstructor")
  {
    constexpr optional<int> opt0(0);
    static_assert(opt0, "");
    static_assert(*opt0 == 0, "");
    REQUIRE((std::is_convertible<int, optional<int>>::value));
    // Copy initialization ( = "abc") won't work due to optional(optional&&)
    // is not constexpr. Use list initialization instead. This invokes
    // optional<ConstexprType>::optional<U>(U&&), with U = const char (&) [4],
    // which direct-initializes the ConstexprType value held by the optional
    // via ConstexprType::ConstexprType(const char*).
    constexpr optional<ConstexprType> opt1 = {"abc"};
    static_assert(opt1, "");
    static_assert(-1 == opt1->x, "");
    REQUIRE(
            (std::is_convertible<const char *, optional<ConstexprType>>::value));
    // direct initialization
    constexpr optional<ConstexprType> opt2{2};
    static_assert(opt2, "");
    static_assert(2 == opt2->x, "");
    REQUIRE_FALSE((std::is_convertible<int, optional<ConstexprType>>::value));

    // this invokes optional<int>::optional(int&&)
    // NOTE: this has different behavior than assignment, e.g.
    // "opt3 = {};" clears the optional rather than setting the value to 0
    constexpr optional<int> opt3({});
    static_assert(opt3, "");
    static_assert(*opt3 == 0, "");

    // this invokes the move constructor with a default constructed optional
    // because non-template function is a better match than template function.
    optional<ConstexprType> opt4({});
    REQUIRE_FALSE(!!opt4);
  }

  struct Implicit {};

  struct Explicit {};

  struct Convert {
      Convert(const Implicit&)  // NOLINT(runtime/explicit)
              : implicit(true), move(false) {}
      Convert(Implicit&&)  // NOLINT(runtime/explicit)
              : implicit(true), move(true) {}
      explicit Convert(const Explicit&) : implicit(false), move(false) {}
      explicit Convert(Explicit&&) : implicit(false), move(true) {}

      bool implicit;
      bool move;
  };

  struct ConvertFromOptional {
      ConvertFromOptional(const Implicit&)  // NOLINT(runtime/explicit)
              : implicit(true), move(false), from_optional(false) {}
      ConvertFromOptional(Implicit&&)  // NOLINT(runtime/explicit)
              : implicit(true), move(true), from_optional(false) {}
      ConvertFromOptional(const optional<Implicit>&)  // NOLINT(runtime/explicit)
              : implicit(true), move(false), from_optional(true) {}
      ConvertFromOptional(optional<Implicit>&&)  // NOLINT(runtime/explicit)
              : implicit(true), move(true), from_optional(true) {}
      explicit ConvertFromOptional(const Explicit&)
              : implicit(false), move(false), from_optional(false) {}
      explicit ConvertFromOptional(Explicit&&)
              : implicit(false), move(true), from_optional(false) {}
      explicit ConvertFromOptional(const optional<Explicit>&)
              : implicit(false), move(false), from_optional(true) {}
      explicit ConvertFromOptional(optional<Explicit>&&)
              : implicit(false), move(true), from_optional(true) {}

      bool implicit;
      bool move;
      bool from_optional;
  };

  SECTION("ConvertingConstructor")
  {
    optional<Implicit> i_empty;
    optional<Implicit> i(in_place);
    optional<Explicit> e_empty;
    optional<Explicit> e(in_place);
    {
      // implicitly constructing optional<Convert> from optional<Implicit>
      optional<Convert> empty = i_empty;
      REQUIRE_FALSE(!!empty);
      optional<Convert> opt_copy = i;
      REQUIRE(!!opt_copy);
      REQUIRE(opt_copy->implicit);
      REQUIRE_FALSE(opt_copy->move);
      optional<Convert> opt_move = optional<Implicit>(in_place);
      REQUIRE(!!opt_move);
      REQUIRE(opt_move->implicit);
      REQUIRE(opt_move->move);
    }
    {
      // explicitly constructing optional<Convert> from optional<Explicit>
      optional<Convert> empty(e_empty);
      REQUIRE_FALSE(!!empty);
      optional<Convert> opt_copy(e);
      REQUIRE(!!opt_copy);
      REQUIRE_FALSE(opt_copy->implicit);
      REQUIRE_FALSE(opt_copy->move);
      REQUIRE_FALSE((std::is_convertible<const optional<Explicit>&,
                                        optional<Convert>>::value));
      optional<Convert> opt_move{optional<Explicit>(in_place)};
      REQUIRE(!!opt_move);
      REQUIRE_FALSE(opt_move->implicit);
      REQUIRE(opt_move->move);
      REQUIRE_FALSE(
              (std::is_convertible<optional<Explicit>&&, optional<Convert>>::value));
    }
    {
      // implicitly constructing optional<ConvertFromOptional> from
      // optional<Implicit> via ConvertFromOptional(optional<Implicit>&&)
      // check that ConvertFromOptional(Implicit&&) is NOT called
      static_assert(
              gtl::internal_optional::is_constructible_convertible_from_optional<
                      ConvertFromOptional, Implicit>::value,
              "");
      optional<ConvertFromOptional> opt0 = i_empty;
      REQUIRE(!!opt0);
      REQUIRE(opt0->implicit);
      REQUIRE_FALSE(opt0->move);
      REQUIRE(opt0->from_optional);
      optional<ConvertFromOptional> opt1 = optional<Implicit>();
      REQUIRE(!!opt1);
      REQUIRE(opt1->implicit);
      REQUIRE(opt1->move);
      REQUIRE(opt1->from_optional);
    }
    {
      // implicitly constructing optional<ConvertFromOptional> from
      // optional<Explicit> via ConvertFromOptional(optional<Explicit>&&)
      // check that ConvertFromOptional(Explicit&&) is NOT called
      optional<ConvertFromOptional> opt0(e_empty);
      REQUIRE(!!opt0);
      REQUIRE_FALSE(opt0->implicit);
      REQUIRE_FALSE(opt0->move);
      REQUIRE(opt0->from_optional);
      REQUIRE_FALSE((std::is_convertible<const optional<Explicit>&,
                                        optional<ConvertFromOptional>>::value));
      optional<ConvertFromOptional> opt1{optional<Explicit>()};
      REQUIRE(!!opt1);
      REQUIRE_FALSE(opt1->implicit);
      REQUIRE(opt1->move);
      REQUIRE(opt1->from_optional);
      REQUIRE_FALSE((std::is_convertible<optional<Explicit>&&,
                                        optional<ConvertFromOptional>>::value));
    }
  }

  SECTION("StructorBasic")
  {
    StructorListener listener;
    Listenable::listener = &listener;
    {
      optional<Listenable> empty;
      REQUIRE_FALSE(!!empty);
      optional<Listenable> opt0(in_place);
      REQUIRE(!!opt0);
      optional<Listenable> opt1(in_place, 1);
      REQUIRE(!!opt1);
      optional<Listenable> opt2(in_place, 1, 2);
      REQUIRE(!!opt2);
    }
    REQUIRE(1 == listener.construct0);
    REQUIRE(1 == listener.construct1);
    REQUIRE(1 == listener.construct2);
    REQUIRE(3 == listener.destruct);
  }

  SECTION("CopyMoveStructor")
  {
    StructorListener listener;
    Listenable::listener = &listener;
    optional<Listenable> original(in_place);
    REQUIRE(1 == listener.construct0);
    REQUIRE(0 == listener.copy);
    REQUIRE(0 == listener.move);
    optional<Listenable> copy(original);
    REQUIRE(1 == listener.construct0);
    REQUIRE(1 == listener.copy);
    REQUIRE(0 == listener.move);
    optional<Listenable> move(std::move(original));
    REQUIRE(1 == listener.construct0);
    REQUIRE(1 == listener.copy);
    REQUIRE(1 == listener.move);
  }

  SECTION("ListInit")
  {
    StructorListener listener;
    Listenable::listener = &listener;
    optional<Listenable> listinit1(in_place, {1});
    optional<Listenable> listinit2(in_place, {1, 2});
    REQUIRE(2 == listener.listinit);
  }

  SECTION("AssignFromNullopt")
  {
    optional<int> opt(1);
    opt = nullopt;
    REQUIRE_FALSE(!!opt);

    StructorListener listener;
    Listenable::listener = &listener;
    optional<Listenable> opt1(in_place);
    opt1 = nullopt;
    REQUIRE_FALSE(opt1);
    REQUIRE(1 == listener.construct0);
    REQUIRE(1 == listener.destruct);

    REQUIRE((std::is_nothrow_assignable<optional<int>, nullopt_t>::value));
    REQUIRE(
            (std::is_nothrow_assignable<optional<Listenable>, nullopt_t>::value));
  }

  SECTION("CopyAssignment")
  {
    const optional<int> empty, opt1 = 1, opt2 = 2;
    optional<int> empty_to_opt1, opt1_to_opt2, opt2_to_empty;

    REQUIRE_FALSE(!!empty_to_opt1);
    empty_to_opt1 = empty;
    REQUIRE_FALSE(!!empty_to_opt1);
    empty_to_opt1 = opt1;
    REQUIRE(!!empty_to_opt1);
    REQUIRE(1 == empty_to_opt1.value());

    REQUIRE_FALSE(!!opt1_to_opt2);
    opt1_to_opt2 = opt1;
    REQUIRE(!!opt1_to_opt2);
    REQUIRE(1 == opt1_to_opt2.value());
    opt1_to_opt2 = opt2;
    REQUIRE(!!opt1_to_opt2);
    REQUIRE(2 == opt1_to_opt2.value());

    REQUIRE_FALSE(!!opt2_to_empty);
    opt2_to_empty = opt2;
    REQUIRE(!!opt2_to_empty);
    REQUIRE(2 == opt2_to_empty.value());
    opt2_to_empty = empty;
    REQUIRE_FALSE(!!opt2_to_empty);

    REQUIRE(std::is_copy_assignable<optional<Copyable>>::value);
    REQUIRE_FALSE(std::is_copy_assignable<optional<MoveableThrow>>::value);
    REQUIRE_FALSE(std::is_copy_assignable<optional<MoveableNoThrow>>::value);
    REQUIRE_FALSE(std::is_copy_assignable<optional<NonMovable>>::value);
  }

  SECTION("MoveAssignment")
  {
    StructorListener listener;
    Listenable::listener = &listener;

    optional<Listenable> empty1, empty2, set1(in_place), set2(in_place);
    REQUIRE(2 == listener.construct0);
    optional<Listenable> empty_to_empty, empty_to_set, set_to_empty(in_place),
            set_to_set(in_place);
    REQUIRE(4 == listener.construct0);
    empty_to_empty = std::move(empty1);
    empty_to_set = std::move(set1);
    set_to_empty = std::move(empty2);
    set_to_set = std::move(set2);
    REQUIRE(0 == listener.copy);
    REQUIRE(1 == listener.move);
    REQUIRE(1 == listener.destruct);
    REQUIRE(1 == listener.move_assign);

    REQUIRE(std::is_move_assignable<optional<Copyable>>::value);
    REQUIRE(std::is_move_assignable<optional<MoveableThrow>>::value);
    REQUIRE(std::is_move_assignable<optional<MoveableNoThrow>>::value);
    REQUIRE_FALSE(std::is_move_assignable<optional<NonMovable>>::value);

    REQUIRE_FALSE(std::is_nothrow_move_assignable<optional<MoveableThrow>>::value);
    REQUIRE(
            std::is_nothrow_move_assignable<optional<MoveableNoThrow>>::value);
  }

  struct NoConvertToOptional {
      // disable implicit conversion from const NoConvertToOptional&
      // to optional<NoConvertToOptional>.
      NoConvertToOptional(const NoConvertToOptional&) = delete;
  };

  struct CopyConvert {
      CopyConvert(const NoConvertToOptional&);
      CopyConvert& operator=(const CopyConvert&) = delete;
      CopyConvert& operator=(const NoConvertToOptional&);
  };

  struct CopyConvertFromOptional {
      CopyConvertFromOptional(const NoConvertToOptional&);
      CopyConvertFromOptional(const optional<NoConvertToOptional>&);
      CopyConvertFromOptional& operator=(const CopyConvertFromOptional&) = delete;
      CopyConvertFromOptional& operator=(const NoConvertToOptional&);
      CopyConvertFromOptional& operator=(const optional<NoConvertToOptional>&);
  };

  struct MoveConvert {
      MoveConvert(NoConvertToOptional&&);
      MoveConvert& operator=(const MoveConvert&) = delete;
      MoveConvert& operator=(NoConvertToOptional&&);
  };

  struct MoveConvertFromOptional {
      MoveConvertFromOptional(NoConvertToOptional&&);
      MoveConvertFromOptional(optional<NoConvertToOptional>&&);
      MoveConvertFromOptional& operator=(const MoveConvertFromOptional&) = delete;
      MoveConvertFromOptional& operator=(NoConvertToOptional&&);
      MoveConvertFromOptional& operator=(optional<NoConvertToOptional>&&);
  };

// template <class U = T> optional<T>& operator=(U&& v);
  SECTION("ValueAssignment")
  {
    optional<int> opt;
    REQUIRE_FALSE(!!opt);
    opt = 42;
    REQUIRE(!!opt);
    REQUIRE(42 == opt.value());
    opt = nullopt;
    REQUIRE_FALSE(!!opt);
    opt = 42;
    REQUIRE(!!opt);
    REQUIRE(42 == opt.value());
    opt = 43;
    REQUIRE(!!opt);
    REQUIRE(43 == opt.value());
    opt = {};  // this should clear optional
    REQUIRE_FALSE(!!opt);

    opt = {44};
    REQUIRE(!!opt);
    REQUIRE(44 == opt.value());

    // U = const NoConvertToOptional&
    REQUIRE((std::is_assignable<optional<CopyConvert>&,
                                    const NoConvertToOptional&>::value));
    // U = const optional<NoConvertToOptional>&
    REQUIRE((std::is_assignable<optional<CopyConvertFromOptional>&,
                                    const NoConvertToOptional&>::value));
    // U = const NoConvertToOptional& triggers SFINAE because
    // std::is_constructible_v<MoveConvert, const NoConvertToOptional&> is false
    REQUIRE_FALSE((std::is_assignable<optional<MoveConvert>&,
                                     const NoConvertToOptional&>::value));
    // U = NoConvertToOptional
    REQUIRE((std::is_assignable<optional<MoveConvert>&,
                                    NoConvertToOptional&&>::value));
    // U = const NoConvertToOptional& triggers SFINAE because
    // std::is_constructible_v<MoveConvertFromOptional, const
    // NoConvertToOptional&> is false
    REQUIRE_FALSE((std::is_assignable<optional<MoveConvertFromOptional>&,
                                     const NoConvertToOptional&>::value));
    // U = NoConvertToOptional
    REQUIRE((std::is_assignable<optional<MoveConvertFromOptional>&,
                                    NoConvertToOptional&&>::value));
    // U = const optional<NoConvertToOptional>&
    REQUIRE(
            (std::is_assignable<optional<CopyConvertFromOptional>&,
                                const optional<NoConvertToOptional>&>::value));
    // U = optional<NoConvertToOptional>
    REQUIRE((std::is_assignable<optional<MoveConvertFromOptional>&,
                                    optional<NoConvertToOptional>&&>::value));
  }

// template <class U> optional<T>& operator=(const optional<U>& rhs);
// template <class U> optional<T>& operator=(optional<U>&& rhs);
  SECTION("ConvertingAssignment")
  {
    optional<int> opt_i;
    optional<char> opt_c('c');
    opt_i = opt_c;
    REQUIRE(!!opt_i);
    REQUIRE(*opt_c == *opt_i);
    opt_i = optional<char>();
    REQUIRE_FALSE(!!opt_i);
    opt_i = optional<char>('d');
    REQUIRE(!!opt_i);
    REQUIRE('d' == *opt_i);

    optional<std::string> opt_str;
    optional<const char *> opt_cstr("abc");
    opt_str = opt_cstr;
    REQUIRE(!!opt_str);
    REQUIRE(std::string("abc") == *opt_str);
    opt_str = optional<const char *>();
    REQUIRE_FALSE(!!opt_str);
    opt_str = optional<const char *>("def");
    REQUIRE(!!opt_str);
    REQUIRE(std::string("def") == *opt_str);

    // operator=(const optional<U>&) with U = NoConvertToOptional
    REQUIRE(
            (std::is_assignable<optional<CopyConvert>,
                                const optional<NoConvertToOptional>&>::value));
    // operator=(const optional<U>&) with U = NoConvertToOptional
    // triggers SFINAE because
    // std::is_constructible_v<MoveConvert, const NoConvertToOptional&> is false
    REQUIRE_FALSE(
            (std::is_assignable<optional<MoveConvert>&,
                                const optional<NoConvertToOptional>&>::value));
    // operator=(optional<U>&&) with U = NoConvertToOptional
    REQUIRE((std::is_assignable<optional<MoveConvert>&,
                                    optional<NoConvertToOptional>&&>::value));
    // operator=(const optional<U>&) with U = NoConvertToOptional triggers SFINAE
    // because std::is_constructible_v<MoveConvertFromOptional,
    // const NoConvertToOptional&> is false.
    // operator=(U&&) with U = const optional<NoConverToOptional>& triggers SFINAE
    // because std::is_constructible<MoveConvertFromOptional,
    // optional<NoConvertToOptional>&&> is true.
    REQUIRE_FALSE(
            (std::is_assignable<optional<MoveConvertFromOptional>&,
                                const optional<NoConvertToOptional>&>::value));
  }

  SECTION("ResetAndHasValue")
  {
    StructorListener listener;
    Listenable::listener = &listener;
    optional<Listenable> opt;
    REQUIRE_FALSE(!!opt);
    REQUIRE_FALSE(opt.has_value());
    opt.emplace();
    REQUIRE(!!opt);
    REQUIRE(opt.has_value());
    opt.reset();
    REQUIRE_FALSE(!!opt);
    REQUIRE_FALSE(opt.has_value());
    REQUIRE(1 == listener.destruct);
    opt.reset();
    REQUIRE_FALSE(!!opt);
    REQUIRE_FALSE(opt.has_value());

    constexpr optional<int> empty;
    static_assert(!empty.has_value(), "");
    constexpr optional<int> nonempty(1);
    static_assert(nonempty.has_value(), "");
  }

  SECTION("Emplace")
  {
    StructorListener listener;
    Listenable::listener = &listener;
    optional<Listenable> opt;
    REQUIRE_FALSE(!!opt);
    opt.emplace(1);
    REQUIRE(!!opt);
    opt.emplace(1, 2);
    REQUIRE(1 == listener.construct1);
    REQUIRE(1 == listener.construct2);
    REQUIRE(1 == listener.destruct);
  }

  SECTION("ListEmplace")
  {
    StructorListener listener;
    Listenable::listener = &listener;
    optional<Listenable> opt;
    REQUIRE_FALSE(!!opt);
    opt.emplace({1});
    REQUIRE(!!opt);
    opt.emplace({1, 2});
    REQUIRE(2 == listener.listinit);
    REQUIRE(1 == listener.destruct);
  }

  SECTION("Swap")
  {
    optional<int> opt_empty, opt1 = 1, opt2 = 2;
    REQUIRE_FALSE(!!opt_empty);
    REQUIRE(!!opt1);
    REQUIRE(1 == opt1.value());
    REQUIRE(!!opt2);
    REQUIRE(2 == opt2.value());
    swap(opt_empty, opt1);
    REQUIRE_FALSE(!!opt1);
    REQUIRE(!!opt_empty);
    REQUIRE(1 == opt_empty.value());
    REQUIRE(!!opt2);
    REQUIRE(2 == opt2.value());
    swap(opt_empty, opt1);
    REQUIRE_FALSE(!!opt_empty);
    REQUIRE(!!opt1);
    REQUIRE(1 == opt1.value());
    REQUIRE(!!opt2);
    REQUIRE(2 == opt2.value());
    swap(opt1, opt2);
    REQUIRE_FALSE(!!opt_empty);
    REQUIRE(!!opt1);
    REQUIRE(2 == opt1.value());
    REQUIRE(!!opt2);
    REQUIRE(1 == opt2.value());

    REQUIRE(noexcept(opt1.swap(opt2)));
    REQUIRE(noexcept(swap(opt1, opt2)));
  }

  SECTION("PointerStuff")
  {
    optional<std::string> opt(in_place, "foo");
    REQUIRE("foo" == *opt);
    const auto& opt_const = opt;
    REQUIRE("foo" == *opt_const);
    REQUIRE(opt->size() == 3);
    REQUIRE(opt_const->size() == 3);

    constexpr optional<ConstexprType> opt1(1);
    static_assert(opt1->x == 1, "");
  }

// gcc has a bug pre 4.9 where it doesn't do correct overload resolution
// between rvalue reference qualified member methods. Skip that test to make
// the build green again when using the old compiler.
#if defined(__GNUC__) && !defined(__clang__)
#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 9)
#define SKIP_OVERLOAD_TEST_DUE_TO_GCC_BUG
#endif
#endif

  SECTION("Value")
  {
    using O = optional<std::string>;
    using CO = const optional<std::string>;
    O lvalue(in_place, "lvalue");
    CO clvalue(in_place, "clvalue");
    REQUIRE("lvalue" == lvalue.value());
    REQUIRE("clvalue" == clvalue.value());
    REQUIRE("xvalue" == O(in_place, "xvalue").value());
#ifndef SKIP_OVERLOAD_TEST_DUE_TO_GCC_BUG
    REQUIRE("cxvalue" == CO(in_place, "cxvalue").value());
    REQUIRE("&" == TypeQuals(lvalue.value()));
    REQUIRE("c&" == TypeQuals(clvalue.value()));
    REQUIRE("&&" == TypeQuals(O(in_place, "xvalue").value()));
    REQUIRE("c&&" == TypeQuals(CO(in_place, "cxvalue").value()));
#endif
  }

  SECTION("DerefOperator")
  {
    using O = optional<std::string>;
    using CO = const optional<std::string>;
    O lvalue(in_place, "lvalue");
    CO clvalue(in_place, "clvalue");
    REQUIRE("lvalue" == *lvalue);
    REQUIRE("clvalue" == *clvalue);
    REQUIRE("xvalue" == *O(in_place, "xvalue"));
#ifndef SKIP_OVERLOAD_TEST_DUE_TO_GCC_BUG
    REQUIRE("cxvalue" == *CO(in_place, "cxvalue"));
    REQUIRE("&" == TypeQuals(*lvalue));
    REQUIRE("c&" == TypeQuals(*clvalue));
    REQUIRE("&&" == TypeQuals(*O(in_place, "xvalue")));
    REQUIRE("c&&" == TypeQuals(*CO(in_place, "cxvalue")));
#endif

    constexpr optional<int> opt1(1);
    static_assert(*opt1 == 1, "");

#if !defined(SKIP_CONSTEXPR_TEST_DUE_TO_CLANG_BUG) && \
    !defined(SKIP_OVERLOAD_TEST_DUE_TO_GCC_BUG)
    using COI = const optional<int>;
    static_assert(*COI(2) == 2, "");
#endif
  }

  SECTION("ValueOr")
  {
    optional<double> opt_empty, opt_set = 1.2;
    REQUIRE(42.0 == opt_empty.value_or(42));
    REQUIRE(1.2 == opt_set.value_or(42));
    REQUIRE(42.0 == optional<double>().value_or(42));
    REQUIRE(1.2 == optional<double>(1.2).value_or(42));

#ifndef SKIP_CONSTEXPR_TEST_DUE_TO_CLANG_BUG
    constexpr optional<double> copt_empty;
    static_assert(42.0 == copt_empty.value_or(42), "");

    constexpr optional<double> copt_set = {1.2};
    static_assert(1.2 == copt_set.value_or(42), "");

    using COD = const optional<double>;
    static_assert(42.0 == COD().value_or(42), "");
    static_assert(1.2 == COD(1.2).value_or(42), "");
#endif
  }

// make_optional cannot be constexpr until C++17
  SECTION("make_optional")
  {
    auto opt_int = make_optional(42);
    REQUIRE((std::is_same<decltype(opt_int), optional<int>>::value));
    REQUIRE(42 == opt_int);

    StructorListener listener;
    Listenable::listener = &listener;

    optional<Listenable> opt0 = make_optional<Listenable>();
    REQUIRE(1 == listener.construct0);
    optional<Listenable> opt1 = make_optional<Listenable>(1);
    REQUIRE(1 == listener.construct1);
    optional<Listenable> opt2 = make_optional<Listenable>(1, 2);
    REQUIRE(1 == listener.construct2);
    optional<Listenable> opt3 = make_optional<Listenable>({1});
    optional<Listenable> opt4 = make_optional<Listenable>({1, 2});
    REQUIRE(2 == listener.listinit);
  }

  SECTION("Comparisons")
  {
    optional<int> ae, be, a2 = 2, b2 = 2, a4 = 4, b4 = 4;

#define optionalTest_Comparisons_EXPECT_LESS(x, y) \
  REQUIRE_FALSE((x) == (y));                        \
  REQUIRE((x) != (y));                         \
  REQUIRE((x) < (y));                          \
  REQUIRE_FALSE((x) > (y));                         \
  REQUIRE((x) <= (y));                         \
  REQUIRE_FALSE((x) >= (y));

#define optionalTest_Comparisons_EXPECT_SAME(x, y) \
  REQUIRE((x) == (y));                         \
  REQUIRE_FALSE((x) != (y));                        \
  REQUIRE_FALSE((x) < (y));                         \
  REQUIRE_FALSE((x) > (y));                         \
  REQUIRE((x) <= (y));                         \
  REQUIRE((x) >= (y));

#define optionalTest_Comparisons_EXPECT_GREATER(x, y) \
  REQUIRE_FALSE((x) == (y));                           \
  REQUIRE((x) != (y));                            \
  REQUIRE_FALSE((x) < (y));                            \
  REQUIRE((x) > (y));                             \
  REQUIRE_FALSE((x) <= (y));                           \
  REQUIRE((x) >= (y));

    // LHS: nullopt, ae, a2, 3, a4
    // RHS: nullopt, be, b2, 3, b4

    // optionalTest_Comparisons_EXPECT_NOT_TO_WORK(nullopt,nullopt);
    optionalTest_Comparisons_EXPECT_SAME(nullopt, be);
    optionalTest_Comparisons_EXPECT_LESS(nullopt, b2);
    // optionalTest_Comparisons_EXPECT_NOT_TO_WORK(nullopt,3);
    optionalTest_Comparisons_EXPECT_LESS(nullopt, b4);

    optionalTest_Comparisons_EXPECT_SAME(ae, nullopt);
    optionalTest_Comparisons_EXPECT_SAME(ae, be);
    optionalTest_Comparisons_EXPECT_LESS(ae, b2);
    optionalTest_Comparisons_EXPECT_LESS(ae, 3);
    optionalTest_Comparisons_EXPECT_LESS(ae, b4);

    optionalTest_Comparisons_EXPECT_GREATER(a2, nullopt);
    optionalTest_Comparisons_EXPECT_GREATER(a2, be);
    optionalTest_Comparisons_EXPECT_SAME(a2, b2);
    optionalTest_Comparisons_EXPECT_LESS(a2, 3);
    optionalTest_Comparisons_EXPECT_LESS(a2, b4);

    // optionalTest_Comparisons_EXPECT_NOT_TO_WORK(3,nullopt);
    optionalTest_Comparisons_EXPECT_GREATER(3, be);
    optionalTest_Comparisons_EXPECT_GREATER(3, b2);
    optionalTest_Comparisons_EXPECT_SAME(3, 3);
    optionalTest_Comparisons_EXPECT_LESS(3, b4);

    optionalTest_Comparisons_EXPECT_GREATER(a4, nullopt);
    optionalTest_Comparisons_EXPECT_GREATER(a4, be);
    optionalTest_Comparisons_EXPECT_GREATER(a4, b2);
    optionalTest_Comparisons_EXPECT_GREATER(a4, 3);
    optionalTest_Comparisons_EXPECT_SAME(a4, b4);
  }

  SECTION("SwapRegression")
  {
    StructorListener listener;
    Listenable::listener = &listener;

    {
      optional<Listenable> a;
      optional<Listenable> b(in_place);
      a.swap(b);
    }

    REQUIRE(1 == listener.construct0);
    REQUIRE(1 == listener.move);
    REQUIRE(2 == listener.destruct);

    {
      optional<Listenable> a(in_place);
      optional<Listenable> b;
      a.swap(b);
    }

    REQUIRE(2 == listener.construct0);
    REQUIRE(2 == listener.move);
    REQUIRE(4 == listener.destruct);
  }

  SECTION("BigStringLeakCheck")
  {
    constexpr size_t n = 1 << 16;

    using OS = optional<std::string>;

    OS a;
    OS b = nullopt;
    OS c = std::string(n, 'c');
    std::string sd(n, 'd');
    OS d = sd;
    OS e(in_place, n, 'e');
    OS f;
    f.emplace(n, 'f');

    OS ca(a);
    OS cb(b);
    OS cc(c);
    OS cd(d);
    OS ce(e);

    OS oa;
    OS ob = nullopt;
    OS oc = std::string(n, 'c');
    std::string sod(n, 'd');
    OS od = sod;
    OS oe(in_place, n, 'e');
    OS of;
    of.emplace(n, 'f');

    OS ma(std::move(oa));
    OS mb(std::move(ob));
    OS mc(std::move(oc));
    OS md(std::move(od));
    OS me(std::move(oe));
    OS mf(std::move(of));

    OS aa1;
    OS ab1 = nullopt;
    OS ac1 = std::string(n, 'c');
    std::string sad1(n, 'd');
    OS ad1 = sad1;
    OS ae1(in_place, n, 'e');
    OS af1;
    af1.emplace(n, 'f');

    OS aa2;
    OS ab2 = nullopt;
    OS ac2 = std::string(n, 'c');
    std::string sad2(n, 'd');
    OS ad2 = sad2;
    OS ae2(in_place, n, 'e');
    OS af2;
    af2.emplace(n, 'f');

    aa1 = af2;
    ab1 = ae2;
    ac1 = ad2;
    ad1 = ac2;
    ae1 = ab2;
    af1 = aa2;

    OS aa3;
    OS ab3 = nullopt;
    OS ac3 = std::string(n, 'c');
    std::string sad3(n, 'd');
    OS ad3 = sad3;
    OS ae3(in_place, n, 'e');
    OS af3;
    af3.emplace(n, 'f');

    aa3 = nullopt;
    ab3 = nullopt;
    ac3 = nullopt;
    ad3 = nullopt;
    ae3 = nullopt;
    af3 = nullopt;

    OS aa4;
    OS ab4 = nullopt;
    OS ac4 = std::string(n, 'c');
    std::string sad4(n, 'd');
    OS ad4 = sad4;
    OS ae4(in_place, n, 'e');
    OS af4;
    af4.emplace(n, 'f');

    aa4 = OS(in_place, n, 'a');
    ab4 = OS(in_place, n, 'b');
    ac4 = OS(in_place, n, 'c');
    ad4 = OS(in_place, n, 'd');
    ae4 = OS(in_place, n, 'e');
    af4 = OS(in_place, n, 'f');

    OS aa5;
    OS ab5 = nullopt;
    OS ac5 = std::string(n, 'c');
    std::string sad5(n, 'd');
    OS ad5 = sad5;
    OS ae5(in_place, n, 'e');
    OS af5;
    af5.emplace(n, 'f');

    std::string saa5(n, 'a');
    std::string sab5(n, 'a');
    std::string sac5(n, 'a');
    std::string sad52(n, 'a');
    std::string sae5(n, 'a');
    std::string saf5(n, 'a');

    aa5 = saa5;
    ab5 = sab5;
    ac5 = sac5;
    ad5 = sad52;
    ae5 = sae5;
    af5 = saf5;

    OS aa6;
    OS ab6 = nullopt;
    OS ac6 = std::string(n, 'c');
    std::string sad6(n, 'd');
    OS ad6 = sad6;
    OS ae6(in_place, n, 'e');
    OS af6;
    af6.emplace(n, 'f');

    aa6 = std::string(n, 'a');
    ab6 = std::string(n, 'b');
    ac6 = std::string(n, 'c');
    ad6 = std::string(n, 'd');
    ae6 = std::string(n, 'e');
    af6 = std::string(n, 'f');

    OS aa7;
    OS ab7 = nullopt;
    OS ac7 = std::string(n, 'c');
    std::string sad7(n, 'd');
    OS ad7 = sad7;
    OS ae7(in_place, n, 'e');
    OS af7;
    af7.emplace(n, 'f');

    aa7.emplace(n, 'A');
    ab7.emplace(n, 'B');
    ac7.emplace(n, 'C');
    ad7.emplace(n, 'D');
    ae7.emplace(n, 'E');
    af7.emplace(n, 'F');
  }

  SECTION("MoveAssignRegression")
  {
    StructorListener listener;
    Listenable::listener = &listener;

    {
      optional<Listenable> a;
      Listenable b;
      a = std::move(b);
    }

    REQUIRE(1 == listener.construct0);
    REQUIRE(1 == listener.move);
    REQUIRE(2 == listener.destruct);
  }

  SECTION("ValueType")
  {
    REQUIRE((std::is_same<optional<int>::value_type, int>::value));
    REQUIRE((std::is_same<optional<std::string>::value_type, std::string>::value));
    REQUIRE_FALSE((std::is_same<optional<int>::value_type, nullopt_t>::value));
  }

  SECTION("Hash")
  {
    std::hash<optional<int>> hash;
    std::set<size_t> hashcodes;
    hashcodes.insert(hash(nullopt));
    for (int i = 0; i < 100; ++i) {
      hashcodes.insert(hash(i));
    }
    REQUIRE(hashcodes.size() > 90);
  }

  struct MoveMeNoThrow {
      MoveMeNoThrow() : x(0) {}
      MoveMeNoThrow(const MoveMeNoThrow& other) : x(other.x) {
        LOG(fatal) << "Should not be called.";
      }
      MoveMeNoThrow(MoveMeNoThrow&& other) noexcept : x(other.x) {}
      int x;
  };

  struct MoveMeThrow {
      MoveMeThrow() : x(0) {}
      MoveMeThrow(const MoveMeThrow& other) : x(other.x) {}
      MoveMeThrow(MoveMeThrow&& other) : x(other.x) {}
      int x;
  };

  SECTION("NoExcept")
  {
    static_assert(
            std::is_nothrow_move_constructible<optional<MoveMeNoThrow>>::value, "");
    static_assert(
            !std::is_nothrow_move_constructible<optional<MoveMeThrow>>::value, "");
    std::vector<optional<MoveMeNoThrow>> v;
    for (int i = 0; i < 10; ++i)
      v.emplace_back();
  }
}

}  // namespace
}  // namespace momemta