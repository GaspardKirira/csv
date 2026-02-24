#include <csv/csv.hpp>

#include <cstdlib>
#include <iostream>

static void expect_true(bool v, const char *msg)
{
  if (!v)
  {
    std::cerr << "FAIL: " << msg << "\n";
    std::exit(1);
  }
}

static void expect_eq(const std::string &a, const std::string &b, const char *msg)
{
  if (a != b)
  {
    std::cerr << "FAIL: " << msg << "\n";
    std::cerr << "  got : " << a << "\n";
    std::cerr << "  want: " << b << "\n";
    std::exit(1);
  }
}

int main()
{
  // Basic parse
  {
    const std::string input = "a,b,c\n1,2,3";
    const auto t = csv::parse(input);

    expect_true(t.size() == 2, "rows count");
    expect_true(t[0].size() == 3, "cols count header");
    expect_eq(t[0][0], "a", "cell 0,0");
    expect_eq(t[1][2], "3", "cell 1,2");
  }

  // Quotes and commas
  {
    const std::string input = "name,desc\nAlice,\"hello, world\"";
    const auto t = csv::parse(input);

    expect_eq(t[1][0], "Alice", "quoted row name");
    expect_eq(t[1][1], "hello, world", "quoted comma");
  }

  // Escaped quotes
  {
    const std::string input = "a\n\"he said \"\"yo\"\"\"";
    const auto t = csv::parse(input);

    expect_eq(t[1][0], "he said \"yo\"", "escaped quote");
  }

  // Write roundtrip
  {
    csv::Table table = {
        {"a", "b", "c"},
        {"1", "hello, world", "he said \"yo\""}};

    const std::string out = csv::write(table);
    const auto back = csv::parse(out);

    expect_true(back.size() == table.size(), "roundtrip rows");
    expect_eq(back[1][1], "hello, world", "roundtrip quoted comma");
    expect_eq(back[1][2], "he said \"yo\"", "roundtrip escaped quote");
  }

  // Malformed CSV: unclosed quote
  {
    bool threw = false;
    try
    {
      (void)csv::parse("a\n\"oops");
    }
    catch (...)
    {
      threw = true;
    }
    expect_true(threw, "unclosed quote throws");
  }

  std::cout << "ok\n";
  return 0;
}
