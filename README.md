# csv

Minimal CSV parser and writer for modern C++.

`csv` provides strict and deterministic CSV parsing and writing with
support for quoted fields.

Header-only. Zero dependencies.

## Download

https://vixcpp.com/registry/pkg/gaspardkirira/csv

## Why csv?

CSV is everywhere:

-   Data import/export
-   Reporting systems
-   CLI tools
-   Analytics pipelines
-   Configuration files
-   Lightweight storage

This library provides:

-   Comma-separated parsing
-   Quoted field support (`"`)
-   Escaped quotes (`""`)
-   Commas inside quoted fields
-   Newlines inside quoted fields
-   Strict malformed input detection

Minimal. Predictable. RFC-style behavior.

## Installation

### Using Vix Registry

``` bash
vix add gaspardkirira/csv
vix deps
```

### Manual

``` bash
git clone https://github.com/GaspardKirira/csv.git
```

Add the `include/` directory to your project.

## Quick Examples

### Parse CSV

``` cpp
#include <csv/csv.hpp>
#include <iostream>

int main()
{
  const std::string input =
    "name,age\n"
    "Alice,20\n"
    "Bob,25\n";

  auto table = csv::parse(input);

  std::cout << table[1][0] << "\n"; // Alice
}
```

### Quoted Fields

``` cpp
#include <csv/csv.hpp>
#include <iostream>

int main()
{
  const std::string input =
    "name,desc\n"
    "Alice,\"hello, world\"\n";

  auto table = csv::parse(input);

  std::cout << table[1][1] << "\n"; // hello, world
}
```

Supports:

-   Commas inside quotes
-   Escaped quotes (`""`)
-   Newlines inside quotes

### Write CSV

``` cpp
#include <csv/csv.hpp>
#include <iostream>

int main()
{
  csv::Table table = {
    {"id", "name"},
    {"1", "Alice"},
    {"2", "Bob"}
  };

  std::string out = csv::write(table);

  std::cout << out << "\n";
}
```

Fields are automatically quoted when needed.

## API Overview

``` cpp
csv::parse(text);
csv::write(table);
csv::write_row(fields);
```

### Types

``` cpp
using csv::Row = std::vector<std::string>;
using csv::Table = std::vector<Row>;
```

## Error Handling

`csv::parse()` throws `std::invalid_argument` if:

-   A quoted field is not closed
-   The CSV is malformed

Strict behavior by design.

## Technical Details

-   Supports LF and CRLF line endings
-   No global state
-   No dynamic allocation beyond output containers
-   Deterministic quoting rules
-   Always uses `,` as separator

## Tests

Run:

``` bash
vix build
vix tests
```

Tests verify:

-   Basic parsing
-   Quoted fields
-   Escaped quotes
-   Roundtrip integrity
-   Malformed input detection

## Design Philosophy

`csv` focuses on:

-   Minimal surface area
-   RFC-style correctness
-   Predictable quoting
-   No configuration complexity
-   Clean integration with modern C++ systems

Works well alongside:

-   `hex`
-   `base64`
-   `endian`
-   `secure_random`
-   `uuid`
-   `hashing`
-   `hmac`

## License

MIT License\
Copyright (c) Gaspard Kirira

