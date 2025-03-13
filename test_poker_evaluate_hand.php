<?php
// Make sure the extension is loaded
if (!extension_loaded('phpoker')) {
    die("The phpoker extension is not loaded. Please check your installation.\n");
}

echo "Testing poker_evaluate_hand function:\n";
echo "=====================================\n\n";

// Test various 5-card poker hands
echo "Testing 5-card hands:\n";
echo "--------------------\n";

$test_hands_5 = [
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

foreach ($test_hands_5 as $hand => $expected_name) {
    $result = poker_evaluate_hand($hand);

    echo "Hand: $hand\n";
    echo "Evaluation Value: {$result['value']}\n";
    echo "Hand Rank: {$result['rank']}\n";
    echo "Hand Name: {$result['name']}\n";
    echo "Card Count: {$result['cards']}\n";

    if ($result['name'] == $expected_name) {
        echo "✓ PASS: Correctly identified as $expected_name\n";
    } else {
        echo "✗ FAIL: Expected $expected_name, got {$result['name']}\n";
    }

    echo "\n";
}

// Test various 7-card poker hands
echo "\nTesting 7-card hands:\n";
echo "--------------------\n";

$test_hands_7 = [
    'Ah Kh Qh Jh Th 2c 3d' => 'Straight Flush',  // Should find the royal flush in the first 5 cards
    '8c 8s 8h 8d Kh 2c 3d' => 'Four of a Kind',  // Four 8s already in the hand
    '2c 2h Qs Qc Qh 3d 4s' => 'Full House',      // Three queens and a pair of 2s
    '2d 7d 3c 9d Jd Ad 5d' => 'Flush',          // Five diamonds
    '9c Th 5s Jc Qd Kh 2c' => 'Straight',       // 9-10-J-Q-K straight
    '5s 5h 5d Jc 2d 3c 7h' => 'Three of a Kind', // Three 5s
    '8s 8h Tc Td 3c 4d 5h' => 'Two Pair',       // Pairs of 8s and 10s
    'As Ah 7c 4d 2s 3c 9d' => 'One Pair',       // Pair of aces
    'Kh Td 7c 4s 2d 3h 9c' => 'High Card'       // No matching cards
];

foreach ($test_hands_7 as $hand => $expected_name) {
    $result = poker_evaluate_hand($hand);

    echo "Hand: $hand\n";
    echo "Evaluation Value: {$result['value']}\n";
    echo "Hand Rank: {$result['rank']}\n";
    echo "Hand Name: {$result['name']}\n";
    echo "Card Count: {$result['cards']}\n";

    if ($result['name'] == $expected_name) {
        echo "✓ PASS: Correctly identified as $expected_name\n";
    } else {
        echo "✗ FAIL: Expected $expected_name, got {$result['name']}\n";
    }

    echo "\n";
}

// Test hands that should get better with 7 cards than 5
echo "\nTesting improved 7-card hands:\n";
echo "-----------------------------\n";

$improved_hands = [
    '2c 3c 4c 5c 7d Ac 6c' => 'Straight Flush', // The 7d would make this just a flush with 5 cards, but 7 cards lets us make a straight flush
    '2h 3h 4h 5h 6d 7d 8h' => 'Flush',          // Can make a flush with the 5 hearts
    '2c 3d 4h 5s 6c 7h Kh' => 'Straight',       // Can make a straight with 2-3-4-5-6
    '5h 5d 5c Kd Qc Js 2h' => 'Three of a Kind', // Three 5s (making sure no straight)
    'Ah Ad Kc Ks 3c 7d Td' => 'Two Pair'        // Pairs of aces and kings
];

foreach ($improved_hands as $hand => $expected_name) {
    $result = poker_evaluate_hand($hand);

    echo "Hand: $hand\n";
    echo "Evaluation Value: {$result['value']}\n";
    echo "Hand Rank: {$result['rank']}\n";
    echo "Hand Name: {$result['name']}\n";
    echo "Card Count: {$result['cards']}\n";

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
    echo "Error handling test failed: Invalid card should throw an exception\n";
} catch (Exception $e) {
    echo "✓ PASS: Invalid card correctly threw exception: " . $e->getMessage() . "\n";
}

// Test with wrong number of cards (4 cards - too few)
try {
    $result = poker_evaluate_hand('Ah Kh Qh Jh');
    echo "Error handling test failed: Too few cards should throw an exception\n";
} catch (Exception $e) {
    echo "✓ PASS: Wrong number of cards correctly threw exception: " . $e->getMessage() . "\n";
}

// Test with 6 cards (neither 5 nor 7)
try {
    $result = poker_evaluate_hand('Ah Kh Qh Jh Th 9s');
    echo "Error handling test failed: 6 cards should throw an exception\n";
} catch (Exception $e) {
    echo "✓ PASS: 6 cards correctly threw exception: " . $e->getMessage() . "\n";
}

echo "\nAll tests completed.\n";

