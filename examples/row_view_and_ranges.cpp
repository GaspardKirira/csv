/**
 * @file row_view_and_ranges.cpp
 * @brief Demonstrates csv::rows(), csv::RowView, csv::DictReader iteration,
 *        csv::describe(), csv::column_index(), and csv::version().
 */

#include <csv/csv.hpp>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <string>

int main()
{
  std::cout << "csv.hpp version: " << csv::version() << "\n\n";

  const std::string raw =
      "name,country,year,score\n"
      "Alice,France,2021,88\n"
      "Bob,Germany,2022,74\n"
      "Carol,France,2023,92\n"
      "Dave,Spain,2021,65\n"
      "Eve,Germany,2022,81\n";

  csv::Table t = csv::parse(raw);

  // csv::describe() — quick human-readable summary
  std::cout << csv::describe(t, 3) << '\n';

  // csv::rows() — C++20 range, skips header automatically
  std::cout << "=== csv::rows() range-for ===\n";
  for (const csv::RowView &row : csv::rows(t))
  {
    std::cout << "  " << row["name"]
              << "  (" << row["country"] << ")"
              << "  score=" << row["score"] << '\n';
  }

  // RowView::at() — index-based access
  std::cout << "\n=== RowView::at() ===\n";
  {
    csv::RowView v(t[0], t[2]); // header = t[0], data = t[2]
    std::cout << "  name=" << v.at(0)
              << "  country=" << v.at(1)
              << "  score=" << v.at(3) << '\n';
  }

  // RowView::contains()
  std::cout << "\n=== RowView::contains() ===\n";
  {
    csv::RowView v(t[0], t[1]);
    std::cout << "  has 'score'   : " << std::boolalpha << v.contains("score") << '\n';
    std::cout << "  has 'missing' : " << std::boolalpha << v.contains("missing") << '\n';
  }

  // Iterate (header, value) pairs
  std::cout << "\n=== RowView iterator (header-value pairs) ===\n";
  {
    csv::RowView v(t[0], t[3]); // Dave's row
    for (const auto &[col, val] : v)
      std::cout << "  " << col << " = " << val << '\n';
  }

  // column_index() — look up column position
  std::cout << "\n=== column_index() ===\n";
  {
    const csv::Row &header = t[0];
    for (const std::string &col : {"name", "score", "missing"})
    {
      auto idx = csv::column_index(header, col);
      if (idx)
        std::cout << "  '" << col << "' is at index " << *idx << '\n';
      else
        std::cout << "  '" << col << "' not found\n";
    }
  }

  // Aggregate over csv::rows()
  std::cout << "\n=== aggregate: average score ===\n";
  {
    double sum = 0.0;
    std::size_t n = 0;
    for (const csv::RowView &row : csv::rows(t))
    {
      sum += std::stod(std::string(row["score"]));
      ++n;
    }
    std::cout << "  avg score = " << (n ? sum / n : 0.0) << '\n';
  }

  // DictReader on the same table
  std::cout << "\n=== DictReader: highest scorer ===\n";
  {
    csv::DictReader reader(t); // non-owning borrow
    std::string best_name;
    int best_score = -1;
    for (const csv::RowView &row : reader)
    {
      const int s = std::stoi(std::string(row["score"]));
      if (s > best_score)
      {
        best_score = s;
        best_name = std::string(row["name"]);
      }
    }
    std::cout << "  " << best_name << " with " << best_score << '\n';
  }

  return 0;
}
