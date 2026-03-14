/**
 * @file test_basic.cpp
 * @brief Complete test suite for csv.hpp.
 *
 * Self-contained: no external test framework required.
 * Every public symbol is exercised; each test is independent and labelled.
 */

#include <csv/csv.hpp>

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// ╔══════════════════════════════════════════════════════════════════════════╗
// ║  Minimal test harness                                                   ║
// ╚══════════════════════════════════════════════════════════════════════════╝

namespace harness
{

  static int total = 0;
  static int passed = 0;
  static int failed = 0;
  static std::string current_suite;

  /// Open a named test suite (purely cosmetic grouping).
  inline void suite(const std::string &name)
  {
    current_suite = name;
    std::cout << "\n┌─ " << name << '\n';
  }

  /// Register a single test result.
  inline void check(bool condition,
                    const std::string &description,
                    const char *file, int line)
  {
    ++total;
    if (condition)
    {
      ++passed;
      std::cout << "│  \033[32m✓\033[0m  " << description << '\n';
    }
    else
    {
      ++failed;
      std::cout << "│  \033[31m✗\033[0m  " << description
                << "  \033[2m(" << file << ':' << line << ")\033[0m\n";
    }
  }

  /// Assert that calling expr throws ExceptionType.
  template <typename ExceptionType, typename Fn>
  inline void throws(Fn &&fn,
                     const std::string &description,
                     const char *file, int line)
  {
    bool caught = false;
    try
    {
      std::forward<Fn>(fn)();
    }
    catch (const ExceptionType &)
    {
      caught = true;
    }
    catch (...)
    {
    }
    check(caught, description, file, line);
  }

  /// Assert that calling expr does NOT throw.
  template <typename Fn>
  inline void nothrows(Fn &&fn,
                       const std::string &description,
                       const char *file, int line)
  {
    bool ok = true;
    try
    {
      std::forward<Fn>(fn)();
    }
    catch (...)
    {
      ok = false;
    }
    check(ok, description, file, line);
  }

  inline void summary()
  {
    std::cout << "\n══════════════════════════════════════════\n";
    if (failed == 0)
      std::cout << "\033[32m  ALL " << total << " TESTS PASSED\033[0m\n";
    else
      std::cout << "\033[31m  " << failed << " / " << total
                << " TESTS FAILED\033[0m\n";
    std::cout << "══════════════════════════════════════════\n";
  }

} // namespace harness

// ── Convenience macros ─────────────────────────────────────────────────────

#define CHECK(expr) harness::check((expr), #expr, __FILE__, __LINE__)
#define CHECK_MSG(expr, m) harness::check((expr), (m), __FILE__, __LINE__)
#define THROWS(T, expr) harness::throws<T>([&]() { expr; }, "throws " #T ": " #expr, __FILE__, __LINE__)
#define NO_THROW(expr) harness::nothrows([&]() { expr; }, "no throw: " #expr, __FILE__, __LINE__)
#define SUITE(name) harness::suite(name)

// ╔══════════════════════════════════════════════════════════════════════════╗
// ║  Helpers                                                                ║
// ╚══════════════════════════════════════════════════════════════════════════╝

static csv::Table P(const std::string &s, const csv::Options &o = {})
{
  return csv::parse(s, o);
}

static std::string W(const csv::Table &t, const csv::WriteOptions &o = {})
{
  return csv::write(t, o);
}

// ╔══════════════════════════════════════════════════════════════════════════╗
// ║  Test suites                                                             ║
// ╚══════════════════════════════════════════════════════════════════════════╝

// ── 1. ParseError ──────────────────────────────────────────────────────────
static void test_parse_error()
{
  SUITE("ParseError");

  // constructors / accessors
  {
    csv::ParseError e("bad input", 3, 7);
    CHECK(e.line() == 3);
    CHECK(e.col() == 7);
    CHECK(e.message() == "bad input");
    // what() must contain the message
    CHECK(std::string(e.what()).find("bad input") != std::string::npos);
    // what() must contain the line number
    CHECK(std::string(e.what()).find('3') != std::string::npos);
  }

  // position-unknown variant
  {
    csv::ParseError e("oops");
    CHECK(e.line() == 0);
    CHECK(e.col() == 0);
  }

  // inheritance
  {
    bool caught_base = false;
    try
    {
      throw csv::ParseError("x");
    }
    catch (const std::invalid_argument &)
    {
      caught_base = true;
    }
    CHECK_MSG(caught_base, "ParseError is-a std::invalid_argument");
  }
}

// ── 2. parse – basic fields ────────────────────────────────────────────────
static void test_parse_basic()
{
  SUITE("parse – basic fields");

  // single row, no trailing newline
  {
    auto t = P("a,b,c");
    CHECK(t.size() == 1);
    CHECK(t[0] == csv::Row({"a", "b", "c"}));
  }

  // single row, trailing newline
  {
    auto t = P("a,b,c\n");
    CHECK(t.size() == 1);
    CHECK(t[0] == csv::Row({"a", "b", "c"}));
  }

  // two rows
  {
    auto t = P("a,b\n1,2\n");
    CHECK(t.size() == 2);
    CHECK(t[0] == csv::Row({"a", "b"}));
    CHECK(t[1] == csv::Row({"1", "2"}));
  }

  // three rows, five columns
  {
    auto t = P("h1,h2,h3,h4,h5\nv1,v2,v3,v4,v5\nw1,w2,w3,w4,w5\n");
    CHECK(t.size() == 3);
    CHECK(t[0].size() == 5);
    CHECK(t[2][4] == "w5");
  }

  // empty input → empty table
  {
    auto t = P("");
    CHECK(t.empty());
  }

  // single field, single row
  {
    auto t = P("hello");
    CHECK(t.size() == 1 && t[0].size() == 1 && t[0][0] == "hello");
  }

  // empty fields between separators
  {
    auto t = P(",,,");
    CHECK(t.size() == 1 && t[0].size() == 4);
    CHECK(std::all_of(t[0].begin(), t[0].end(),
                      [](const std::string &s)
                      { return s.empty(); }));
  }

  // numeric content (strings preserved verbatim)
  {
    auto t = P("1,2,3\n4,5,6\n");
    CHECK(t[1][2] == "6");
  }
}

