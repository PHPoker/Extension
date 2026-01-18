<p align="center">
    <img src="https://github.com/PHPoker/Extension/blob/main/docs/logo-with-text.png?raw=true" height="300" alt="PHPoker Extension">
    <p align="center">
        <a href="https://github.com/PHPoker/Extension"><img alt="Total Downloads" src="https://img.shields.io/badge/version-1.0.5-blue.svg"></a>
        <a href="https://www.php.net/license/3_01.txt"><img alt="License" src="https://img.shields.io/badge/license-PHP--3.01-blue.svg"></a>
    </p>
</p>
A high-performance PHP extension for poker hand evaluation and equity calculation using the well-known "Two Plus Two" evaluation algorithm, originally developed by Kevin Suffecool.

## Overview

The PHPoker Extension provides fast poker hand evaluation and equity calculation directly from PHP. It is a direct C implementation of Kevin Suffecool's poker hand evaluator algorithm (also known as the "Two Plus Two" evaluator), which uses perfect hashing and lookup tables to achieve O(1) performance for hand evaluation.

### Features

- Evaluate 5 or 7 card poker hands
- Calculate equity percentages between multiple players
- Support for specifying board cards and dead cards
- Fast C implementation as a PHP extension

## Installation

PHPoker must be compiled specifically for your system's PHP version and architecture. Pre-compiled binaries are not provided in the repository.

### Requirements

- PHP 7.0 or higher (development files)
- C compiler (gcc/clang)
- phpize and php-config
- make

### Dependencies Installation

#### Debian/Ubuntu:
```bash
sudo apt-get install php-dev build-essential
```

#### CentOS/RHEL:
```bash
sudo yum install php-devel gcc make
```

#### macOS:
```bash
brew install php@8.1 # or your PHP version
```

### Building from Source

1. Clone the repository:
   ```bash
   git clone https://github.com/phpoker/phpoker.git
   cd phpoker
   ```

2. Build the extension:
   ```bash
   ./build.sh
   ```

3. If the build script doesn't work for you, you can manually build with:
   ```bash
   mkdir -p build
   cp src/*.c src/*.h src/config.m4 build/
   cd build
   phpize
   ./configure --enable-phpoker
   make
   sudo make install
   ```

4. Enable the extension in your php.ini:
   ```
   extension=phpoker.so
   ```

5. Verify the extension is loaded:
   ```bash
   php -m | grep phpoker
   ```

### Using in Docker

If you want to test the extension in a Docker environment, you can use this sample Dockerfile:

```Dockerfile
FROM php:8.1-cli

# Install dependencies
RUN apt-get update && apt-get install -y git

# Clone and build PHPoker
WORKDIR /tmp
RUN git clone https://github.com/PHPoker/PHPoker-Extension.git \
    && cd phpoker \
    && mkdir -p build \
    && cp src/*.c src/*.h src/config.m4 build/ \
    && cd build \
    && phpize \
    && ./configure --enable-phpoker \
    && make \
    && make install

# Enable the extension
RUN echo "extension=phpoker.so" > /usr/local/etc/php/conf.d/phpoker.ini

# Verify installation 
RUN php -m | grep phpoker

# Set working directory
WORKDIR /app
```

## Usage

### Evaluating a Poker Hand

```php
<?php
// Evaluate a 5-card hand
$result = poker_evaluate_hand('Ah Kh Qh Jh Th');
var_dump($result);

// Evaluate a 7-card hand (finds the best 5-card hand)
$result = poker_evaluate_hand('Ah Kh Qh Jh Th 2c 3d');
var_dump($result);
```

Sample output:
```
array(4) {
  ["value"]=>
  int(1)
  ["rank"]=>
  int(1)
  ["name"]=>
  string(14) "Straight Flush"
  ["cards"]=>
  int(5)
}
```

The return array contains:
- **value**: A numeric evaluation value (lower is better)
- **rank**: Hand rank (1-9, where 1 is a straight flush and 9 is high card)
- **name**: String representation of the hand (e.g., "Straight Flush", "Four of a Kind")
- **cards**: Number of cards in the hand

### Calculating Equity

```php
<?php
// Calculate equity between two hands preflop
$result = poker_calculate_equity(['Ah Ad', 'Kh Kd'], [], 10000);
var_dump($result);

// Calculate equity with a flop
$result = poker_calculate_equity(['Ah Kd', '2c 2h'], ['Kc 7d 2s'], 10000);
var_dump($result);

// With dead cards
$result = poker_calculate_equity(['9h 9d', 'Ad Kh'], [], 10000, ['As']);
var_dump($result);
```

