/** @file
 * Interfejs klasy analizującej wczytane przez program linie.
 *
 * @author Grzegorz Bogusław Zaleski (418494)
 * @copyright Uniwersytet Warszawski
 * @date 26 kwietnia 2020
 */

#ifndef GAMMA_PARSER_H
#define GAMMA_PARSER_H

#include <stdbool.h>

/** @brief Funkcja wstępnie analizuje linie polecą wczytaną przez program.
 * Funkcja przerabia wczytaną do konsoli linię na identyfikator komendy
 * oraz jej argumenty, alokuje w sposób kontrolowany pamieć na te argumenty,
 * wstępnie sprawdza czy wczytania linii nadaje się do wykonania czy też
 * jest błędna albo pusta - powinna zostać zignorowana.
 * Funkcja pomocnicza w @ref main.
 * @param[in] line          – linia z poleceniem wczytana do programu,
 * @param[in] read          – długość wczytanej linii,
 * @param[out] command      – wskaźnik na bufor identyfikator komendy,
 * @param[out] arg1         – wskaźnik na bufor pierwszego argumentu polecenia,
 * @param[out] arg2         – wskaźnik na bufor drugiego argumentu polecenia,
 * @param[out] arg3         – wskaźnik na bufor trzeciego argumentu polecenia,
 * @param[out] arg4         – wskaźnik na bufor czwartego argumentu polecenia,
 * @param[out] com_len      – wskaźnik na długość identyfikatora komendy,
 * @param[out] arg_len1     – wskaźnik na długość pierwszego argumentu,
 * @param[out] arg_len2     – wskaźnik na długość drugiego argumentu,
 * @param[out] arg_len3     – wskaźnik na długość trzeciego argumentu,
 * @param[out] arg_len4     – wskaźnik na długość czwartego argumentu,
 * @param[out] arguments    – wskaźnik na liczbe argumentów na wczytanej linii,
 * @param[out] memory_error – wskaźnik na zmienna logiczną w której,
 * zanotowywane jest wystąpienie błędu z alokacją pamieci.
 * @return Wartość @p -1 kiedy linia powinna być zignorowana tj. jest pusta
 * lub wystąpił błąd z alokacją pamięci, @p 0 kiedy linia jest błedna
 * i program powinnien zasygnalizować to funkcją @ref call_error,
 * @p 1 jeśli linia reprezentuje poprawne polecenie po wstępnej analizie.
 */
int analyse_line(char *line, int read, char **command, char **arg1,
                 char **arg2, char **arg3, char **arg4, int *com_len,
                 int *arg_len1, int *arg_len2, int *arg_len3, int *arg_len4,
                 int *arguments, bool *memory_error);

#endif //GAMMA_PARSER_H
