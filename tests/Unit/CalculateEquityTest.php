<?php

// Make sure the extension is loaded
beforeAll(function () {
    if (!extension_loaded('phpoker')) {
        throw new Exception("The phpoker extension is not loaded. Please check your installation.\n");
    }
});

function assert_equity(array $results, int $index, float $expected, float $margin): void
{
    $actual = $results[$index]['equity'];
    expect($actual)->toBeGreaterThanOrEqual($expected - $margin);
    expect($actual)->toBeLessThanOrEqual($expected + $margin);
}

$iterations = 1000000;
$margin = 0.2;

test('equity preflop AA vs KK', function () use ($iterations, $margin) {
    $result = poker_calculate_equity(['Ah Ad', 'Kh Kd'], [], $iterations);

    expect($result)->toBeArray()->toHaveCount(2);
    assert_equity($result, 0, 82.6, $margin);
    assert_equity($result, 1, 17.3, $margin);
});

test('equity flop AK vs 22 on K72', function () use ($iterations, $margin) {
    $result = poker_calculate_equity(['Ah Kd', '2c 2h'], ['Kc', '7d', '2s'], $iterations);

    expect($result)->toBeArray()->toHaveCount(2);
    assert_equity($result, 0, 1.9, $margin);
    assert_equity($result, 1, 98.0, $margin);
});

test('equity turn JJ vs AT vs 78s on K-9-6-T', function () use ($iterations, $margin) {
    $result = poker_calculate_equity(['Jh Jd', 'As Td', '7h 8h'], ['Kc', '9d', '6s', 'Tc'], $iterations);

    expect($result)->toBeArray()->toHaveCount(3);
    assert_equity($result, 0, 9.5, $margin);
    assert_equity($result, 1, 0.0, $margin);
    assert_equity($result, 2, 90.4, $margin);
});

test('equity river AQ vs 45 on A-4-7-2-5', function () use ($iterations) {
    $result = poker_calculate_equity(['Ah Qd', '4d 5h'], ['As', '4h', '7c', '2d', '5d'], $iterations);

    expect($result)->toBeArray()->toHaveCount(2);
    expect($result[0]['equity'])->toBe(0.0);
    expect($result[1]['equity'])->toBe(100.0);
});

test('equity preflop 99 vs AK with ace dead', function () use ($iterations, $margin) {
    $result = poker_calculate_equity(['9h 9d', 'Ad Kh'], [], $iterations, ['As']);

    expect($result)->toBeArray()->toHaveCount(2);
    assert_equity($result, 0, 59.3, $margin);
    assert_equity($result, 1, 40.6, $margin);
});

test('equity preflop 99 vs AK without dead cards', function () use ($iterations, $margin) {
    $result = poker_calculate_equity(['9h 9d', 'Ad Kh'], [], $iterations);

    expect($result)->toBeArray()->toHaveCount(2);
    assert_equity($result, 0, 54.8, $margin);
    assert_equity($result, 1, 45.1, $margin);
});

test('dead cards reduce deck availability', function () use ($iterations) {
    $ranks = ['2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K', 'A'];
    $suits = ['c', 'd', 'h', 's'];
    $used = ['9h', '9d', 'Ad', 'Kh'];
    $dead = [];

    foreach ($ranks as $rank) {
        foreach ($suits as $suit) {
            $card = $rank . $suit;
            if (in_array($card, $used, true)) {
                continue;
            }
            $dead[] = $card;
        }
    }

    expect(fn() => poker_calculate_equity(['9h 9d', 'Ad Kh'], [], $iterations, $dead))
        ->toThrow(Exception::class, 'Not enough cards left in deck after removing used/dead cards');
});
