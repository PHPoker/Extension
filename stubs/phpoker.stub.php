<?php

/**
 * Evaluate a 5- or 7-card poker hand.
 *
 * @param string $hand Space-separated card codes (e.g. "Ah Kh Qh Jh Th").
 * @return array{value:int, rank:int, name:string, cards:int}
 */
function poker_evaluate_hand(string $hand): array
{
}

/**
 * Calculate equity for multiple poker hands.
 *
 * @param list<string> $hole_cards Each player's hole cards (e.g. "Ah Ad").
 * @param list<string>|null $board_cards Optional board cards (0-5 cards).
 * @param int|null $iterations Optional number of iterations for Monte Carlo.
 * @param list<string>|null $dead_cards Optional dead cards to remove from the deck.
 * @return array<int, array{equity:float, wins:int, ties:int}>
 */
function poker_calculate_equity(
    array $hole_cards,
    ?array $board_cards = null,
    ?int $iterations = null,
    ?array $dead_cards = null
): array {
}
