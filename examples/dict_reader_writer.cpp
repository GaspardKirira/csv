/**
 * @file dict_reader_writer.cpp
 * @brief Demonstrates csv::DictReader (named-column row access) and
 *        csv::DictWriter (writing rows as key-value pairs).
 */

#include <csv/csv.hpp>
#include <iostream>
#include <string>

int main()
{
  // DictReader
  const std::string raw =
      "name,department,salary\n"
      "Alice,Engineering,95000\n"
      "Bob,Marketing,72000\n"
      "Carol,Engineering,88000\n"
      "Dave,HR,61000\n";

  csv::Table table = csv::parse(raw);
  csv::DictReader reader(table); // non-owning: table stays in scope

  std::cout << "=== DictReader ===\n";
  std::cout << "Columns:";
  for (const std::string &col : reader.fieldnames())
    std::cout << "  " << col;
  std::cout << "\n\n";

  double total_salary = 0.0;
  for (const csv::RowView &row : reader)
  {
    const std::string name = std::string(row["name"]);
    const std::string dept = std::string(row["department"]);
    const double salary = std::stod(std::string(row["salary"]));
    total_salary += salary;

    std::cout << "  " << name
              << "  (" << dept << ")"
              << "  $" << salary << '\n';
  }
  std::cout << "\n  Total payroll: $" << total_salary << '\n';

  // DictWriter
  std::cout << "\n=== DictWriter ===\n";

  // Column order is fixed; missing keys produce empty fields.
  csv::DictWriter writer({"id", "product", "qty", "unit_price"});

  writer.writerow({{"id", "1"}, {"product", "Widget A"}, {"qty", "100"}, {"unit_price", "2.50"}});
  writer.writerow({{"id", "2"}, {"product", "Widget B"}, {"qty", "250"}}); // unit_price omitted
  writer.writerow({{"unit_price", "9.99"}, {"product", "Gadget"}, {"id", "3"}, {"qty", "10"}});

  std::cout << writer.str();

  // Round-trip: write → parse → DictReader
  std::cout << "\n=== round-trip through DictWriter CSV ===\n";

  csv::Table t2 = csv::parse(writer.str());
  csv::DictReader r2(std::move(t2)); // owning: r2 takes the table

  for (const csv::RowView &row : r2)
  {
    std::cout << "  product=" << row["product"]
              << "  qty=" << row["qty"]
              << "  price=" << row["unit_price"] << '\n';
  }

  return 0;
}
