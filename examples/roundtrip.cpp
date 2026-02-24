#include <csv/csv.hpp>

#include <iostream>

int main()
{
  const std::string input =
      "a,b,c\n"
      "1,\"hello, world\",\"he said \"\"yo\"\"\"\n"
      "2,\"line1\nline2\",ok\n";

  const auto table = csv::parse(input);
  const std::string out = csv::write(table);
  const auto back = csv::parse(out);

  std::cout << "rows in : " << table.size() << "\n";
  std::cout << "rows out: " << back.size() << "\n";
  std::cout << "ok\n";
  return 0;
}