// ── 3. parse – line endings ────────────────────────────────────────────────
static void test_parse_line_endings()
{
  SUITE("parse – line endings");

  // LF only
  {
    auto t = P("a,b\n1,2\n");
    CHECK(t.size() == 2);
  }

  // CRLF
  {
    auto t = P("a,b\r\n1,2\r\n");
    CHECK(t.size() == 2);
    CHECK(t[0] == csv::Row({"a", "b"}));
    CHECK(t[1] == csv::Row({"1", "2"}));
  }

  // bare CR (legacy Mac style)
  {
    auto t = P("a,b\r1,2\r");
    CHECK(t.size() == 2);
    CHECK(t[1][0] == "1");
  }

  // mixed CRLF and LF in same document
  {
    auto t = P("a,b\r\nc,d\ne,f\r\n");
    CHECK(t.size() == 3);
    CHECK(t[1] == csv::Row({"c", "d"}));
  }

  // no trailing newline → row still committed
  {
    auto t = P("x,y\nz,w");
    CHECK(t.size() == 2);
    CHECK(t[1] == csv::Row({"z", "w"}));
  }
}

// ── 4. parse – quoted fields ───────────────────────────────────────────────
static void test_parse_quoted()
{
  SUITE("parse – quoted fields");

  // basic quoted field
  {
    auto t = P(R"("hello",world)");
    CHECK(t[0][0] == "hello");
    CHECK(t[0][1] == "world");
  }

  // comma inside quoted field
  {
    auto t = P(R"("hello, world",end)");
    CHECK(t[0][0] == "hello, world");
  }

  // escaped (doubled) quote
  {
    auto t = P(R"("say ""hi""",end)");
    CHECK(t[0][0] == R"(say "hi")");
  }

  // only a doubled quote inside field
  {
    auto t = P(R"("""")");
    CHECK(t[0][0] == "\"");
  }

  // multiple doubled quotes
  {
    auto t = P(R"("a""b""c")");
    CHECK(t[0][0] == "a\"b\"c");
  }

  // LF inside quoted field → still one logical row
  {
    auto t = P("\"line1\nline2\",after");
    CHECK(t.size() == 1);
    CHECK(t[0][0] == "line1\nline2");
    CHECK(t[0][1] == "after");
  }

  // CRLF inside quoted field
  {
    auto t = P("\"cr\r\nlf\",x");
    CHECK(t.size() == 1);
    CHECK(t[0][0] == "cr\r\nlf");
  }

  // quoted empty field
  {
    auto t = P(R"("","")");
    CHECK(t[0][0].empty() && t[0][1].empty());
  }

  // entire document is one big quoted field
  {
    auto t = P("\"a,b,c\"");
    CHECK(t.size() == 1 && t[0].size() == 1);
    CHECK(t[0][0] == "a,b,c");
  }

  // quotes in the middle of several rows
  {
    auto t = P("\"A\",B\nC,\"D\"\n");
    CHECK(t.size() == 2);
    CHECK(t[0][0] == "A");
    CHECK(t[1][1] == "D");
  }
}

// ── 5. parse – malformed input must throw ─────────────────────────────────
static void test_parse_malformed()
{
  SUITE("parse – malformed input");

  // unclosed quote (EOF inside quoted field)
  THROWS(csv::ParseError, P("\"unclosed"));

  // unclosed quote with content
  THROWS(csv::ParseError, P("a,\"still open\nb,c"));

  // illegal byte after closing quote (not sep / newline / another quote)
  THROWS(csv::ParseError, P(R"("ok"X)"));

  // illegal byte after closing quote — letter
  THROWS(csv::ParseError, P(R"("value"extra,b)"));

  // unclosed quote spanning multiple lines
  THROWS(csv::ParseError, P("\"line1\nline2\nline3"));

  // catches as base class
  {
    bool caught = false;
    try
    {
      P("\"bad");
    }
    catch (const std::invalid_argument &)
    {
      caught = true;
    }
    CHECK_MSG(caught, "malformed CSV caught as std::invalid_argument");
  }
}

// ── 6. parse – Options: separator ─────────────────────────────────────────
static void test_options_separator()
{
  SUITE("Options – separator");

  // semicolon
  {
    csv::Options o;
    o.separator = ';';
    auto t = P("a;b;c\n1;2;3\n", o);
    CHECK(t[0] == csv::Row({"a", "b", "c"}));
    CHECK(t[1] == csv::Row({"1", "2", "3"}));
  }

  // tab (TSV)
  {
    csv::Options o;
    o.separator = '\t';
    auto t = P("x\ty\tz\n10\t20\t30\n", o);
    CHECK(t[1][2] == "30");
  }

  // pipe
  {
    csv::Options o;
    o.separator = '|';
    auto t = P("a|b|c", o);
    CHECK(t[0].size() == 3 && t[0][1] == "b");
  }

  // separator inside quoted field is literal
  {
    csv::Options o;
    o.separator = ';';
    auto t = P("\"a;b\";c", o);
    CHECK(t[0][0] == "a;b");
    CHECK(t[0][1] == "c");
  }
}

// ── 7. parse – Options: trim_whitespace ───────────────────────────────────
static void test_options_trim()
{
  SUITE("Options – trim_whitespace");

  csv::Options o;
  o.trim_whitespace = true;

  // leading and trailing spaces
  {
    auto t = P("  hello  ,  world  ", o);
    CHECK(t[0][0] == "hello");
    CHECK(t[0][1] == "world");
  }

  // tabs
  {
    auto t = P("\tfoo\t,\tbar\t", o);
    CHECK(t[0][0] == "foo");
    CHECK(t[0][1] == "bar");
  }

  // already clean — no change
  {
    auto t = P("a,b,c", o);
    CHECK(t[0] == csv::Row({"a", "b", "c"}));
  }

  // empty after trim → still empty string
  {
    auto t = P("   ,   ", o);
    CHECK(t[0][0].empty() && t[0][1].empty());
  }

  // quoted fields must NOT be trimmed
  {
    auto t = P(R"("  keep  ",plain  )", o);
    CHECK(t[0][0] == "  keep  "); // preserved
    CHECK(t[0][1] == "plain");    // trimmed (unquoted)
  }
}

// ── 8. parse – Options: skip_empty_lines ──────────────────────────────────
static void test_options_skip_empty()
{
  SUITE("Options – skip_empty_lines");

  csv::Options o;
  o.skip_empty_lines = true;

  // blank line in middle
  {
    auto t = P("a,b\n\nc,d\n", o);
    CHECK(t.size() == 2);
    CHECK(t[1] == csv::Row({"c", "d"}));
  }

  // blank line at start
  {
    auto t = P("\na,b\n", o);
    CHECK(t.size() == 1);
  }

  // blank line at end
  {
    auto t = P("a,b\n\n", o);
    CHECK(t.size() == 1);
  }

  // multiple consecutive blank lines
  {
    auto t = P("a\n\n\n\nb\n", o);
    CHECK(t.size() == 2);
  }

  // all blank → empty table
  {
    auto t = P("\n\n\n", o);
    CHECK(t.empty());
  }

  // without the option: blank line → one-row with empty field
  {
    auto t = P("a\n\nb\n");
    CHECK(t.size() == 3);
    CHECK(t[1].size() == 1 && t[1][0].empty());
  }
}

