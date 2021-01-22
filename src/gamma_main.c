/** @file
 * Główna cześć pogramu - udostępniająca rozgrywkę w grę
 * Gamma w dwóch dostępnych trybach.
 *
 * @author Grzegorz Bogusław Zaleski (418494)
 * @copyright Uniwersytet Warszawski
 * @date 15 maja 2020
 */

/**
 * Makro wymagane do poprawnego działania funkcji @ref getline.
 */
#define _GNU_SOURCE

/**
 * Oznaczenie końca pliku.
 */
#define END -1

/**
 * Największa liczba dostępna w formacie uint.
 */
#define MAXUINT 4294967295

#include "gamma.h"
#include "parser.h"
#include "interactive.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/** @brief Sprawdzenie czy dwa ciągi znaków są identyczne.
 * Funkcja pomocnicza w @ref interactive_game.
 * @param[in] A – pierwszy ciąg znaków,
 * @param[in] B – drugi ciąg znaków,
 * @return Wartość @p true
 */
inline static bool same_string(char *A, char *B)
{
    return strcmp(A, B) == 0;
}

/** @brief Funkcja konwertuje napis na liczbę.
 * Ciag znaków przy użyciu funkcji @ref strtoull jest przerabiany na liczbę
 * z zakresu uint, funkcja dodatkowo sprawdza też czy liczba nie jest zbyt
 * duża jak na ograniczenia w zadaniu.
 * Funkcja pomocnicza w @ref main.
 * @param[in] arg     – ciag znaków zawierający liczbę,
 * @param[in] result  – liczba wynikowa (referencja na nią),
 * @return Wartość @p true, jeśli napis @p arg zawierał poprawną liczbę
 * w formacie uint, a @p false w przeciwnym razie.
 */
static bool convert(char *arg, uint *result)
{
    // Przypadek brzegowy - liczba zero,
    if (same_string(arg, "\0"))
    {
        *result = 0;
        return true;
    }

    muint preresult = strtoull(arg, NULL, 10);
    if (preresult <= MAXUINT)
    {
        *result = (uint) preresult;
        return true;
    }
    return false;
}

/** @brief Funkcja wypisująca błąd (error).
 * Funkcja wypisuje wiadomosć na wyjscie stderr o błędzie podczas działania
 * programu, tj. że została wczytana błędna linia z argumentami.
 * Funkcja pomocnicza w @ref main.
 * @param[in] line – numer linii gdzie nastąpił błąd.
 */
inline static void call_error(int line)
{
    fprintf(stderr,"ERROR %d\n", line);
}

/** @brief Główna część programu - wykonywanie działań zadanych przez wczytane komendy.
 * Program służy do prowadzenia rozgrywki w grę gamma w jednym z dwóch trybów.
 * Poprzez komende w konsoli wybierany jest jeden z trybów gry (wsadowy lub
 * interaktywny) a także ustalane parametry rozgrywki takie jak rozmiar planszy
 * liczba graczy czy maksymalna liczba obszarów.
 * Po wybraniu trybu interaktywnego funkcja wywołuje funkcję
 * @ref interactive_game. Po zakończeniu rozgrywki program czyści całą
 * zaalokowaną pamięć.
 * @return Wartość @p 0, jeśli program zakończył wykonywanie
 * poleceń bez żadnych błedów, a @p 1 jeśli napotkał błąd z alokacją pamięci.
 */
