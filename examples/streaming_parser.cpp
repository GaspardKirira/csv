/**
 * @file streaming_parser.cpp
 * @brief Demonstrates csv::StreamingParser: process rows one-by-one via a
 *        callback without accumulating the entire Table in memory.
 *
 *        Useful for very large files (hundreds of millions of rows) where
 *        materialising a full csv::Table would exhaust RAM.
 */

#include <csv/csv.hpp>
#include <iostream>
#include <sstream>
#include <string>

// Generate a synthetic large CSV in-memory for the demo
static std::string make_large_csv(std::size_t rows)
{
  std::string s;
  s.reserve(rows * 30);
  s += "id,value,category\n";
  for (std::size_t i = 1; i <= rows; ++i)
  {
    s += std::to_string(i);
    s += ',';
    s += std::to_string(i * 3);
    s += ',';
    s += (i % 2 == 0) ? "even" : "odd";
    s += '\n';
  }
  return s;
}

int main()
{
  constexpr std::size_t TOTAL_ROWS = 1'000'000;
  std::cout << "Generating " << TOTAL_ROWS << " data rows…\n";

  const std::string csv_data = make_large_csv(TOTAL_ROWS);

  // Stream-parse: count rows and accumulate column sums
  std::size_t row_count = 0;
  std::size_t even_count = 0;
  std::size_t odd_count = 0;
  bool header_seen = false;

  csv::StreamingParser parser(
      [&](const csv::Row &row)
      {
        if (!header_seen)
        {
          header_seen = true; // skip the header row
          return;
        }
        ++row_count;
        if (row.size() >= 3)
        {
          if (row[2] == "even")
            ++even_count;
          else
            ++odd_count;
        }
      });

  std::istringstream ss(csv_data);
  parser.parse(ss);

  std::cout << "Rows processed : " << row_count << '\n';
  std::cout << "Even rows      : " << even_count << '\n';
  std::cout << "Odd  rows      : " << odd_count << '\n';

  // Incremental push() — useful when reading from a socket chunk
  std::cout << "\n--- Incremental push() demo ---\n";

  std::size_t short_count = 0;
  csv::StreamingParser p2([&](const csv::Row &)
                          { ++short_count; });

  const std::string chunk1 = "a,b,c\n1,2,";
  const std::string chunk2 = "3\n4,5,6\n";

  p2.push(chunk1.data(), chunk1.size());
  p2.push(chunk2.data(), chunk2.size());
  p2.finish();

  std::cout << "Rows from two chunks: " << short_count << " (expected 3)\n";

  // Reset and reuse
  p2.reset();
  std::size_t after_reset = 0;
  // Assign a new counter; the existing lambda still captures short_count.
  // Easier: just use another parser.
  csv::StreamingParser p3([&](const csv::Row &)
                          { ++after_reset; });
  p3.parse(std::string("x,y\n1,2\n3,4\n5,6\n"));
  std::cout << "Rows after reset     : " << after_reset << " (expected 3)\n";

  return 0;
}
