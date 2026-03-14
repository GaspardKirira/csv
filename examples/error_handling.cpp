/**
 * @file error_handling.cpp
 * @brief Demonstrates error handling with csv::ParseError:
 *        unclosed quotes, illegal bytes after closing quotes,
 *        field/row size limits, and distinguishing error kinds.
 */

#include <csv/csv.hpp>
#include <iostream>

// Attempt a parse and print the result or error
static void try_parse(const std::string &label, const std::string &text,
                      const csv::Options &opt = {})
{
  std::cout << "--- " << label << " ---\n";
  std::cout << "  input: " << text << '\n';
  try
  {
    csv::Table t = csv::parse(text, opt);
    std::cout << "  OK: " << t.size() << " row(s)\n";
    for (const csv::Row &row : t)
    {
      std::cout << "    [";
      for (std::size_t i = 0; i < row.size(); ++i)
      {
        if (i)
          std::cout << ", ";
        std::cout << '"' << row[i] << '"';
      }
      std::cout << "]\n";
    }
  }
  catch (const csv::ParseError &e)
  {
    std::cout << "  ParseError  : " << e.what() << '\n';
    std::cout << "  message()   : " << e.message() << '\n';
    std::cout << "  line=" << e.line() << "  col=" << e.col() << '\n';
  }
  catch (const std::invalid_argument &e)
  {
    std::cout << "  invalid_argument: " << e.what() << '\n';
  }
  catch (const std::ios_base::failure &e)
  {
    std::cout << "  ios_base::failure: " << e.what() << '\n';
  }
  std::cout << '\n';
}

int main()
{
  // 1. Unclosed quote
  try_parse("unclosed quote",
            R"(name,age
"Alice,30
Bob,25)");

  // 2. Illegal byte after closing quote
  try_parse("byte after closing quote",
            R"("Alice"X,30)");

  // 3. Correct: doubled quote inside quoted field
  try_parse("doubled quote (valid)",
            R"("say ""hello""",world)");

  // 4. Correct: comma inside quoted field
  try_parse("comma inside quotes (valid)",
            R"("Smith, John",42)");

  // 5. Correct: newline inside quoted field
  try_parse("newline inside quotes (valid)",
            "\"line1\nline2\",end");

  // 6. Field size limit exceeded
  {
    csv::Options opt;
    opt.max_field_size = 5; // allow at most 5 bytes per field
    try_parse("field size limit (5 bytes)",
              "ok,toolong_field,x",
              opt);
  }

  // 7. Row width limit exceeded
  {
    csv::Options opt;
    opt.max_fields_per_row = 3;
    try_parse("row width limit (3 fields)",
              "a,b,c,d,e",
              opt);
  }

  // 8. ParseError is-a std::invalid_argument
  std::cout << "--- ParseError inherits from std::invalid_argument ---\n";
  try
  {
    csv::parse("\"bad");
  }
  catch (const std::invalid_argument &e)
  {
    std::cout << "  Caught via base class: " << e.what() << '\n';
    // Down-cast to access position info
    const auto *pe = dynamic_cast<const csv::ParseError *>(&e);
    if (pe)
      std::cout << "  (is ParseError)  line=" << pe->line() << '\n';
  }
  std::cout << '\n';

  // 9. Empty input → no error, empty table
  try_parse("empty input", "");

  // 10. All-empty lines with skip_empty_lines
  {
    csv::Options opt;
    opt.skip_empty_lines = true;
    try_parse("all-empty lines (skip_empty_lines=true)", "\n\n\n", opt);
  }

  return 0;
}
