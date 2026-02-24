#include <csv/csv.hpp>

#include <iostream>

int main()
{
  csv::Table table = {
      {"id", "name", "note"},
      {"1", "Alice", "hello, world"},
      {"2", "Bob", "he said \"yo\""},
      {"3", "Carol", "line1\nline2"}};

  const std::string out = csv::write(table);

  std::cout << out << "\n";
  return 0;
}
