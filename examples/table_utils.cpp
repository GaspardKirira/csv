/**
 * @file table_utils.cpp
 * @brief Demonstrates the table manipulation utilities:
 *        csv::filter_rows, csv::select_columns, csv::column,
 *        csv::transpose, csv::vstack, csv::vstack_skip_header,
 *        csv::transform_fields.
 */

#include <csv/csv.hpp>
#include <iostream>

static void print_table(const csv::Table &t, const std::string &label)
{
  std::cout << "--- " << label
            << "  (" << t.size() << " rows)\n";
  for (const csv::Row &row : t)
  {
    std::cout << "  ";
    for (std::size_t i = 0; i < row.size(); ++i)
    {
      if (i)
        std::cout << " | ";
      std::cout << row[i];
    }
    std::cout << '\n';
  }
  std::cout << '\n';
}

int main()
{
  const std::string raw =
      "name,country,year,score\n"
      "Alice,France,2021,88\n"
      "Bob,Germany,2022,74\n"
      "Carol,France,2023,92\n"
      "Dave,Spain,2021,65\n"
      "Eve,Germany,2022,81\n";

  csv::Table t = csv::parse(raw);
  print_table(t, "original");

  // filter_rows
  auto french = csv::filter_rows(t, [](const csv::Row &r)
                                 { return r.size() >= 2 && r[1] == "France"; });
  print_table(french, "filter: country == France");

  auto high = csv::filter_rows(t, [](const csv::Row &r)
                               {
        if (r.size() < 4) return false;
        try { return std::stoi(r[3]) >= 80; } catch (...) { return false; } });
  print_table(high, "filter: score >= 80");

  // select_columns
  auto sub = csv::select_columns(t, {"name", "score"});
  print_table(sub, "select_columns: name + score");

  // column
  auto scores = csv::column(t, "score");
  std::cout << "--- column 'score'  (" << scores.size() << " values)\n  ";
  for (const std::string &s : scores)
    std::cout << s << ' ';
  std::cout << "\n\n";

  // transform_fields
  auto upper = csv::transform_fields(sub, [](std::string &s)
                                     {
        for (char& c : s)
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c))); });
  print_table(upper, "transform_fields: upper-case");

  // transpose
  csv::Table small = csv::parse("a,b,c\n1,2,3\n4,5,6\n");
  csv::Table tx = csv::transpose(small);
  print_table(small, "before transpose");
  print_table(tx, "after  transpose");

  // vstack / vstack_skip_header
  const std::string part1 = "name,score\nAlice,88\nBob,74\n";
  const std::string part2 = "name,score\nCarol,92\nDave,65\n";

  csv::Table ta = csv::parse(part1);
  csv::Table tb = csv::parse(part2);

  csv::Table stacked = csv::vstack_skip_header(ta, tb);
  print_table(stacked, "vstack_skip_header (two CSVs, one header)");

  return 0;
}
