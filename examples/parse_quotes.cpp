#include <csv/csv.hpp>

#include <iostream>

int main()
{
  const std::string input =
      "name,desc\n"
      "Alice,\"hello, world\"\n"
      "Bob,\"line1\nline2\"\n"
      "Carol,\"he said \"\"yo\"\"\"\n";

  const auto table = csv::parse(input);

  std::cout << "rows: " << table.size() << "\n";
  std::cout << "Bob desc:\n"
            << table[2][1] << "\n";
  std::cout << "Carol desc:\n"
            << table[3][1] << "\n";
  return 0;
}
