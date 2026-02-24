#include <csv/csv.hpp>

#include <iostream>

static void print_table(const csv::Table &t)
{
  for (std::size_t r = 0; r < t.size(); ++r)
  {
    std::cout << "row " << r << ":\n";
    for (std::size_t c = 0; c < t[r].size(); ++c)
      std::cout << "  [" << c << "] " << t[r][c] << "\n";
  }
}

int main()
{
  const std::string input =
      "name,age,city\n"
      "Alice,20,Kampala\n"
      "Bob,25,Nairobi\n";

  const auto table = csv::parse(input);

  print_table(table);
  return 0;
}
