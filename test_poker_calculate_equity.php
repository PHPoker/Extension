<?php
// Make sure the extension is loaded
if (!extension_loaded('phpoker')) {
    die("The phpoker extension is not loaded. Please check your installation.\n");
}

echo "Testing poker_calculate_equity function:\n";
echo "======================================\n\n";

// Test case 1: Two players, no community cards yet
echo "Test case 1: Preflop equity - AA vs KK\n";
$result = poker_calculate_equity(['Ah Ad', 'Kh Kd'], [], 10000);
print_table($result, ['AA', 'KK']);
echo "\n";

// Test case 2: Two players with flop
echo "Test case 2: Flop equity - AK vs 22 on K72 flop\n";
$result = poker_calculate_equity(['Ah Kd', '2c 2h'], ['Kc 7d 2s'], 10000);
print_table($result, ['AK', '22']);
echo "\n";

// Test case 3: Three players with turn
echo "Test case 3: Turn equity - JJ vs AT vs 78s on K-9-6-T board\n";
$result = poker_calculate_equity(['Jh Jd', 'As Td', '7h 8h'], ['Kc 9d 6s Tc'], 10000);
print_table($result, ['JJ', 'AT', '78s']);
echo "\n";

// Test case 4: Two players with complete board (river)
echo "Test case 4: River equity (final outcome) - AQ vs 45 on A-4-7-2-5 board\n";
$result = poker_calculate_equity(['Ah Qd', '4d 5h'], ['As 4h 7c 2d 5d'], 10000);
print_table($result, ['AQ', '45']);
echo "\n";

// Test case 5: Two players with dead cards
echo "Test case 5: Preflop equity with dead cards - 99 vs AK (with one Ace dead)\n";
$result = poker_calculate_equity(['9h 9d', 'Ad Kh'], [], 10000, ['As']);
print_table($result, ['99', 'AK']);
echo "\n";

// Test error handling
echo "Testing error handling:\n";
echo "======================\n\n";

// Test with duplicate cards
try {
    $result = poker_calculate_equity(['Ah Kd', 'Ah Qd'], [], 10000);
    echo "Error handling test failed: Duplicate cards should throw an exception\n";
} catch (Exception $e) {
    echo "✓ PASS: Duplicate cards correctly threw exception: " . $e->getMessage() . "\n";
}

// Test with invalid card format
try {
    $result = poker_calculate_equity(['Ah Kd', 'Xx Qd'], [], 10000);
    echo "Error handling test failed: Invalid card format should throw an exception\n";
} catch (Exception $e) {
    echo "✓ PASS: Invalid card format correctly threw exception: " . $e->getMessage() . "\n";
}

// Test with too many board cards
try {
    $result = poker_calculate_equity(['Ah Kd', 'Jh Qd'], ['2s', '3s', '4s', '5s', '6s', '7s'], 10000);
    echo "Error handling test failed: Too many board cards should throw an exception\n";
} catch (Exception $e) {
    echo "✓ PASS: Too many board cards correctly threw exception: " . $e->getMessage() . "\n";
}

// Test with too few players
try {
    $result = poker_calculate_equity(['Ah Kd'], [], 50000);
    echo "Error handling test failed: Too few players should throw an exception\n";
} catch (Exception $e) {
    echo "✓ PASS: Too few players correctly threw exception: " . $e->getMessage() . "\n";
}

echo "\nAll tests completed.\n";


// Helper function to print equity results in a table
function print_table($results, $labels) {
    echo str_pad("Hand", 10) . " | " .
        str_pad("Equity %", 10) . " | " .
        str_pad("Wins", 10) . " | " .
        str_pad("Ties", 10) . "\n";
    echo str_repeat("-", 45) . "\n";

    foreach ($results as $i => $player) {
        $label = isset($labels[$i]) ? $labels[$i] : "Player " . ($i + 1);
        echo str_pad($label, 10) . " | " .
            str_pad(number_format($player['equity'], 2) . "%", 10) . " | " .
            str_pad($player['wins'], 10) . " | " .
            str_pad($player['ties'], 10) . "\n";
    }
}

