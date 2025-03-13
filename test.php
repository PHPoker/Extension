<?php
// Make sure the extension is loaded
if (!extension_loaded('phpoker')) {
    die("The phpoker extension is not loaded. Please check your installation.\n");
}

// Test various poker hands
$test_hands = [
    'Ah Kh Qh Jh Th' => 'Straight Flush',  // Royal flush (technically a straight flush)
    '8c 8s 8h 8d Kh' => 'Four of a Kind',
    'Qc Qh Qs Kc Kh' => 'Full House',
    '2d 7d 9d Jd Ad' => 'Flush',
    '9c Th Jc Qd Kh' => 'Straight',
    '5s 5h 5d Jc 2d' => 'Three of a Kind',
    '8s 8h Tc Td 3c' => 'Two Pair',
    'As Ah 7c 4d 2s' => 'One Pair',
    'Kh Td 7c 4s 2d' => 'High Card'
];

echo "Testing poker_evaluate_hand function:\n";
echo "=====================================\n\n";

foreach ($test_hands as $hand => $expected_name) {
    $result = poker_evaluate_hand($hand);

    echo "Hand: $hand\n";
    echo "Evaluation Value: {$result['value']}\n";
    echo "Hand Rank: {$result['rank']}\n";
    echo "Hand Name: {$result['name']}\n";

    if ($result['name'] == $expected_name) {
        echo "✓ PASS: Correctly identified as $expected_name\n";
    } else {
        echo "✗ FAIL: Expected $expected_name, got {$result['name']}\n";
    }

    echo "\n";
}

// Additional tests for error handling
echo "Testing error handling:\n";
echo "======================\n\n";

// Test with invalid card
try {
    $result = poker_evaluate_hand('Ah Kh Qh Jh Xx');
    echo "Error handling test failed: Invalid card should raise an error\n";
} catch (Exception $e) {
    echo "✓ PASS: Invalid card correctly caused an error\n";
}

// Test with wrong number of cards
try {
    $result = poker_evaluate_hand('Ah Kh Qh Jh');
    echo "Error handling test failed: Too few cards should raise an error\n";
} catch (Exception $e) {
    echo "✓ PASS: Wrong number of cards correctly caused an error\n";
}

echo "\nAll tests completed.\n";