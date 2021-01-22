/** @file
 * Interfejs klasy przechowującej stan gry gamma
 *
 * @author Marcin Peczarski & Grzegorz B. Zaleski
 * @copyright Uniwersytet Warszawski
 * @date 18 marca - 12 maja 2020
 */

#ifndef GAMMA_H
#define GAMMA_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Maksymalny indeks gracza którego identyfikator
 * można zapisać używając jednego znaku.
 */
#define WIDE 9

/**
 * Makro czysto stylistyczne, dotyczące liczb naturalnych 64-bitowych.
 */
typedef uint64_t muint;

/**
 * Makro czysto stylistyczne, dotyczące liczb naturalnych 32-bitowych.
 */
typedef uint32_t uint;

/**
 * Struktura gracza gry Gamma
 */
typedef struct player player;

/**
 * Struktura przechowująca stan gry.
 */
typedef struct gamma gamma_t;

/** @brief Tworzy strukturę przechowującą stan gry.
 * Alokuje pamięć na nową strukturę przechowującą stan gry.
 * Inicjuje tę strukturę tak, aby reprezentowała początkowy stan gry.
 * @param[in] width   – szerokość planszy, liczba dodatnia,
 * @param[in] height  – wysokość planszy, liczba dodatnia,
 * @param[in] players – liczba graczy, liczba dodatnia,
 * @param[in] areas   – maksymalna liczba obszarów,
 *                      jakie może zająć jeden gracz,
 * @return Wskaźnik na utworzoną strukturę lub NULL, gdy nie udało się
 * zaalokować pamięci lub któryś z parametrów jest niepoprawny.
 */
gamma_t *gamma_new(uint width, uint height, uint players, uint areas);

/** @brief Usuwa strukturę przechowującą stan gry.
 * Usuwa z pamięci strukturę wskazywaną przez @p g.
 * Nic nie robi, jeśli wskaźnik ten ma wartość NULL.
 * @param[in] game    – wskaźnik na usuwaną strukturę.
 */
void gamma_delete(gamma_t *game);

/** @brief Wykonuje ruch.
 * Ustawia pionek gracza @p player na polu (@p x, @p y).
 * @param[in,out] game – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player   – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new,
 * @param[in] x        – numer kolumny, liczba nieujemna mniejsza od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y        – numer wiersza, liczba nieujemna mniejsza od wartości
 *                      @p height z funkcji @ref gamma_new.
 * @return Wartość @p true, jeśli ruch został wykonany, a @p false,
 * gdy ruch jest nielegalny lub któryś z parametrów jest niepoprawny.
 */
bool gamma_move(gamma_t *game, uint player, uint x, uint y);

/** @brief Oblicza logarytm dziesiętny zaokrąglony w góre do liczby całkowitej.
 * Funkcja pomocnicza do szacowania rozmiaru tablicy w @ref gamma_spaced_board,
 * @ref show_board oraz @ref interactive_game.
 * @param[in] x    – liczba logarytmowana.
 * @return Wynik tego działania.
 */
uint ceil_log(uint x);

/** @brief Wykonuje złoty ruch.
 * Ustawia pionek gracza @p player na polu (@p x, @p y) zajętym przez innego
 * gracza, usuwając pionek innego gracza.
 * @param[in,out] game – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player   – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new,
 * @param[in] x        – numer kolumny, liczba nieujemna mniejsza od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y        – numer wiersza, liczba nieujemna mniejsza od wartości
 *                      @p height z funkcji @ref gamma_new.
 * @return Wartość @p true, jeśli ruch został wykonany, a @p false,
 * gdy gracz wykorzystał już swój złoty ruch, ruch jest nielegalny
 * lub któryś z parametrów jest niepoprawny.
 */
bool gamma_golden_move(gamma_t *game, uint player, uint x, uint y);

/** @brief Podaje liczbę pól zajętych przez gracza.
 * Podaje liczbę pól zajętych przez gracza @p player.
 * @param[in] game    – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new.
 * @return Liczba pól zajętych przez gracza lub zero,
 * jeśli któryś z parametrów jest niepoprawny.
 */
