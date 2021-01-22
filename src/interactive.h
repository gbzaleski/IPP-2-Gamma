/** @file
 * Interfejs rozgrywki w gre Gamma w trybie interaktywnym.
 *
 * @author Grzegorz Bogusław Zaleski (418494)
 * @copyright Uniwersytet Warszawski
 * @date 5 maja 2020
 */

#ifndef GAMMA_INTERACTIVE_H
#define GAMMA_INTERACTIVE_H

#include "gamma.h"

/** @brief Funkcja przeprowadza rozgrywke w trybie interaktywnym.
 * Funkcja wczytuje kolejne poleca w sposób interaktwny, strzałkami przesuwane
 * są pozycje kursora na planszy, spacja wywołuje @ref gamma_move gracza
 * którego aktualnie jest tura, klawisz G wywoluje @ref gamma_golden_move
 * a klawisz C pomija turę aktualnego gracza. Wyświetlana plansza jest dynamicznie
 * zmieniana. Po zakończenej grze funkcja wyświetla wyniki gry.
 * Istnieje też możliwosć przerwania gry za pomocą kombinacji klawiszy Ctrl-D.
 * @param[in,out] game   – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] _width     – szerokość planszy, liczba dodatnia,
 * @param[in] _height    – wysokość planszy, liczba dodatnia,
 * @param[in] players    – liczba graczy, liczba dodatnia,
 * @param[in] max_areas  – maksymalna liczba obszarów,
 *                      jakie może zająć jeden gracz,
 * @return Wartość @p true, jeśli rozgrywka została pomyślnie przeprowadzona,
 * albo @p false jeśli nastąpił niespodziewany błąd.
 */
bool interactive_game(gamma_t *game, uint width, uint height, uint players, uint max_areas);

#endif //GAMMA_INTERACTIVE_H
