/**
 * @file file_io.cpp
 * @brief Demonstrates csv::load() and csv::save() for reading and writing
 *        CSV files on disk, including CRLF output and error handling.
 */

#include <csv/csv.hpp>
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;

int main()
{
  const fs::path tmp_dir = fs::temp_directory_path();
  const fs::path lf_path = tmp_dir / "employees_lf.csv";
  const fs::path crlf_path = tmp_dir / "employees_crlf.csv";

  // Build a sample table
  csv::Table employees = {
      {"id", "name", "department", "salary"},
      {"1", "Alice Dupont", "Engineering", "95000"},
      {"2", "Bob O'Brien", "Marketing", "72000"},
      {"3", "Carol \"CC\" Chen", "Engineering", "88000"}, // embedded quotes
      {"4", "Dave, Jr.", "HR", "61000"},                  // embedded comma
  };

  // Save with LF endings (Unix default)
  csv::save(lf_path.string(), employees);
  std::cout << "Saved (LF)  : " << lf_path << '\n';

  // Save with CRLF endings (Windows / RFC 4180)
  csv::WriteOptions wo;
  wo.line_ending = "\r\n";
  csv::save(crlf_path.string(), employees, wo);
  std::cout << "Saved (CRLF): " << crlf_path << '\n';

  // Reload and verify round-trip
  const csv::Table reloaded = csv::load(lf_path.string());

  std::cout << "\nReloaded " << reloaded.size() << " rows:\n";
  for (const csv::Row &row : reloaded)
  {
    for (std::size_t i = 0; i < row.size(); ++i)
    {
      if (i)
        std::cout << " | ";
      std::cout << row[i];
    }
    std::cout << '\n';
  }

  if (!csv::tables_equal(employees, reloaded))
    std::cerr << "MISMATCH after round-trip!\n";
  else
    std::cout << "\nRound-trip verified OK.\n";

  // Error handling: non-existent file
  std::cout << "\n--- Error handling ---\n";
  try
  {
    csv::load("/no/such/file.csv");
  }
  catch (const std::ios_base::failure &e)
  {
    std::cout << "Caught expected error: " << e.what() << '\n';
  }

  // Error handling: malformed CSV
  const fs::path bad_path = tmp_dir / "bad.csv";
  {
    std::ofstream f(bad_path.string());
    f << "a,b,c\n1,\"unclosed\n";
  }
  try
  {
    csv::load(bad_path.string());
  }
  catch (const csv::ParseError &e)
  {
    std::cout << "Caught ParseError: " << e.what() << '\n';
    std::cout << "  line=" << e.line() << "  col=" << e.col() << '\n';
  }

  // Clean up temp files
  fs::remove(lf_path);
  fs::remove(crlf_path);
  fs::remove(bad_path);

  return 0;
}
