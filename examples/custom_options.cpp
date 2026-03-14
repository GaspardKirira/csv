/**
 * @file custom_options.cpp
 * @brief Demonstrates csv::Options: custom separator, whitespace trimming,
 *        skipping empty lines, and comment lines.
 */

#include <csv/csv.hpp>
#include <iostream>

int main()
{
  // Semicolon-separated with surrounding spaces
  const std::string tsv =
      "# country data – semicolon separated\n"
      "\n"
      "  country  ;  capital  ;  population_M  \n"
      "  France   ;  Paris    ;  68            \n"
      "  Germany  ;  Berlin   ;  84            \n"
      "  Spain    ;  Madrid   ;  47            \n"
      "\n"; // trailing blank line

  csv::Options opt;
  opt.separator = ';';
  opt.trim_whitespace = true;  // strip spaces around each field
  opt.skip_empty_lines = true; // discard blank lines
  opt.skip_comments = true;    // discard lines starting with '#'
  opt.comment_char = '#';

  csv::Table t = csv::parse(tsv, opt);

  std::cout << "Rows parsed (header + data): " << t.size() << "\n\n";

  // Print with aligned columns
  for (const csv::Row &row : t)
  {
    for (std::size_t i = 0; i < row.size(); ++i)
    {
      if (i)
        std::cout << " | ";
      std::cout.width(14);
      std::cout << std::left << row[i];
    }
    std::cout << '\n';
  }

  // Tab-separated values
  std::cout << "\n--- TSV example ---\n";

  const std::string tsv2 = "id\tname\tvalue\n1\talpha\t1.1\n2\tbeta\t2.2\n";

  csv::Options tsv_opt;
  tsv_opt.separator = '\t';

  csv::Table t2 = csv::parse(tsv2, tsv_opt);
  for (const csv::Row &row : t2)
  {
    for (std::size_t i = 0; i < row.size(); ++i)
    {
      if (i)
        std::cout << '\t';
      std::cout << row[i];
    }
    std::cout << '\n';
  }

  return 0;
}