int main()
{
    // Deklaracja zmiennych używanych przez program.
    size_t mem;
    char *line = NULL, *command = NULL,
         *arg1 = NULL, *arg2 = NULL, *arg3 = NULL, *arg4 = NULL;
    int line_cnt = 0, com_len = 0, arg_len1 = 0,
        arg_len2 = 0, arg_len3 = 0, arg_len4 = 0,
        read, dir, arguments;
    uint argument1, argument2, argument3, argument4;
    gamma_t *game = NULL;
    bool error_occured = false;
    bool game_allocated = false;

    // Wczytywanie kolejnych linii poleceń aż do momentu wczytanie End of File.
    while (error_occured == false && (read = getline(&line, &mem, stdin)) != END)
    {
        // Licznik linii.
        line_cnt++;

        // Analiza wczytanego polecenia i przeparsowanie jego na argumenty.
        dir = analyse_line(line, read, &command, &arg1, &arg2, &arg3, &arg4,
               &com_len, &arg_len1, &arg_len2, &arg_len3, &arg_len4, &arguments, &error_occured);

        // Wyjście awaryjne z powodu braku pamięci.
        if (error_occured)
        {
            break;
        }

        // Próba rozpoczecia jednego z dwóch rodzai rozgrywki.
        if (dir == 1 && convert(arg1, &argument1) && convert(arg2, &argument2)
            && convert(arg3, &argument3) && convert(arg4, &argument4))
        {
            if (game == NULL && arguments == 4)
            {
                if (same_string(command,"B"))
                {
                    // Wywołanie trybu wsadowego.
                    if ((game = gamma_new(argument1, argument2,
                            argument3, argument4)) != NULL)
                    {
                        game_allocated = true;
                        printf("OK %d\n", line_cnt);
                    }
                    else
                    {
                        call_error(line_cnt);
                    }
                }
                else if (same_string(command,"I"))
                {
                    // Wywołanie trybu interaktywnego.
                    if ((game = gamma_new(argument1, argument2,
                            argument3, argument4)) != NULL)
                    {
                        game_allocated = true;
                        error_occured = interactive_game(game, argument1, argument2,
                                argument3, argument4) == false;

                        // Po grze interaktywnej program kończy działanie.
                        free(command);
                        free(arg1);
                        free(arg2);
                        free(arg3);
                        free(arg4);
                        break;
                    }
                    else
                    {
                        call_error(line_cnt);
                    }
                }
                else
                {
                    call_error(line_cnt);
                }
            }
            // Komenda modyfikująca rozgrywkę w trybie wsadowym.
            else if (game != NULL)
            {
                // Wywołanie zwykłego ruchu.
                if (same_string(command, "m") && arguments == 3)
                {
                    bool response = gamma_move(game, argument1, argument2, argument3);
                    printf("%d\n",response);
                }
                // Wywołanie złotego ruchu.
                else if (same_string(command, "g") && arguments == 3)
                {
                    bool response = gamma_golden_move(game, argument1, argument2, argument3);
                    printf("%d\n",response);
                }
                // Wywołanie liczby zajętych pól.
                else if (same_string(command, "b") && arguments == 1)
                {
                    muint response = gamma_busy_fields(game, argument1);
                    printf("%ld\n", response);
                }
                // Wywołanie liczby wolnych pól danego gracza.
                else if (same_string(command, "f") && arguments == 1)
                {
                    muint response = gamma_free_fields(game, argument1);
                    printf("%ld\n", response);
                }
                // Pytanie o możliwość wykonania złotego ruchu.
                else if (same_string(command, "q") && arguments == 1)
                {
                    bool response = gamma_golden_possible(game, argument1);
                    printf("%d\n", response);
                }
                // Wyświetlenie planszy aktualnego stanu rozgrywki.
                else if (same_string(command, "p") && arguments == 0)
                {
                    char *response = gamma_board(game);
                    if (response == NULL)
                    {
                        call_error(line_cnt);
                    }
                    else
                    {
                        printf("%s", response);
                        free(response);
                    }
                }
                else
                {
                    call_error(line_cnt);
                }
            }
            else
            {
                call_error(line_cnt);
            }
        }
        // Błędne polecenie.
        else if (dir != -1)
        {
            call_error(line_cnt);
        }

        // Przygotowanie pamięci pod kolejne polecenie.
        free(command);
        free(arg1);
        free(arg2);
        free(arg3);
        free(arg4);
    }

    // Oczyszczenie pamieci pod koniec programu.
    free(line);
    if (game_allocated)
    {
        gamma_delete(game);
    }

    // Zakomunikowanie błędu.
    if (error_occured)
    {
        exit(1);
    }

    return 0;
}
