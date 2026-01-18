<?php
/**
 * Simple Poker Equity Calculator Performance Test
 *
 * Benchmark script for comparing the performance of the C extension
 * with a pure PHP implementation.
 */

// Make sure the extension is loaded
if (!extension_loaded('phpoker')) {
    die("The phpoker extension is not loaded. Please check your installation.\n");
}

// Benchmarking function
function benchmark_equity($scenario, $hole_cards, $board_cards, $iterations) {
    echo "Running scenario: $scenario\n";
    echo "  Cards: " . implode(", ", $hole_cards) . "\n";

    if (!empty($board_cards)) {
        echo "  Board: " . implode(" ", $board_cards) . "\n";
    } else {
        echo "  Board: None\n";
    }

    echo "  Iterations: " . number_format($iterations) . "\n";

    // Time the calculation
    $start = microtime(true);
    $result = poker_calculate_equity($hole_cards, $board_cards, $iterations);
    $end = microtime(true);

    $time = $end - $start;
    $iterations_per_second = $iterations / $time;

    // Display results
    echo "  Time: " . number_format($time, 4) . " seconds\n";
    echo "  Speed: " . number_format($iterations_per_second) . " iterations/second\n";

    // Show sample of equity calculation
    echo "  Equity results:\n";
    foreach ($result as $i => $player) {
        echo "    Player " . ($i + 1) . ": " .
            number_format($player['equity'], 2) . "% (Wins: " .
            number_format($player['wins']) . ", Ties: " .
            number_format($player['ties']) . ")\n";
    }

    echo "\n";

    return [
        'scenario' => $scenario,
        'time' => $time,
        'iterations_per_second' => $iterations_per_second
    ];
}

// Test scenarios
$scenarios = [
    // Preflop scenarios
    [
        'name' => '1. Preflop: AA vs KK',
        'hole_cards' => ['Ah Ad', 'Kh Kd'],
        'board' => [],
        'iterations' => 100000
    ],
    [
        'name' => '2. Preflop: AK vs QQ',
        'hole_cards' => ['Ah Kd', 'Qh Qd'],
        'board' => [],
        'iterations' => 100000
    ],
    [
        'name' => '3. Preflop: 3 players',
        'hole_cards' => ['Ah Kd', 'Qh Qd', 'Jh Td'],
        'board' => [],
        'iterations' => 100000
    ],

    // Flop scenarios
    [
        'name' => '4. Flop: Set vs Flush Draw',
        'hole_cards' => ['Ah Ad', '2h 3h'],
        'board' => ['Ac', '8h', '9h'],
        'iterations' => 50000
    ],

    // Turn scenarios
    [
        'name' => '5. Turn: Top Pair vs Straight Draw',
        'hole_cards' => ['Ah Kd', 'Tc 9c'],
        'board' => ['Ad', '8h', '7c', '6s'],
        'iterations' => 25000
    ]
];

echo "PHPoker Equity Calculator Performance Test\n";
echo "=========================================\n\n";

echo "PHP Version: " . PHP_VERSION . "\n";
echo "System: " . php_uname() . "\n\n";

$results = [];

// Run all benchmarks
foreach ($scenarios as $scenario) {
    $results[] = benchmark_equity(
        $scenario['name'],
        $scenario['hole_cards'],
        $scenario['board'],
        $scenario['iterations']
    );
}

// Summary
echo "Performance Summary\n";
echo "-----------------\n";
echo str_pad("Scenario", 30) . " | " .
    str_pad("Time (s)", 10) . " | " .
    "Iterations/second\n";
echo str_repeat("-", 70) . "\n";

foreach ($results as $result) {
    echo str_pad($result['scenario'], 30) . " | " .
        str_pad(number_format($result['time'], 4), 10) . " | " .
        number_format($result['iterations_per_second']) . "\n";
}

echo "\nBenchmark completed!\n";