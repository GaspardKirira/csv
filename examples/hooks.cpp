/**
 * @file hooks.cpp
 * @brief Demonstrates Options::field_transformer and Options::row_filter:
 *        transform fields in-flight during parsing and selectively discard rows.
 */

#include <csv/csv.hpp>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>

static void to_upper(std::string &s)
{
  for (char &c : s)
    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
}

int main()
{
  const std::string raw =
      "name,department,salary,active\n"
      "  alice ,Engineering,95000,yes\n"
      "  bob   ,Marketing  ,72000,no\n"
      "  carol ,Engineering,88000,yes\n"
      "  dave  ,HR         ,61000,no\n"
      "  eve   ,Engineering,91000,yes\n";

  // 1. field_transformer: trim spaces + upper-case every field
  std::cout << "=== field_transformer: trim + upper-case ===\n";
  {
    csv::Options opt;
    opt.trim_whitespace = true;
    opt.field_transformer = [](std::string &s)
    { to_upper(s); };

    csv::Table t = csv::parse(raw, opt);
    for (const csv::Row &row : t)
    {
      for (std::size_t i = 0; i < row.size(); ++i)
      {
        if (i)
          std::cout << " | ";
        std::cout << row[i];
      }
      std::cout << '\n';
    }
  }

  // 2. row_filter: keep only Engineering rows that are active
  std::cout << "\n=== row_filter: Engineering + active only ===\n";
  {
    bool header_passed = false;

    csv::Options opt;
    opt.trim_whitespace = true;
    opt.row_filter = [&](const csv::Row &row) -> bool
    {
      if (!header_passed)
      {
        header_passed = true;
        return true;
      } // header
      return row.size() >= 4 &&
             row[1] == "Engineering" &&
             row[3] == "yes";
    };

    csv::Table t = csv::parse(raw, opt);
    std::cout << "Rows kept (header + matches): " << t.size() << '\n';
    for (const csv::Row &row : t)
    {
      for (std::size_t i = 0; i < row.size(); ++i)
      {
        if (i)
          std::cout << " | ";
        std::cout << row[i];
      }
      std::cout << '\n';
    }
  }

  // 3. Combining both hooks: normalise + filter
  std::cout << "\n=== combined: normalise names + keep salary >= 88000 ===\n";
  {
    // Capitalise first letter of each field word (title-case names only)
    auto title_case = [](std::string &s)
    {
      bool cap_next = true;
      for (char &c : s)
      {
        if (c == ' ')
        {
          cap_next = true;
          continue;
        }
        if (cap_next)
        {
          c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
          cap_next = false;
        }
      }
    };

    bool header_done = false;
    csv::Options opt;
    opt.trim_whitespace = true;
    opt.field_transformer = title_case;
    opt.row_filter = [&](const csv::Row &row) -> bool
    {
      if (!header_done)
      {
        header_done = true;
        return true;
      }
      if (row.size() < 3)
        return false;
      try
      {
        return std::stoi(row[2]) >= 88000;
      }
      catch (...)
      {
        return false;
      }
    };

    csv::Table t = csv::parse(raw, opt);
    for (const csv::Row &row : t)
    {
      for (std::size_t i = 0; i < row.size(); ++i)
      {
        if (i)
          std::cout << " | ";
        std::cout << row[i];
      }
      std::cout << '\n';
    }
  }

  return 0;
}
