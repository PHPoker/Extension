<?php

test('has the poker_evaluate_hand function', function () {
    expect(function_exists('poker_evaluate_hand'))->toBeTrue();
});


test('correctly identifies a 5-card hand', function ($handString, $expectedName, $expectedRank) {
    $result = poker_evaluate_hand($handString);

    expect($result)->toBeArray()
                   ->toHaveKeys(['value', 'rank', 'name', 'cards'])
                   ->and($result['rank'])->toBe($expectedRank)
                   ->and($result['name'])->toBe($expectedName)
                   ->and($result['cards'])->toBe(5);

    expect($result['value'])->toBeInt()->toBeGreaterThanOrEqual(0);
})->with([
    'Royal Flush' => ['Ah Kh Qh Jh Th', 'Straight Flush', 1],
    'Straight Flush' => ['9c 8c 7c 6c 5c', 'Straight Flush', 1],
    'Four of a Kind' => ['8c 8s 8h 8d Kh', 'Four of a Kind', 2],
    'Full House' => ['Qc Qh Qs Kc Kh', 'Full House', 3],
    'Flush' => ['2d 7d 9d Jd Ad', 'Flush', 4],
    'Straight' => ['9c Th Jc Qd Kh', 'Straight', 5],
    'Wheel Straight' => ['Ac 2d 3h 4s 5c', 'Straight', 5],
    'Three of a Kind' => ['5s 5h 5d Jc 2d', 'Three of a Kind', 6],
    'Two Pair' => ['8s 8h Tc Td 3c', 'Two Pair', 7],
    'One Pair' => ['As Ah 7c 4d 2s', 'One Pair', 8],
    'High Card' => ['Kh Td 7c 4s 2d', 'High Card', 9],
]);


test('correctly identifies the best 5-card hand from 7 cards', function ($handString, $expectedName, $expectedRank) {
    $result = poker_evaluate_hand($handString);

    expect($result)->toBeArray()
                   ->toHaveKeys(['value', 'rank', 'name', 'cards'])
                   ->and($result['rank'])->toBe($expectedRank)
                   ->and($result['name'])->toBe($expectedName)
                   ->and($result['cards'])->toBe(7);

    expect($result['value'])->toBeInt()->toBeGreaterThanOrEqual(0);
})->with([
    'Royal Flush in 7 cards' => ['Ah Kh Qh Jh Th 2c 3d', 'Straight Flush', 1],
    'Four of a Kind in 7 cards' => ['8c 8s 8h 8d Kh 2c 3d', 'Four of a Kind', 2],
    'Full House with multiple options' => ['2c 2h Qs Qc Qh 3d 3s', 'Full House', 3],
    'Flush with extra cards' => ['2d 7d 3c 9d Jd Ad 5d', 'Flush', 4],
    'Straight with extra cards' => ['9c Th 5s Jc Qd Kh 2c', 'Straight', 5],
    'Three of a Kind with better options' => ['5s 5h 5d Jc 2d 3c 7h', 'Three of a Kind', 6],
    'Two Pair with better options' => ['8s 8h Tc Td 3c 4d 5h', 'Two Pair', 7],
    'One Pair with ace kicker' => ['As Ah 7c 4d 2s 3c 9d', 'One Pair', 8],
    'High Card with ace high' => ['Kh Td 7c 4s 2d 3h Ac', 'High Card', 9],
    'Improved hand (straight flush from scattered cards)' => ['2c 3c 4c 5c 7d Ac 6c', 'Straight Flush', 1],
    'Improved hand (flush with scattered hearts)' => ['2h 3h 4h 5h 6d 7d 8h', 'Flush', 4],
    'Improved hand (straight from scattered cards)' => ['2c 3d 4h 5s 6c 7h Kh', 'Straight', 5],
]);


test('throws exception for invalid input', function ($handString, $exceptionMessage) {
    expect(fn() => poker_evaluate_hand($handString))->toThrow(Exception::class, $exceptionMessage);
})->with([
    'Invalid card' => ['Ah Kh Qh Jh Xx', 'Invalid card format in hand'],
    'Too few cards' => ['Ah Kh Qh Jh', 'Invalid number of cards (need exactly 5 or 7)'],
    'Six cards (neither 5 nor 7)' => ['Ah Kh Qh Jh Th 9s', 'Invalid number of cards (need exactly 5 or 7)'],
]);


test('ranks hands correctly from best to worst', function () {

    $hands = [
        'Ah Kh Qh Jh Th', // Royal flush (Straight Flush rank 1)
        '8c 8s 8h 8d Kh', // Four of a kind (rank 2)
        'Qc Qh Qs Kc Kh', // Full house (rank 3)
        '2d 7d 9d Jd Ad', // Flush (rank 4)
        '9c Th Jc Qd Kh', // Straight (rank 5)
        '5s 5h 5d Jc 2d', // Three of a kind (rank 6)
        '8s 8h Tc Td 3c', // Two pair (rank 7)
        'As Ah 7c 4d 2s', // One pair (rank 8)
        'Kh Td 7c 4s 2d', // High card (rank 9)
    ];

    $values = array_map(function ($hand) {
        $result = poker_evaluate_hand($hand);
        return $result['value'];
    }, $hands);

    array_reduce($values, function ($prev, $current) {
        if ($prev !== null) {
            expect($prev)->toBeLessThan($current);
        }
        return $current;
    }, null);
});


test('correctly ranks hands of the same type by kickers', function () {
    // Test pairs with different kickers
    $result1 = poker_evaluate_hand('As Ah Kc Qd 2s');
    $result2 = poker_evaluate_hand('As Ah Kc Jd 2s');
    expect($result1['value'])->toBeLessThan($result2['value']);


    $result1 = poker_evaluate_hand('As Ah Kc Kd 2s');
    $result2 = poker_evaluate_hand('Qs Qh Kc Kd 2s');
    expect($result1['value'])->toBeLessThan($result2['value']);

    $result1 = poker_evaluate_hand('As Ks Qs Js 9s');
    $result2 = poker_evaluate_hand('Ks Qs Js 9s 7s');
    expect($result1['value'])->toBeLessThan($result2['value']);
});