muint gamma_busy_fields(gamma_t *game, uint player);

/** @brief Podaje liczbę pól, jakie jeszcze gracz może zająć.
 * Podaje liczbę wolnych pól, na których w danym stanie gry gracz @p player może
 * postawić swój pionek w następnym ruchu.
 * @param[in] game    – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new.
 * @return Liczba pól, jakie jeszcze może zająć gracz lub zero,
 * jeśli któryś z parametrów jest niepoprawny.
 */
muint gamma_free_fields(gamma_t *game, uint player);

/** @brief Sprawdza, czy gracz może wykonać złoty ruch.
 * Sprawdza, czy gracz @p player jeszcze nie wykonał w tej rozgrywce złotego
 * ruchu i jest przynajmniej jedno pole zajęte przez innego gracza, którego
 * zajęcie nie rozpójni obszarów tego gracza ponad dozwoloną liczbę.
 * @param[in] game    – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new.
 * @return Wartość @p true, jeśli gracz jeszcze nie wykonał w tej rozgrywce
 * złotego ruchu i jest przynajmniej jedno pole zajęte przez innego gracza,
 * a @p false w przeciwnym przypadku.
 */
bool gamma_golden_possible(gamma_t *game, uint player);

/** @brief Daje napis opisujący stan planszy.
 * Alokuje w pamięci bufor, w którym umieszcza napis zawierający tekstowy
 * opis aktualnego stanu planszy.
 * Funkcja wywołująca musi zwolnić ten bufor.
 * @param[in] game    – wskaźnik na strukturę przechowującą stan gry.
 * @return Wskaźnik na zaalokowany bufor zawierający napis opisujący stan
 * planszy lub NULL, jeśli nie udało się zaalokować pamięci.
 */
char *gamma_board(gamma_t *game);

/** @brief Funkcja pomocznicza do przesuwania wyświetlanej treści.
 * Wypisuje na stdout @p margin spacji dzięki czemu następny stdout
 * jest przesunięty w prawo.
 * @param[in] margin – długośc przesunięcia, tj. liczba spacji,
*/
void push (uint margin);

/** @brief Wyświetla aktualny stan planszy.
 * Wypisuje do konsoli aktualny stan planszy z uzglednienie tego że na
 * polu (@p x, @p y) znajduje się aktualnie kursor.
 * Funkcja pomocnicza w @ref interactive_game.
 * @param[in] game   – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] cur_x  – numer kolumny, gdzie znajduje się kursor, liczba nieujemna
 *                      mniejsza od wartości @p width z funkcji @ref gamma_new,
 * @param[in] cur_y  – numer wiersza, gdzie znajduje się kursor, liczba nieujemna
 *                      mniejsza od wartości @p width z funkcji @ref gamma_new,
 * @param[in] player – identyfikator gracza, którego jest tura,
 * @param[in] margin – zmienna pomocnicza do środkowa planszy,
 *                      tj. odległość planszy od lewej ściany terminala.
 */
void show_board(gamma_t *game, uint cur_x, uint cur_y, uint player, uint margin);

/** @brief Zwraca liczbę wszystkich zajętych pól na planszy.
 * Funkcja pomocnicza w @ref interactive_game.
 * @param[in] game – wskaźnik na strukturę przechowującą stan gry.
 * @return Liczba zajętych pól.
 */
muint gamma_all_busy_fields(gamma_t *game);

/** @brief Zwraca liczbę wszystkich obszarów gracza @p player.
 * Funkcja pomocnicza w @ref interactive_game.
 * @param[in] game   – wskaźnik na strukturę przechowującą stan gry.
 * @param[in] player – identyfikator gracza.
 * @return Liczba obszarów danego gracza.
 */
uint gamma_areas(gamma_t *game, uint player);

/** @brief Zwraca liczbe pól zajętych przez gracza, który zajął
 * ich najwięcej.
 * Funkcja pomocnicza w @ref interactive_game.
 * @param[in] game   – wskaźnik na strukturę przechowującą stan gry.
 * @return Liczba pól tego gracza.
 */
uint gamma_best_result(gamma_t *game);

#endif /* GAMMA_H */