Sample output:
```
array(2) {
  [0]=>
  array(3) {
    ["equity"]=>
    float(82.45)
    ["wins"]=>
    int(8245)
    ["ties"]=>
    int(0)
  }
  [1]=>
  array(3) {
    ["equity"]=>
    float(17.55)
    ["wins"]=>
    int(1755)
    ["ties"]=>
    int(0)
  }
}
```

The `poker_calculate_equity` function accepts:
1. An array of hole cards (strings, one per player)
2. An array of board cards (optional)
3. Number of iterations for the Monte Carlo simulation (optional, default 10,000)
4. An array of dead cards to remove from the deck (optional)

It returns an array with each player's equity and win/tie statistics.

### Card Notation

Cards are represented as a two-character string:
- First character is the rank: `2`, `3`, `4`, `5`, `6`, `7`, `8`, `9`, `T`, `J`, `Q`, `K`, or `A`
- Second character is the suit: `c` (clubs), `d` (diamonds), `h` (hearts), or `s` (spades)

Example: `Ah` = Ace of hearts, `Tc` = Ten of clubs

Cards in a hand are separated by spaces: `Ah Kd Qc Js Th`

## Running Tests

The extension includes test scripts that demonstrate its functionality:

```bash
php test_poker_evaluate_hand.php
php test_poker_calculate_equity.php
```

## How It Works

The Two Plus Two evaluator uses a clever algorithm that can evaluate any poker hand in constant time (O(1)) using lookup tables. The evaluation process:

1. Represents each card with a unique prime number for its rank
2. For non-flush hands, multiplies these prime numbers to get a unique product
3. Uses perfect hashing to map the product to a unique hand value
4. For flush and straight hands, uses bit manipulation and table lookups

The evaluation returns a numeric value where lower numbers indicate stronger hands.

## Performance

The C implementation makes this extension extremely fast compared to pure PHP implementations:

- Evaluating a single hand takes less than a microsecond
- Monte Carlo equity calculations can process tens of thousands of hands per second

## Distribution

Since this is a compiled extension, there are some important considerations for distribution:

1. **Source Code Distribution**:
   - This project is distributed as source code that needs to be compiled for each specific environment
   - This ensures compatibility across different PHP versions and platforms
   - The build script handles most of the complexity for end users

2. **NOT available via PECL**:
   - This extension is not currently distributed through PECL
   - Users must compile from source

3. **Versioning**:
   - Version changes are reflected in the `PHP_PHPOKER_VERSION` constant in `phpoker.h`
   - The GitHub repository uses semantic versioning (MAJOR.MINOR.PATCH)

4. **Extension Compatibility**:
   - Compiled extensions are specific to the PHP version they were built for
   - Users need to rebuild when upgrading PHP versions

## Contributing

Contributions are welcome! Here's how you can help:

1. Fork the repository
2. Create a new branch (`git checkout -b feature/your-feature`)
3. Make your changes
4. Run tests to ensure everything works
5. Commit your changes (`git commit -am 'Add some feature'`)
6. Push to the branch (`git push origin feature/your-feature`)
7. Create a new Pull Request

### Development Setup

For development, you'll need:

1. PHP development environment
2. C compiler and build tools
3. Knowledge of PHP extension development or C programming

The main files to modify:
- `phpoker.c` - Main extension code
- `phpoker.h` - Header file
- `arrays.h` - Contains lookup tables for hand evaluation

### Development Workflow

1. **Make code changes**: Edit the source files in the `src/` directory
2. **Build for testing**: Run `./build.sh` to compile
3. **Test your changes**: Run the test scripts to verify functionality
4. **Memory testing**: Consider using Valgrind to check for memory leaks

### Common Development Tasks

1. **Adding a new function**:
   - Add function prototype to `phpoker.h`
   - Implement function in `phpoker.c`
   - Add to `phpoker_functions` array in `phpoker.c`
   - Create appropriate argument info

2. **Modifying the algorithm**:
   - Most core algorithm functions are in `phpoker.c`
   - The lookup tables are in `arrays.h`

3. **Version bump**:
   - Update `PHP_PHPOKER_VERSION` in `phpoker.h` when making changes

## Acknowledgments

This project is based on:
- Kevin Suffecool's poker hand evaluator algorithm
- The "Two Plus Two" evaluation approach
- Paul Senzee's perfect hash algorithm for hand evaluation

## License

This project is licensed under the PHP License 3.01 - see the LICENSE file for details or visit the official [PHP License page](https://www.php.net/license/3_01.txt).