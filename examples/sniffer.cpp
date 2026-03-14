/**
 * @file sniffer.cpp
 * @brief Demonstrates csv::sniff(): automatic dialect detection for unknown
 *        CSV files (separator, line ending, header presence).
 *
 */

#include <csv/csv.hpp>
#include <iostream>

static void detect_and_parse(const std::string &label, const std::string &text)
{
  std::cout << "=== " << label << " ===\n";

  csv::Dialect d = csv::sniff(text);

  std::cout << "  separator   : '" << d.separator << "'\n";
  std::cout << "  line_ending : "
            << (d.line_ending == "\r\n" ? "CRLF" : "LF") << '\n';
  std::cout << "  has_header  : " << std::boolalpha << d.has_header << '\n';

  // Build Options from the sniffed dialect
  csv::Options opt;
  opt.separator = d.separator;
  opt.skip_empty_lines = true;

  csv::Table t = csv::parse(text, opt);
  std::cout << "  rows parsed : " << t.size() << '\n';

  // Print first two rows
  for (std::size_t r = 0; r < std::min<std::size_t>(2, t.size()); ++r)
  {
    std::cout << "  [" << r << "]";
    for (const std::string &f : t[r])
      std::cout << "  \"" << f << '"';
    std::cout << '\n';
  }
  std::cout << '\n';
}

int main()
{
  // Comma-separated with header
  detect_and_parse(
      "Comma-separated (header)",
      "name,age,city\nAlice,30,Paris\nBob,25,Berlin\n");

  // Semicolon-separated with header
  detect_and_parse(
      "Semicolon-separated (header)",
      "name;age;city\nAlice;30;Paris\nBob;25;Berlin\n");

  // Tab-separated with header
  detect_and_parse(
      "Tab-separated (header)",
      "name\tage\tcity\nAlice\t30\tParis\nBob\t25\tBerlin\n");

  // Pipe-separated, no header (all numeric)
  detect_and_parse(
      "Pipe-separated (no header – all data)",
      "1|100|200\n2|300|400\n3|500|600\n");

  // CRLF line endings
  detect_and_parse(
      "CRLF line endings",
      "id,value\r\n1,alpha\r\n2,beta\r\n3,gamma\r\n");

  // Mixed: European decimal comma in quoted field, semicolon separator
  detect_and_parse(
      "European decimal format (semicolon sep, quoted comma)",
      "artikel;preis;menge\n"
      "Widget;\"1,99\";100\n"
      "Gadget;\"9,50\";25\n");

  return 0;
}
