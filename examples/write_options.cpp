/**
 * @file write_options.cpp
 * @brief Demonstrates csv::WriteOptions: CRLF line endings, always_quote,
 *        custom separators, write_to() streaming output, and proper escaping.
 */

#include <csv/csv.hpp>
#include <iostream>
#include <sstream>

static void hex_dump_endings(const std::string &s, const std::string &label)
{
  std::cout << label << "  last 10 bytes: ";
  const std::size_t start = s.size() > 10 ? s.size() - 10 : 0;
  for (std::size_t i = start; i < s.size(); ++i)
  {
    const unsigned char c = static_cast<unsigned char>(s[i]);
    if (c == '\r')
      std::cout << "<CR>";
    else if (c == '\n')
      std::cout << "<LF>";
    else
      std::cout << c;
  }
  std::cout << '\n';
}

int main()
{
  csv::Table data = {
      {"name", "description", "price"},
      {"Mocha", "Rich, bold espresso", "3.50"},
      {"Latte", "Milk & foam, \"smooth\"", "4.00"},
      {"Tea", "Simple\nherbal blend", "2.50"},
  };

  //  Default: LF, quote only when necessary
  std::cout << "=== Default (LF, minimal quoting) ===\n";
  const std::string lf = csv::write(data);
  std::cout << lf;
  hex_dump_endings(lf, "");

  // CRLF (RFC 4180 / Windows)
  std::cout << "\n=== CRLF line endings ===\n";
  {
    csv::WriteOptions wo;
    wo.line_ending = "\r\n";
    const std::string crlf = csv::write(data, wo);
    hex_dump_endings(crlf, "");
    // Verify parseable
    csv::Table reloaded = csv::parse(crlf);
    std::cout << "Re-parsed rows: " << reloaded.size()
              << "  match=" << std::boolalpha
              << csv::tables_equal(data, reloaded) << '\n';
  }

  // always_quote: every field is wrapped regardless of content
  std::cout << "\n=== always_quote ===\n";
  {
    csv::WriteOptions wo;
    wo.always_quote = true;
    std::cout << csv::write(data, wo);
  }

  // Semicolon separator (European / Excel)
  std::cout << "\n=== Semicolon separator ===\n";
  {
    csv::WriteOptions wo;
    wo.separator = ';';
    std::cout << csv::write(data, wo);
  }

  // write_to() — stream directly without building a full string
  std::cout << "\n=== write_to() streaming output ===\n";
  {
    csv::WriteOptions wo;
    wo.line_ending = "\r\n";
    std::ostringstream oss;
    csv::write_to(oss, data, wo);
    std::cout << "Bytes streamed: " << oss.str().size() << '\n';

    // Parse what we streamed
    std::istringstream iss(oss.str());
    csv::Table rt = csv::parse(iss);
    std::cout << "Round-trip match: "
              << std::boolalpha << csv::tables_equal(data, rt) << '\n';
  }

  // write_row(): serialise individual rows
  std::cout << "\n=== write_row() ===\n";
  for (const csv::Row &row : data)
    std::cout << csv::write_row(row) << '\n';

  return 0;
}