// ── 9. parse – Options: skip_comments ─────────────────────────────────────
static void test_options_comments()
{
  SUITE("Options – skip_comments");

  csv::Options o;
  o.skip_comments = true; // default comment_char = '#'

  // comment at start
  {
    auto t = P("# comment\na,b\n1,2\n", o);
    CHECK(t.size() == 2);
    CHECK(t[0] == csv::Row({"a", "b"}));
  }

  // comment in the middle
  {
    auto t = P("a,b\n# mid\n1,2\n", o);
    CHECK(t.size() == 2);
    CHECK(t[1] == csv::Row({"1", "2"}));
  }

  // multiple comments
  {
    auto t = P("# c1\n# c2\na,b\n# c3\n1,2\n", o);
    CHECK(t.size() == 2);
  }

  // custom comment_char = '!'
  {
    csv::Options o2;
    o2.skip_comments = true;
    o2.comment_char = '!';
    auto t = P("! ignore\na,b\n! also\n1,2\n", o2);
    CHECK(t.size() == 2);
  }

  // line starting with whitespace then '#' counts as comment
  {
    auto t = P("   # indented comment\na,b\n", o);
    CHECK(t.size() == 1);
  }

  // '#' not at start of first non-whitespace → regular data
  {
    auto t = P("a#b,c\n", o);
    CHECK(t[0][0] == "a#b");
  }
}

// ── 10. parse – Options: max_field_size ───────────────────────────────────
static void test_options_max_field_size()
{
  SUITE("Options – max_field_size");

  csv::Options o;
  o.max_field_size = 5;

  // within limit
  NO_THROW(P("ab,cde,xy", o));

  // exactly at limit
  NO_THROW(P("12345,a", o));

  // one byte over
  THROWS(csv::ParseError, P("toolong,b", o));

  // inside a quoted field also checked
  THROWS(csv::ParseError, P(R"("toolong",b)", o));

  // limit 0 = unlimited
  {
    csv::Options o2;
    o2.max_field_size = 0;
    NO_THROW(P(std::string(10000, 'x'), o2));
  }
}

// ── 11. parse – Options: max_fields_per_row ───────────────────────────────
static void test_options_max_fields()
{
  SUITE("Options – max_fields_per_row");

  csv::Options o;
  o.max_fields_per_row = 3;

  // exactly 3
  NO_THROW(P("a,b,c", o));

  // 4 → throws
  THROWS(csv::ParseError, P("a,b,c,d", o));

  // limit 0 = unlimited
  {
    csv::Options o2;
    o2.max_fields_per_row = 0;
    NO_THROW(P("a,b,c,d,e,f,g,h,i,j", o2));
  }
}

// ── 12. parse – Options: field_transformer ────────────────────────────────
static void test_options_field_transformer()
{
  SUITE("Options – field_transformer");

  // upper-case everything
  {
    csv::Options o;
    o.field_transformer = [](std::string &s)
    {
      for (char &c : s)
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    };
    auto t = P("hello,world\nfoo,bar\n", o);
    CHECK(t[0][0] == "HELLO");
    CHECK(t[1][1] == "BAR");
  }

  // transformer applied after trim
  {
    csv::Options o;
    o.trim_whitespace = true;
    o.field_transformer = [](std::string &s)
    { s = "X" + s; };
    auto t = P("  hi  ,  there  ", o);
    CHECK(t[0][0] == "Xhi");
    CHECK(t[0][1] == "Xthere");
  }

  // transformer on quoted fields too
  {
    csv::Options o;
    o.field_transformer = [](std::string &s)
    { s = "[" + s + "]"; };
    auto t = P(R"("quoted",plain)", o);
    CHECK(t[0][0] == "[quoted]");
    CHECK(t[0][1] == "[plain]");
  }
}

// ── 13. parse – Options: row_filter ───────────────────────────────────────
static void test_options_row_filter()
{
  SUITE("Options – row_filter");

  // keep only rows with 3 fields
  {
    csv::Options o;
    o.row_filter = [](const csv::Row &r)
    { return r.size() == 3; };
    auto t = P("a,b,c\n1,2\n3,4,5\n", o);
    CHECK(t.size() == 2);
    CHECK(t[0] == csv::Row({"a", "b", "c"}));
    CHECK(t[1] == csv::Row({"3", "4", "5"}));
  }

  // discard all rows → empty table
  {
    csv::Options o;
    o.row_filter = [](const csv::Row &)
    { return false; };
    auto t = P("a,b\n1,2\n", o);
    CHECK(t.empty());
  }

  // keep all rows
  {
    csv::Options o;
    o.row_filter = [](const csv::Row &)
    { return true; };
    auto t = P("a,b\n1,2\n", o);
    CHECK(t.size() == 2);
  }
}

// ── 14. parse – from std::istream ─────────────────────────────────────────
static void test_parse_istream()
{
  SUITE("parse – from istream");

  // basic istringstream
  {
    std::istringstream ss("a,b,c\n1,2,3\n");
    auto t = csv::parse(ss);
    CHECK(t.size() == 2);
    CHECK(t[1] == csv::Row({"1", "2", "3"}));
  }

  // empty stream → empty table
  {
    std::istringstream ss("");
    auto t = csv::parse(ss);
    CHECK(t.empty());
  }

  // large stream (>64 KiB buffer boundary)
  {
    std::ostringstream builder;
    for (int i = 0; i < 5000; ++i)
      builder << i << ',' << (i * 2) << '\n';
    std::istringstream ss(builder.str());
    auto t = csv::parse(ss);
    CHECK(t.size() == 5000);
    CHECK(t[4999][1] == std::to_string(4999 * 2));
  }

  // options forwarded to istream overload
  {
    std::istringstream ss("# comment\na,b\n1,2\n");
    csv::Options o;
    o.skip_comments = true;
    auto t = csv::parse(ss, o);
    CHECK(t.size() == 2);
  }
}

