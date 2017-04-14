/* Copyright 2016 The TensorFlow Authors. All Rights Reserved.

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

/**
 * \file
 * \brief Unit tests for the Scanner class
 * \ingroup UnitTests
 */

#include <catch.hpp>

#include <strings/Scanner.h>
#include <strings/StringPiece.h>

using namespace momemta;
using namespace momemta::strings;

/*class ScannerTest : public ::testing::Test {
 protected:
  // Returns a string with all chars that are in <clz>, in byte value order.
  string ClassStr(Scanner::CharClass clz) {
    string s;
    for (int i = 0; i < 256; ++i) {
      char ch = i;
      if (Scanner::Matches(clz, ch)) {
        s += ch;
      }
    }
    return s;
  }
};*/

TEST_CASE("Scanner", "[core/strings]") {
    SECTION("Any") {
        StringPiece remaining, match;
        REQUIRE(Scanner("   horse0123")
                        .Any(Scanner::SPACE)
                        .Any(Scanner::DIGIT)
                        .Any(Scanner::LETTER)
                        .GetResult(&remaining, &match));
        REQUIRE("   horse" == match);
        REQUIRE("0123" == remaining);

        REQUIRE(Scanner("")
                        .Any(Scanner::SPACE)
                        .Any(Scanner::DIGIT)
                        .Any(Scanner::LETTER)
                        .GetResult(&remaining, &match));
        REQUIRE("" == remaining);
        REQUIRE("" == match);

        REQUIRE(Scanner("----")
                        .Any(Scanner::SPACE)
                        .Any(Scanner::DIGIT)
                        .Any(Scanner::LETTER)
                        .GetResult(&remaining, &match));
        REQUIRE("----" == remaining);
        REQUIRE("" == match);
    }

    SECTION("AnySpace") {
        StringPiece remaining, match;
        REQUIRE(Scanner("  a b ")
                            .AnySpace()
                            .One(Scanner::LETTER)
                            .AnySpace()
                            .GetResult(&remaining, &match));
        REQUIRE("  a " == match);
        REQUIRE("b " == remaining);
    }

    SECTION("AnyEscapedNewline") {
        StringPiece remaining, match;
        REQUIRE(Scanner("\\\n")
                            .Any(Scanner::LETTER_DIGIT_UNDERSCORE)
                            .GetResult(&remaining, &match));
        REQUIRE("\\\n" == remaining);
        REQUIRE("" == match);
    }

    SECTION("AnyEmptyString") {
        StringPiece remaining, match;
        REQUIRE(Scanner("")
                            .Any(Scanner::LETTER_DIGIT_UNDERSCORE)
                            .GetResult(&remaining, &match));
        REQUIRE("" == remaining);
        REQUIRE("" == match);
    }

    SECTION("Eos") {
        REQUIRE_FALSE(Scanner("a").Eos().GetResult());
        REQUIRE(Scanner("").Eos().GetResult());
        REQUIRE_FALSE(Scanner("abc").OneLiteral("ab").Eos().GetResult());
        REQUIRE(Scanner("abc").OneLiteral("abc").Eos().GetResult());
    }

    SECTION("Many") {
        StringPiece remaining, match;
        REQUIRE(Scanner("abc").Many(Scanner::LETTER).GetResult());
        REQUIRE_FALSE(Scanner("0").Many(Scanner::LETTER).GetResult());
        REQUIRE_FALSE(Scanner("").Many(Scanner::LETTER).GetResult());

        REQUIRE(
                Scanner("abc ").Many(Scanner::LETTER).GetResult(&remaining, &match));
        REQUIRE(" " == remaining);
        REQUIRE("abc" == match);
        REQUIRE(
                Scanner("abc").Many(Scanner::LETTER).GetResult(&remaining, &match));
        REQUIRE("" == remaining);
        REQUIRE("abc" == match);
    }

    SECTION("One") {
        StringPiece remaining, match;
        REQUIRE(Scanner("abc").One(Scanner::LETTER).GetResult());
        REQUIRE_FALSE(Scanner("0").One(Scanner::LETTER).GetResult());
        REQUIRE_FALSE(Scanner("").One(Scanner::LETTER).GetResult());

        REQUIRE(Scanner("abc")
                            .One(Scanner::LETTER)
                            .One(Scanner::LETTER)
                            .GetResult(&remaining, &match));
        REQUIRE("c" == remaining);
        REQUIRE("ab" == match);
        REQUIRE(Scanner("a").One(Scanner::LETTER).GetResult(&remaining, &match));
        REQUIRE("" == remaining);
        REQUIRE("a" == match);
    }

    SECTION("OneLiteral") {
        REQUIRE_FALSE(Scanner("abc").OneLiteral("abC").GetResult());
        REQUIRE(Scanner("abc").OneLiteral("ab").OneLiteral("c").GetResult());
    }

    SECTION("ScanUntil") {
        StringPiece remaining, match;
        REQUIRE(Scanner(R"(' \1 \2 \3 \' \\'rest)")
                            .OneLiteral("'")
                            .ScanUntil('\'')
                            .OneLiteral("'")
                            .GetResult(&remaining, &match));
        REQUIRE(R"( \\'rest)" == remaining);
        REQUIRE(R"(' \1 \2 \3 \')" == match);

        // The "scan until" character is not present.
        remaining = match = "unset";
        REQUIRE_FALSE(Scanner(R"(' \1 \2 \3 \\rest)")
                             .OneLiteral("'")
                             .ScanUntil('\'')
                             .GetResult(&remaining, &match));
        REQUIRE("unset" == remaining);
        REQUIRE("unset" == match);

        // Scan until an escape character.
        remaining = match = "";
        REQUIRE(
                Scanner(R"(123\456)").ScanUntil('\\').GetResult(&remaining, &match));
        REQUIRE(R"(\456)" == remaining);
        REQUIRE("123" == match);
    }

    SECTION("ScanEscapedUntil") {
        StringPiece remaining, match;
        REQUIRE(Scanner(R"(' \1 \2 \3 \' \\'rest)")
                            .OneLiteral("'")
                            .ScanEscapedUntil('\'')
                            .OneLiteral("'")
                            .GetResult(&remaining, &match));
        REQUIRE("rest" == remaining);
        REQUIRE(R"(' \1 \2 \3 \' \\')" == match);

        // The "scan until" character is not present.
        remaining = match = "unset";
        REQUIRE_FALSE(Scanner(R"(' \1 \2 \3 \' \\rest)")
                             .OneLiteral("'")
                             .ScanEscapedUntil('\'')
                             .GetResult(&remaining, &match));
        REQUIRE("unset" == remaining);
        REQUIRE("unset" == match);
    }

    SECTION("ZeroOrOneLiteral") {
        StringPiece remaining, match;
        REQUIRE(
                Scanner("abc").ZeroOrOneLiteral("abC").GetResult(&remaining, &match));
        REQUIRE("abc" == remaining);
        REQUIRE("" == match);

        REQUIRE(
                Scanner("abcd").ZeroOrOneLiteral("ab").ZeroOrOneLiteral("c").GetResult(
                        &remaining, &match));
        REQUIRE("d" == remaining);
        REQUIRE("abc" == match);

        REQUIRE(
                Scanner("").ZeroOrOneLiteral("abc").GetResult(&remaining, &match));
        REQUIRE("" == remaining);
        REQUIRE("" == match);
    }

// Test output of GetResult (including the forms with optional params),
// and that it can be called multiple times.
    SECTION("CaptureAndGetResult") {
        StringPiece remaining, match;

        Scanner scan("  first    second");
        REQUIRE(scan.Any(Scanner::SPACE)
                            .RestartCapture()
                            .One(Scanner::LETTER)
                            .Any(Scanner::LETTER_DIGIT)
                            .StopCapture()
                            .Any(Scanner::SPACE)
                            .GetResult(&remaining, &match));
        REQUIRE("second" == remaining);
        REQUIRE("first" == match);
        REQUIRE(scan.GetResult());
        remaining = "";
        REQUIRE(scan.GetResult(&remaining));
        REQUIRE("second" == remaining);
        remaining = "";
        match = "";
        REQUIRE(scan.GetResult(&remaining, &match));
        REQUIRE("second" == remaining);
        REQUIRE("first" == match);

        scan.RestartCapture().One(Scanner::LETTER).One(Scanner::LETTER);
        remaining = "";
        match = "";
        REQUIRE(scan.GetResult(&remaining, &match));
        REQUIRE("cond" == remaining);
        REQUIRE("se" == match);
    }

// Tests that if StopCapture is not called, then calling GetResult, then
// scanning more, then GetResult again will update the capture.
    SECTION("MultipleGetResultExtendsCapture") {
        StringPiece remaining, match;

        Scanner scan("one2three");
        REQUIRE(scan.Many(Scanner::LETTER).GetResult(&remaining, &match));
        REQUIRE("2three" == remaining);
        REQUIRE("one" == match);
        REQUIRE(scan.Many(Scanner::DIGIT).GetResult(&remaining, &match));
        REQUIRE("three" == remaining);
        REQUIRE("one2" == match);
        REQUIRE(scan.Many(Scanner::LETTER).GetResult(&remaining, &match));
        REQUIRE("" == remaining);
        REQUIRE("one2three" == match);
    }

    SECTION("FailedMatchDoesntChangeResult") {
        // A failed match doesn't change pointers passed to GetResult.
        Scanner scan("name");
        StringPiece remaining = "rem";
        StringPiece match = "match";
        REQUIRE_FALSE(scan.One(Scanner::SPACE).GetResult(&remaining, &match));
        REQUIRE("rem" == remaining);
        REQUIRE("match" == match);
    }

    SECTION("DefaultCapturesAll") {
        // If RestartCapture() is not called, the whole string is used.
        Scanner scan("a b");
        StringPiece remaining = "rem";
        StringPiece match = "match";
        REQUIRE(scan.Any(Scanner::LETTER)
                            .AnySpace()
                            .Any(Scanner::LETTER)
                            .GetResult(&remaining, &match));
        REQUIRE("" == remaining);
        REQUIRE("a b" == match);
    }

    SECTION("AllCharClasses") {

        auto ClassStr = [](Scanner::CharClass clz) -> std::string {
            std::string s;
            for (int i = 0; i < 256; ++i) {
                char ch = i;
                if (Scanner::Matches(clz, ch)) {
                    s += ch;
                }
            }
            return s;
        };

        REQUIRE(256 == ClassStr(Scanner::ALL).size());
        REQUIRE("0123456789" == ClassStr(Scanner::DIGIT));
        REQUIRE("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" ==
                  ClassStr(Scanner::LETTER));
        REQUIRE("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" ==
                  ClassStr(Scanner::LETTER_DIGIT));
        REQUIRE(
                "-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_"
                        "abcdefghijklmnopqrstuvwxyz" ==
                ClassStr(Scanner::LETTER_DIGIT_DASH_UNDERSCORE));
        REQUIRE(
                "-./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                        "abcdefghijklmnopqrstuvwxyz" ==
                ClassStr(Scanner::LETTER_DIGIT_DASH_DOT_SLASH));
        REQUIRE(
                "-./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_"
                        "abcdefghijklmnopqrstuvwxyz" ==
                ClassStr(Scanner::LETTER_DIGIT_DASH_DOT_SLASH_UNDERSCORE));
        REQUIRE(".0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" ==
                  ClassStr(Scanner::LETTER_DIGIT_DOT));
        REQUIRE("+-.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" ==
                  ClassStr(Scanner::LETTER_DIGIT_DOT_PLUS_MINUS));
        REQUIRE(".0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz" ==
                  ClassStr(Scanner::LETTER_DIGIT_DOT_UNDERSCORE));
        REQUIRE("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz" ==
                  ClassStr(Scanner::LETTER_DIGIT_UNDERSCORE));
        REQUIRE("abcdefghijklmnopqrstuvwxyz" == ClassStr(Scanner::LOWERLETTER));
        REQUIRE("0123456789abcdefghijklmnopqrstuvwxyz" ==
                  ClassStr(Scanner::LOWERLETTER_DIGIT));
        REQUIRE("0123456789_abcdefghijklmnopqrstuvwxyz" ==
                  ClassStr(Scanner::LOWERLETTER_DIGIT_UNDERSCORE));
        REQUIRE("123456789" == ClassStr(Scanner::NON_ZERO_DIGIT));
        REQUIRE("\t\n\v\f\r " == ClassStr(Scanner::SPACE));
        REQUIRE("ABCDEFGHIJKLMNOPQRSTUVWXYZ" == ClassStr(Scanner::UPPERLETTER));
    }

    SECTION("Peek") {
        REQUIRE('a' == Scanner("abc").Peek());
        REQUIRE('a' == Scanner("abc").Peek('b'));
        REQUIRE('\0' == Scanner("").Peek());
        REQUIRE('z' == Scanner("").Peek('z'));
        REQUIRE('A' == Scanner("0123A").Any(Scanner::DIGIT).Peek());
        REQUIRE('\0' == Scanner("0123A").Any(Scanner::LETTER_DIGIT).Peek());
    }
}
