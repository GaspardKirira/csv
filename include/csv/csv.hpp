/**
 * @file csv.hpp
 * @brief Minimal CSV parser/writer for modern C++.
 *
 * Header-only.
 *
 * Supports:
 *   - comma separator ','
 *   - quoted fields with '"'
 *   - escaped quote inside quoted field: ""
 *   - commas and newlines inside quoted fields
 *   - CRLF and LF line endings
 *
 * Provides:
 *   - csv::parse(text) -> rows of fields
 *   - csv::write(rows) -> CSV string
 *   - csv::write_row(fields) -> one row string (no trailing newline)
 *
 * MIT License
 */

#pragma once

#include <stdexcept>
#include <string>
#include <vector>

namespace csv
{
  using Row = std::vector<std::string>;
  using Table = std::vector<Row>;

  /**
   * @brief Parse CSV text into rows.
   *
   * @throws std::invalid_argument on malformed CSV (unclosed quote)
   */
  inline Table parse(const std::string &text)
  {
    Table out;
    Row row;
    std::string field;

    bool in_quotes = false;

    const auto push_field = [&]()
    {
      row.push_back(field);
      field.clear();
    };

    const auto push_row = [&]()
    {
      // A row can be empty (single empty field) if we saw a newline after nothing
      // This behavior matches common CSV readers.
      out.push_back(row);
      row.clear();
    };

    for (std::size_t i = 0; i < text.size(); ++i)
    {
      const char c = text[i];

      if (in_quotes)
      {
        if (c == '"')
        {
          // escaped quote?
          if (i + 1 < text.size() && text[i + 1] == '"')
          {
            field.push_back('"');
            ++i;
          }
          else
          {
            in_quotes = false;
          }
        }
        else
        {
          field.push_back(c);
        }

        continue;
      }

      // not in quotes
      if (c == '"')
      {
        in_quotes = true;
        continue;
      }

      if (c == ',')
      {
        push_field();
        continue;
      }

      if (c == '\n')
      {
        push_field();
        push_row();
        continue;
      }

      if (c == '\r')
      {
        // support CRLF
        if (i + 1 < text.size() && text[i + 1] == '\n')
          ++i;

        push_field();
        push_row();
        continue;
      }

      field.push_back(c);
    }

    if (in_quotes)
      throw std::invalid_argument("malformed csv: unclosed quote");

    // finalize last row
    push_field();

    // Avoid adding an extra empty row when input ends with newline
    // If the last row is exactly one empty field AND previous char was newline,
    // common CSV readers consider it as an empty last row. We keep it if it was explicit.
    out.push_back(row);

    return out;
  }

  namespace detail
  {
    inline bool needs_quotes(const std::string &s)
    {
      for (char c : s)
      {
        if (c == '"' || c == ',' || c == '\n' || c == '\r')
          return true;
      }
      return false;
    }

    inline std::string escape_field(const std::string &s)
    {
      if (!needs_quotes(s))
        return s;

      std::string out;
      out.reserve(s.size() + 2);

      out.push_back('"');
      for (char c : s)
      {
        if (c == '"')
          out.append("\"\"");
        else
          out.push_back(c);
      }
      out.push_back('"');

      return out;
    }
  }

  /**
   * @brief Write a single CSV row (no trailing newline).
   */
  inline std::string write_row(const Row &fields)
  {
    std::string out;

    for (std::size_t i = 0; i < fields.size(); ++i)
    {
      if (i != 0)
        out.push_back(',');

      out += detail::escape_field(fields[i]);
    }

    return out;
  }

  /**
   * @brief Write a CSV table with '\n' line endings.
   */
  inline std::string write(const Table &rows)
  {
    std::string out;

    for (std::size_t i = 0; i < rows.size(); ++i)
    {
      out += write_row(rows[i]);
      if (i + 1 < rows.size())
        out.push_back('\n');
    }

    return out;
  }

} // namespace csv