// ── 15. write_row ──────────────────────────────────────────────────────────
static void test_write_row()
{
  SUITE("write_row");

  // plain fields – no quoting needed
  CHECK(csv::write_row({"a", "b", "c"}) == "a,b,c");

  // empty fields
  CHECK(csv::write_row({"", "", ""}) == ",,");

  // field with comma → must be quoted
  CHECK(csv::write_row({"hello, world", "end"}) == R"("hello, world",end)");

  // field with double-quote → doubled
  CHECK(csv::write_row({R"(say "hi")"}) == R"("say ""hi""")");

  // field with LF → quoted
  {
    std::string r = csv::write_row({"line1\nline2", "x"});
    CHECK(r.find('"') != std::string::npos);
    CHECK(r.find("line1\nline2") != std::string::npos);
  }

  // field with CR → quoted
  {
    std::string r = csv::write_row({"cr\rhere"});
    CHECK(r.front() == '"' && r.back() == '"');
  }

  // single empty row
  CHECK(csv::write_row({""}) == "");

  // single quoted-only result
  CHECK(csv::write_row({"a"}) == "a");

  // WriteOptions: semicolon separator
  {
    csv::WriteOptions wo;
    wo.separator = ';';
    CHECK(csv::write_row({"a", "b", "c"}, wo) == "a;b;c");
  }

  // WriteOptions: always_quote
  {
    csv::WriteOptions wo;
    wo.always_quote = true;
    std::string r = csv::write_row({"a", "b"}, wo);
    CHECK(r == "\"a\",\"b\"");
  }

  // WriteOptions: custom quote_char (single-quote)
  {
    csv::WriteOptions wo;
    wo.quote_char = '\'';
    wo.always_quote = true;
    std::string r = csv::write_row({"x"}, wo);
    CHECK(r == "'x'");
  }
}

// ── 16. write ──────────────────────────────────────────────────────────────
static void test_write_table()
{
  SUITE("write – full table");

  // empty table → empty string
  CHECK(csv::write({}) == "");

  // one row
  CHECK(csv::write({{"a", "b", "c"}}) == "a,b,c\n");

  // multiple rows, default LF
  {
    csv::Table t = {{"h1", "h2"}, {"v1", "v2"}, {"v3", "v4"}};
    std::string s = csv::write(t);
    CHECK(s == "h1,h2\nv1,v2\nv3,v4\n");
  }

  // CRLF line endings
  {
    csv::WriteOptions wo;
    wo.line_ending = "\r\n";
    csv::Table t = {{"a", "b"}, {"1", "2"}};
    std::string s = csv::write(t, wo);
    CHECK(s == "a,b\r\n1,2\r\n");
  }

  // fields needing quoting are escaped
  {
    csv::Table t = {{"name", "note"}, {"Alice", "hello, world"}, {"Bob", "say \"hi\""}};
    std::string s = csv::write(t);
    // Re-parse must recover identical table
    CHECK(csv::tables_equal(csv::parse(s), t));
  }

  // always_quote
  {
    csv::WriteOptions wo;
    wo.always_quote = true;
    std::string s = csv::write({{"a", "b"}}, wo);
    CHECK(s == "\"a\",\"b\"\n");
  }
}

// ── 17. write_to ──────────────────────────────────────────────────────────
static void test_write_to()
{
  SUITE("write_to – ostream");

  csv::Table t = {{"x", "y"}, {"1", "2"}, {"3", "4"}};

  // matches csv::write()
  {
    std::ostringstream oss;
    csv::write_to(oss, t);
    CHECK(oss.str() == csv::write(t));
  }

  // CRLF option honoured
  {
    csv::WriteOptions wo;
    wo.line_ending = "\r\n";
    std::ostringstream oss;
    csv::write_to(oss, t, wo);
    CHECK(oss.str().find("\r\n") != std::string::npos);
  }

  // empty table → nothing written
  {
    std::ostringstream oss;
    csv::write_to(oss, {});
    CHECK(oss.str().empty());
  }
}

// ── 18. round-trip (parse → write → parse) ────────────────────────────────
static void test_roundtrip()
{
  SUITE("round-trip: parse → write → parse");

  auto rt = [](const std::string &raw, const csv::Options &po = {},
               const csv::WriteOptions &wo = {})
  {
    return csv::parse(csv::write(csv::parse(raw, po), wo));
  };

  // plain data
  {
    const std::string s = "name,age\nAlice,30\nBob,25\n";
    CHECK(csv::tables_equal(rt(s), csv::parse(s)));
  }

  // fields with commas
  {
    csv::Table t = {{"a"}, {"hello, world"}, {"end"}};
    CHECK(csv::tables_equal(csv::parse(csv::write(t)), t));
  }

  // fields with quotes
  {
    csv::Table t = {{"msg"}, {R"(say "hi")"}};
    CHECK(csv::tables_equal(csv::parse(csv::write(t)), t));
  }

  // fields with newlines
  {
    csv::Table t = {{"text"}, {"line1\nline2"}};
    CHECK(csv::tables_equal(csv::parse(csv::write(t)), t));
  }

  // CRLF round-trip
  {
    csv::Table t = {{"h1", "h2"}, {"a", "b"}, {"c", "d"}};
    csv::WriteOptions wo;
    wo.line_ending = "\r\n";
    CHECK(csv::tables_equal(csv::parse(csv::write(t, wo)), t));
  }

  // semicolon separator round-trip
  {
    csv::Options po;
    po.separator = ';';
    csv::WriteOptions wo;
    wo.separator = ';';
    auto t = csv::parse("a;b;c\n1;2;3\n", po);
    CHECK(csv::tables_equal(csv::parse(csv::write(t, wo), po), t));
  }
}

// ── 19. load / save (file I/O) ─────────────────────────────────────────────
static void test_file_io()
{
  SUITE("load / save – file I/O");

  const fs::path tmp = fs::temp_directory_path() / "csv_test_basic.csv";
  const fs::path tmp2 = fs::temp_directory_path() / "csv_test_basic2.csv";

  csv::Table original = {
      {"id", "name", "note"},
      {"1", "Alice", "hello, world"},
      {"2", "Bob", R"(say "hi")"},
      {"3", "Carol", "multi\nline"},
  };

  // save and reload
  {
    csv::save(tmp.string(), original);
    csv::Table loaded = csv::load(tmp.string());
    CHECK_MSG(csv::tables_equal(original, loaded),
              "file round-trip (LF)");
  }

  // CRLF round-trip
  {
    csv::WriteOptions wo;
    wo.line_ending = "\r\n";
    csv::save(tmp2.string(), original, wo);
    csv::Table loaded = csv::load(tmp2.string());
    CHECK_MSG(csv::tables_equal(original, loaded),
              "file round-trip (CRLF)");
  }

  // save/load with c-string path
  {
    csv::save(tmp.string().c_str(), original);
    csv::Table loaded = csv::load(tmp.string().c_str());
    CHECK_MSG(csv::tables_equal(original, loaded),
              "load/save via const char*");
  }

  // load: non-existent file → ios_base::failure
  THROWS(std::ios_base::failure, csv::load("__no_such_file_xyz__.csv"));

  // save: bad path → ios_base::failure
  THROWS(std::ios_base::failure,
         csv::save("/no/such/directory/out.csv", original));

  // load: malformed CSV → ParseError
  {
    std::ofstream bad(tmp.string());
    bad << "\"unclosed\n";
    bad.close();
    THROWS(csv::ParseError, csv::load(tmp.string()));
  }

  // deprecated aliases still compile and work
  {
    csv::save(tmp.string(), original);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    csv::Table loaded = csv::load_csv(tmp.string());
#pragma GCC diagnostic pop
    CHECK_MSG(csv::tables_equal(original, loaded),
              "deprecated load_csv() still works");
  }

  fs::remove(tmp);
  fs::remove(tmp2);
}

// ── 20. RowView ─────────────────────────────────────────────────────────────
static void test_row_view()
{
  SUITE("RowView");

  csv::Row header = {"name", "age", "city"};
  csv::Row data = {"Alice", "30", "Paris"};
  csv::RowView v(header, data);

  // size
  CHECK(v.size() == 3);
  CHECK(!v.empty());

  // at() by index
  CHECK(v.at(0) == "Alice");
  CHECK(v.at(2) == "Paris");

  // at() out of range
  THROWS(std::out_of_range, v.at(5));

  // operator[] by name
  CHECK(v["name"] == "Alice");
  CHECK(v["age"] == "30");
  CHECK(v["city"] == "Paris");

  // operator[] unknown column
  THROWS(std::out_of_range, (void)v["missing"]);

  // contains()
  CHECK(v.contains("age"));
  CHECK(!v.contains("score"));

  // header() / data() accessors
  CHECK(v.header()[1] == "age");
  CHECK(v.data()[0] == "Alice");

  // iterator (range-for yields (header_field, data_field) pairs)
  {
    std::vector<std::string> cols, vals;
    for (const auto &[col, val] : v)
    {
      cols.push_back(std::string(col));
      vals.push_back(std::string(val));
    }
    CHECK(cols == std::vector<std::string>({"name", "age", "city"}));
    CHECK(vals == std::vector<std::string>({"Alice", "30", "Paris"}));
  }

  // short data row — size capped to min(header, data)
  {
    csv::Row short_data = {"Bob"};
    csv::RowView vs(header, short_data);
    CHECK(vs.size() == 1);
    CHECK(vs.at(0) == "Bob");
  }
}

// ── 21. DictReader ─────────────────────────────────────────────────────────
static void test_dict_reader()
{
  SUITE("DictReader");

  csv::Table t = csv::parse("name,age,city\nAlice,30,NYC\nBob,25,LA\nCarol,35,Paris\n");

  // non-owning constructor
  {
    csv::DictReader reader(t);
    CHECK(reader.size() == 3);
    CHECK(!reader.empty());
    CHECK(reader.fieldnames() == csv::Row({"name", "age", "city"}));

    std::vector<std::string> names;
    for (const csv::RowView &row : reader)
      names.push_back(std::string(row["name"]));
    CHECK(names == std::vector<std::string>({"Alice", "Bob", "Carol"}));
  }

  // owning constructor (move)
  {
    csv::Table t2 = csv::parse("x,y\n1,2\n3,4\n");
    csv::DictReader reader(std::move(t2));
    CHECK(reader.size() == 2);
    CHECK(reader.fieldnames()[0] == "x");
  }

  // empty table
  {
    csv::DictReader empty_reader(csv::Table{});
    CHECK(empty_reader.empty());
    CHECK(empty_reader.fieldnames().empty());
    CHECK(empty_reader.begin() == empty_reader.end());
  }

  // table with only header (no data rows)
  {
    csv::Table header_only = {{"a", "b", "c"}};
    csv::DictReader reader(header_only);
    CHECK(reader.size() == 0);
    CHECK(reader.empty());
  }

  // named access inside loop
  {
    int total_age = 0;
    for (const csv::RowView &row : csv::DictReader(t))
      total_age += std::stoi(std::string(row["age"]));
    CHECK(total_age == 90);
  }
}

// ── 22. DictWriter ─────────────────────────────────────────────────────────
static void test_dict_writer()
{
  SUITE("DictWriter");

  // basic writerow
  {
    csv::DictWriter w({"name", "age", "city"});
    w.writerow({{"name", "Alice"}, {"age", "30"}, {"city", "NYC"}});
    w.writerow({{"name", "Bob"}, {"age", "25"}, {"city", "LA"}});
    csv::Table t = csv::parse(w.str());
    CHECK(t.size() == 3); // header + 2 data rows
    CHECK(t[1][0] == "Alice");
    CHECK(t[2][1] == "25");
  }

  // missing key → empty field
  {
    csv::DictWriter w({"a", "b", "c"});
    w.writerow({{"a", "1"}, {"c", "3"}}); // b missing
    csv::Table t = csv::parse(w.str());
    CHECK(t[1][1].empty());
  }

  // extra key → silently ignored
  {
    csv::DictWriter w({"a", "b"});
    w.writerow({{"a", "x"}, {"b", "y"}, {"z", "IGNORED"}});
    csv::Table t = csv::parse(w.str());
    CHECK(t[1].size() == 2);
  }

  // no header option
  {
    csv::DictWriter w({"x", "y"}, {}, /*write_header=*/false);
    w.writerow({{"x", "1"}, {"y", "2"}});
    csv::Table t = csv::parse(w.str());
    CHECK(t.size() == 1); // only the data row, no header
    CHECK(t[0][0] == "1");
  }

  // writerows()
  {
    csv::DictWriter w({"id", "val"});
    w.writerows({
        {{"id", "1"}, {"val", "alpha"}},
        {{"id", "2"}, {"val", "beta"}},
        {{"id", "3"}, {"val", "gamma"}},
    });
    csv::Table t = csv::parse(w.str());
    CHECK(t.size() == 4);
    CHECK(t[3][1] == "gamma");
  }

  // release() empties the buffer
  {
    csv::DictWriter w({"k", "v"});
    w.writerow({{"k", "a"}, {"v", "b"}});
    std::string s = w.release();
    CHECK(!s.empty());
    CHECK(w.str().empty()); // buffer cleared
  }

  // bytes_written()
  {
    csv::DictWriter w({"a"});
    std::size_t before = w.bytes_written();
    w.writerow({{"a", "hello"}});
    CHECK(w.bytes_written() > before);
  }

  // flush_to()
  {
    csv::DictWriter w({"p", "q"});
    w.writerow({{"p", "x"}, {"q", "y"}});
    std::ostringstream oss;
    w.flush_to(oss);
    CHECK(!oss.str().empty());
    CHECK(w.str().empty());
  }

  // WriteOptions forwarded
  {
    csv::WriteOptions wo;
    wo.separator = ';';
    wo.line_ending = "\r\n";
    csv::DictWriter w({"a", "b"}, wo);
    w.writerow({{"a", "1"}, {"b", "2"}});
    CHECK(w.str().find(';') != std::string::npos);
    CHECK(w.str().find("\r\n") != std::string::npos);
  }
}

// ── 23. StreamingParser ────────────────────────────────────────────────────
static void test_streaming_parser()
{
  SUITE("StreamingParser");

  // basic callback invocation
  {
    std::vector<csv::Row> collected;
    csv::StreamingParser p([&](const csv::Row &r)
                           { collected.push_back(r); });
    p.parse(std::string("a,b\n1,2\n3,4\n"));
    CHECK(collected.size() == 3);
    CHECK(collected[0] == csv::Row({"a", "b"}));
    CHECK(collected[2] == csv::Row({"3", "4"}));
  }

  // no Table allocated (result via callback only)
  {
    std::size_t n = 0;
    csv::StreamingParser p([&](const csv::Row &)
                           { ++n; });
    p.parse(std::string("x\ny\nz\n"));
    CHECK(n == 3);
  }

  // incremental push()
  {
    std::size_t rows = 0;
    csv::StreamingParser p([&](const csv::Row &)
                           { ++rows; });
    const std::string chunk1 = "a,b,";
    const std::string chunk2 = "c\n1,2,3\n";
    p.push(chunk1.data(), chunk1.size());
    p.push(chunk2.data(), chunk2.size());
    p.finish();
    CHECK(rows == 2);
  }

  // reset() and reuse
  {
    std::size_t n = 0;
    csv::StreamingParser p([&](const csv::Row &)
                           { ++n; });
    p.parse(std::string("a\nb\n"));
    CHECK(n == 2);
    p.reset();
    p.parse(std::string("x\ny\nz\n"));
    CHECK(n == 5); // 2 + 3
  }

  // istream overload
  {
    std::size_t n = 0;
    csv::StreamingParser p([&](const csv::Row &)
                           { ++n; });
    std::istringstream ss("1,2\n3,4\n5,6\n");
    p.parse(ss);
    CHECK(n == 3);
  }

  // options forwarded (skip_empty_lines)
  {
    std::size_t n = 0;
    csv::Options opt;
    opt.skip_empty_lines = true;
    csv::StreamingParser p([&](const csv::Row &)
                           { ++n; }, opt);
    p.parse(std::string("a\n\nb\n\n\nc\n"));
    CHECK(n == 3);
  }

  // malformed input → ParseError propagates
  THROWS(csv::ParseError, {
    csv::StreamingParser p([](const csv::Row &) {});
    p.parse(std::string("\"unclosed"));
  });
}

// ── 24. rows() range ──────────────────────────────────────────────────────
static void test_rows_range()
{
  SUITE("rows() – C++20 range");

  csv::Table t = csv::parse("name,score\nAlice,88\nBob,74\nCarol,92\n");

  // skips header (index 0)
  {
    std::size_t n = 0;
    for ([[maybe_unused]] const csv::RowView &r : csv::rows(t))
      ++n;
    CHECK(n == 3);
  }

  // named access inside range-for
  {
    std::vector<std::string> names;
    for (const csv::RowView &r : csv::rows(t))
      names.push_back(std::string(r["name"]));
    CHECK(names == std::vector<std::string>({"Alice", "Bob", "Carol"}));
  }

  // size() and empty()
  {
    const auto range = csv::rows(t);
    CHECK(range.size() == 3);
    CHECK(!range.empty());
  }

  // empty table
  {
    csv::Table empty;
    const auto range = csv::rows(empty);
    CHECK(range.empty());
    CHECK(range.size() == 0);
  }

  // header-only table → data range is empty
  {
    csv::Table header_only = {{"a", "b", "c"}};
    const auto range = csv::rows(header_only);
    CHECK(range.empty());
  }

  // iterator post-increment
  {
    auto range = csv::rows(t);
    auto it = range.begin();
    auto first = *it++;
    CHECK(std::string(first["name"]) == "Alice");
    CHECK(std::string((*it)["name"]) == "Bob");
  }
}

// ── 25. column_index ──────────────────────────────────────────────────────
static void test_column_index()
{
  SUITE("column_index");

  csv::Row h = {"id", "name", "score", "city"};

  CHECK(csv::column_index(h, "id") == std::optional<std::size_t>(0));
  CHECK(csv::column_index(h, "score") == std::optional<std::size_t>(2));
  CHECK(csv::column_index(h, "city") == std::optional<std::size_t>(3));
  CHECK(csv::column_index(h, "nope") == std::nullopt);

  // empty header
  CHECK(csv::column_index(csv::Row{}, "x") == std::nullopt);
}

// ── 26. column() ──────────────────────────────────────────────────────────
static void test_column_extract()
{
  SUITE("column – extract single column");

  csv::Table t = csv::parse("name,age,city\nAlice,30,NYC\nBob,25,LA\nCarol,35,Paris\n");

  // valid column
  {
    auto names = csv::column(t, "name");
    CHECK(names.size() == 3);
    CHECK(names[0] == "Alice");
    CHECK(names[2] == "Carol");
  }

  {
    auto ages = csv::column(t, "age");
    int sum = 0;
    for (const auto &a : ages)
      sum += std::stoi(a);
    CHECK(sum == 90);
  }

  // unknown column → out_of_range
  THROWS(std::out_of_range, csv::column(t, "missing"));

  // empty table → empty vector
  CHECK(csv::column(csv::Table{}, "x").empty());

  // short row padded with empty string
  {
    csv::Table t2 = {{"a", "b"}, {"1", "2"}, {"3"}}; // second data row has only 1 field
    auto col = csv::column(t2, "b");
    CHECK(col[0] == "2");
    CHECK(col[1].empty());
  }
}

// ── 27. select_columns ────────────────────────────────────────────────────
static void test_select_columns()
{
  SUITE("select_columns");

  csv::Table t = csv::parse("a,b,c,d\n1,2,3,4\n5,6,7,8\n");

  // subset in different order
  {
    auto sub = csv::select_columns(t, {"c", "a"});
    CHECK(sub[0] == csv::Row({"c", "a"}));
    CHECK(sub[1] == csv::Row({"3", "1"}));
    CHECK(sub[2] == csv::Row({"7", "5"}));
  }

  // single column
  {
    auto sub = csv::select_columns(t, {"b"});
    CHECK(sub[0].size() == 1 && sub[0][0] == "b");
    CHECK(sub[1][0] == "2");
  }

  // duplicate column name (allowed)
  {
    auto sub = csv::select_columns(t, {"a", "a"});
    CHECK(sub[1] == csv::Row({"1", "1"}));
  }

  // unknown column → out_of_range
  THROWS(std::out_of_range, csv::select_columns(t, {"a", "zzz"}));

  // empty table → empty result
  CHECK(csv::select_columns(csv::Table{}, {"a"}).empty());
}

// ── 28. filter_rows ───────────────────────────────────────────────────────
static void test_filter_rows()
{
  SUITE("filter_rows");

  csv::Table t = csv::parse("name,score\nAlice,88\nBob,55\nCarol,92\nDave,70\n");

  // keep high scorers
  {
    auto f = csv::filter_rows(t, [](const csv::Row &r)
                              { return r.size() >= 2 && std::stoi(r[1]) >= 80; });
    CHECK(f.size() == 3); // header + Alice + Carol
    CHECK(f[1][0] == "Alice");
    CHECK(f[2][0] == "Carol");
  }

  // header always included, even when no data rows match
  {
    auto f = csv::filter_rows(t, [](const csv::Row &r)
                              { return r.size() >= 2 && r[1] == "999"; });
    CHECK(f.size() == 1);
    CHECK(f[0] == csv::Row({"name", "score"}));
  }

  // keep all
  {
    auto f = csv::filter_rows(t, [](const csv::Row &)
                              { return true; });
    CHECK(csv::tables_equal(f, t));
  }

  // empty table
  CHECK(csv::filter_rows(csv::Table{}, [](const csv::Row &)
                         { return true; })
            .empty());
}

// ── 29. transform_fields ─────────────────────────────────────────────────
static void test_transform_fields()
{
  SUITE("transform_fields");

  csv::Table t = {{"Hello", "World"}, {"foo", "BAR"}};

  // lower-case
  auto lower = csv::transform_fields(t, [](std::string &s)
                                     {
        for (char& c : s)
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c))); });
  CHECK(lower[0] == csv::Row({"hello", "world"}));
  CHECK(lower[1] == csv::Row({"foo", "bar"}));

  // original not modified (transform returns a copy)
  CHECK(t[0][0] == "Hello");

  // append suffix
  auto suffixed = csv::transform_fields(t, [](std::string &s)
                                        { s += "!"; });
  CHECK(suffixed[1][1] == "BAR!");
}

// ── 30. transpose ─────────────────────────────────────────────────────────
static void test_transpose()
{
  SUITE("transpose");

  // square
  {
    csv::Table t = {{"a", "b"}, {"1", "2"}, {"3", "4"}};
    auto tx = csv::transpose(t);
    CHECK(tx.size() == 2);
    CHECK(tx[0].size() == 3);
    CHECK(tx[0] == csv::Row({"a", "1", "3"}));
    CHECK(tx[1] == csv::Row({"b", "2", "4"}));
  }

  // single row → N×1 columns
  {
    csv::Table t = {{"x", "y", "z"}};
    auto tx = csv::transpose(t);
    CHECK(tx.size() == 3);
    CHECK(tx[0] == csv::Row({"x"}));
  }

  // single column → 1×N row
  {
    csv::Table t = {{"a"}, {"1"}, {"2"}, {"3"}};
    auto tx = csv::transpose(t);
    CHECK(tx.size() == 1);
    CHECK(tx[0].size() == 4);
  }

  // double-transpose is identity
  {
    csv::Table t = {{"a", "b", "c"}, {"1", "2", "3"}, {"4", "5", "6"}};
    CHECK(csv::tables_equal(csv::transpose(csv::transpose(t)), t));
  }

  // ragged rows padded with empty string
  {
    csv::Table t = {{"a", "b", "c"}, {"1", "2"}}; // second row has only 2 fields
    auto tx = csv::transpose(t);
    CHECK(tx.size() == 3);
    CHECK(tx[2][1].empty()); // padded
  }

  // empty table
  CHECK(csv::transpose(csv::Table{}).empty());
}

// ── 31. vstack / vstack_skip_header ───────────────────────────────────────
static void test_vstack()
{
  SUITE("vstack / vstack_skip_header");

  csv::Table ta = csv::parse("name,score\nAlice,88\nBob,74\n");
  csv::Table tb = csv::parse("name,score\nCarol,92\nDave,65\n");

  // vstack keeps both headers
  {
    auto s = csv::vstack(ta, tb);
    CHECK(s.size() == 6);
    CHECK(s[3] == csv::Row({"name", "score"})); // second header visible
  }

  // vstack_skip_header drops tb header
  {
    auto s = csv::vstack_skip_header(ta, tb);
    CHECK(s.size() == 5);
    CHECK(s[0] == csv::Row({"name", "score"})); // only ta header
    CHECK(s[3][0] == "Carol");
    CHECK(s[4][0] == "Dave");
  }

  // vstack_skip_header with empty b
  {
    auto s = csv::vstack_skip_header(ta, csv::Table{});
    CHECK(csv::tables_equal(s, ta));
  }

  // vstack_skip_header with header-only b
  {
    csv::Table header_only = {{"name", "score"}};
    auto s = csv::vstack_skip_header(ta, header_only);
    CHECK(csv::tables_equal(s, ta));
  }

  // three-way merge
  {
    csv::Table tc = csv::parse("name,score\nEve,81\n");
    auto s = csv::vstack_skip_header(
        csv::vstack_skip_header(ta, tb), tc);
    CHECK(s.size() == 6); // header + Alice + Bob + Carol + Dave + Eve
  }
}

// ── 32. tables_equal ──────────────────────────────────────────────────────
static void test_tables_equal()
{
  SUITE("tables_equal");

  csv::Table a = {{"x", "y"}, {"1", "2"}};
  csv::Table b = {{"x", "y"}, {"1", "2"}};
  csv::Table c = {{"x", "y"}, {"1", "3"}};

  CHECK(csv::tables_equal(a, b));
  CHECK(!csv::tables_equal(a, c));
  CHECK(csv::tables_equal(csv::Table{}, csv::Table{}));
  CHECK(!csv::tables_equal(a, csv::Table{}));
}

// ── 33. sniff ─────────────────────────────────────────────────────────────
static void test_sniff()
{
  SUITE("sniff – dialect detection");

  // comma separator
  {
    auto d = csv::sniff("name,age\nAlice,30\nBob,25\n");
    CHECK(d.separator == ',');
    CHECK(d.has_header);
  }

  // semicolon separator
  {
    auto d = csv::sniff("name;age;city\nAlice;30;NYC\nBob;25;LA\n");
    CHECK(d.separator == ';');
    CHECK(d.has_header);
  }

  // tab separator
  {
    auto d = csv::sniff("a\tb\tc\n1\t2\t3\n4\t5\t6\n");
    CHECK(d.separator == '\t');
    CHECK(d.has_header);
  }

  // pipe separator
  {
    auto d = csv::sniff("a|b|c\n1|2|3\n4|5|6\n");
    CHECK(d.separator == '|');
  }

  // CRLF detected
  {
    auto d = csv::sniff("a,b\r\n1,2\r\n3,4\r\n");
    CHECK(d.line_ending == "\r\n");
  }

  // LF detected
  {
    auto d = csv::sniff("a,b\n1,2\n3,4\n");
    CHECK(d.line_ending == "\n");
  }

  // all-numeric → no header
  {
    auto d = csv::sniff("1,2,3\n4,5,6\n7,8,9\n");
    CHECK(!d.has_header);
  }

  // empty sample → doesn't crash
  NO_THROW(csv::sniff(""));
}

// ── 34. describe / version ────────────────────────────────────────────────
static void test_meta()
{
  SUITE("describe / version");

  csv::Table t = csv::parse("name,age\nAlice,30\nBob,25\n");

  // describe mentions row count
  {
    std::string d = csv::describe(t);
    CHECK(d.find("3") != std::string::npos); // "3 row(s)"
  }

  // describe with max_rows=1
  {
    csv::Table big = csv::parse("a,b\n1,2\n3,4\n5,6\n7,8\n9,10\n");
    std::string d = csv::describe(big, 1);
    CHECK(d.find("not shown") != std::string::npos);
  }

  // describe of empty table doesn't crash
  NO_THROW(csv::describe(csv::Table{}));

  // version
  {
    std::string v = csv::version();
    CHECK(!v.empty());
    // Must contain at least two dots (major.minor.patch)
    CHECK(std::count(v.begin(), v.end(), '.') == 2);
  }
}

// ── 35. performance smoke-test: 100k rows ─────────────────────────────────
static void test_large_file()
{
  SUITE("performance – 100 000 rows");

  constexpr std::size_t ROWS = 100'000;
  constexpr std::size_t COLS = 6;

  std::string big;
  big.reserve(ROWS * COLS * 6);
  for (std::size_t r = 0; r < ROWS; ++r)
  {
    for (std::size_t c = 0; c < COLS; ++c)
    {
      if (c)
        big.push_back(',');
      big += std::to_string(r * COLS + c);
    }
    big.push_back('\n');
  }

  csv::Table t = csv::parse(big);
  CHECK(t.size() == ROWS);
  CHECK(t[0].size() == COLS);
  CHECK(t[ROWS - 1][COLS - 1] == std::to_string((ROWS - 1) * COLS + (COLS - 1)));

  // round-trip at scale
  std::string out = csv::write(t);
  csv::Table t2 = csv::parse(out);
  CHECK_MSG(csv::tables_equal(t, t2), "100k-row round-trip");

  // streaming parse must visit every row
  {
    std::size_t count = 0;
    csv::StreamingParser p([&](const csv::Row &)
                           { ++count; });
    p.parse(big);
    CHECK(count == ROWS);
  }
}

// ── 36. edge cases ────────────────────────────────────────────────────────
static void test_edge_cases()
{
  SUITE("edge cases");

  // single separator → two empty fields
  {
    auto t = P(",");
    CHECK(t.size() == 1 && t[0].size() == 2);
    CHECK(t[0][0].empty() && t[0][1].empty());
  }

  // quoted field that is only separators
  {
    auto t = P(R"(",,,")");
    CHECK(t[0][0] == ",,,");
  }

  // field with only whitespace (trim off)
  {
    csv::Options o;
    o.trim_whitespace = true;
    auto t = P("   ,\t,  ", o);
    CHECK(t[0][0].empty() && t[0][1].empty() && t[0][2].empty());
  }

  // deeply nested escaped quotes
  // 10 quote chars: 1 opening + 4×"" pairs + 1 closing → field is """"
  {
    auto t = P(R"("""""""""")");
    CHECK(t[0][0] == "\"\"\"\"");
    CHECK(t[0][0].size() == 4);
  }

  // all separators (no data)
  {
    auto t = P(",,,,");
    CHECK(t[0].size() == 5);
  }

  // unicode / UTF-8 passthrough (bytes treated opaquely)
  {
    // "café" in UTF-8: c a f \xc3 \xa9
    const std::string utf8 = "caf\xc3\xa9,\xe4\xb8\xad\xe6\x96\x87\n";
    auto t = P(utf8);
    CHECK(t.size() == 1 && t[0].size() == 2);
    CHECK(t[0][0] == "caf\xc3\xa9");
  }

  // quoted field directly followed by newline (no separator after)
  {
    auto t = P("\"only\"\n");
    CHECK(t.size() == 1 && t[0][0] == "only");
  }

  // multiple sequential newlines without skip_empty_lines
  {
    auto t = P("a\n\n\nb\n");
    CHECK(t.size() == 4);
  }

  // first line is blank with skip_empty_lines
  {
    csv::Options o;
    o.skip_empty_lines = true;
    auto t = P("\n\na,b\n1,2\n", o);
    CHECK(t.size() == 2);
  }

  // separator == quote_char is legal (edge configuration)
  {
    csv::Options o;
    o.separator = '"';
    o.quote_char = '\'';
    auto t = P("a\"b\"c", o);
    CHECK(t[0].size() == 3);
    CHECK(t[0][1] == "b");
  }

  // write then parse: field that is empty string
  {
    csv::Table t = {{"a", ""}, {"", "b"}};
    auto rt = csv::parse(csv::write(t));
    CHECK(csv::tables_equal(t, rt));
  }

  // write_row on empty Row
  {
    CHECK(csv::write_row({}) == "");
  }
}

// ╔══════════════════════════════════════════════════════════════════════════╗
// ║  main                                                                   ║
// ╚══════════════════════════════════════════════════════════════════════════╝

int main()
{
  std::cout << "csv.hpp – test suite\n";
  std::cout << "════════════════════════════════════════════\n";

  test_parse_error();
  test_parse_basic();
  test_parse_line_endings();
  test_parse_quoted();
  test_parse_malformed();
  test_options_separator();
  test_options_trim();
  test_options_skip_empty();
  test_options_comments();
  test_options_max_field_size();
  test_options_max_fields();
  test_options_field_transformer();
  test_options_row_filter();
  test_parse_istream();
  test_write_row();
  test_write_table();
  test_write_to();
  test_roundtrip();
  test_file_io();
  test_row_view();
  test_dict_reader();
  test_dict_writer();
  test_streaming_parser();
  test_rows_range();
  test_column_index();
  test_column_extract();
  test_select_columns();
  test_filter_rows();
  test_transform_fields();
  test_transpose();
  test_vstack();
  test_tables_equal();
  test_sniff();
  test_meta();
  test_large_file();
  test_edge_cases();

  harness::summary();
  return harness::failed > 0 ? 1 : 0;
}
