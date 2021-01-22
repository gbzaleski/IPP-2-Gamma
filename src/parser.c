/** @file
 * Implementacja funkcji analizujących wczytane linie.
 *
 * @author Grzegorz Bogusław Zaleski (418494)
 * @copyright Uniwersytet Warszawski
 * @date 12 maja 2020
 */

#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/** @brief Funkcja sprawdza czy napis reprezentuje poprawną liczbę.
 * Funkcja sprawdza czy podany napis zawiera same cyfry,
 * bez zer wiodących oraz czy liczba tworzona przez uint mieści się
 * w zakresie wartości liczby uint.
 * Funkcja pomocnicza w @ref analyse_line.
 * @param[in] arg  – wskaźnik na ciag znaków do sprawdzenia,
 * @param[in] len  – długość napisu (ciągu znaków),
 * @return Wartość @p true, jeśli napis @p arg reprezentuje poprawną
 * liczbę w systemie uint, a @p false w przeciwnym razie.
 */
static bool is_fine_number(char **arg , int len)
{
    // Pomijamy doczepiony znak końca linii.
    len--;

    // Brak argumentu.
    if (len == 0)
        return true;
    // Liczba 0.
    else if (len == 1 && isdigit((*arg)[0]))
        return true;
    // Niepoprawna liczba z zerem wiodącym.
    else if (len > 1 && (*arg)[0] == '0')
        return false;
    // Argument za duży.
    else if (len > 10)
        return false;

    // Sprawdzenie czy argument składa się z samych cyfr.
    for (int i = 0; i < len; ++i)
    {
        if (isdigit((*arg)[i]) == false)
            return false;
    }

    return true;
}

/** @brief Funkcja sprawdza czy podany znak jest niedozwolony.
 * Funkcja pomocnicza w @ref analyse_line.
 * @param[in] c  – znak do sprawdzenia,
 * @return Wartość @p true, jeśli znak @p c reprezentuje niedozwolony
 * znak, a @p false w przeciwnym razie.
 */
static inline bool wrong_sign(char c)
{
    unsigned short _c = c;
    return _c < 33;
}

/** @brief Funkcja alokuje pamieć na napis.
 * Funkcja pomocnicza w @ref analyse_line.
 * @param[in] tab  – wskaźnik na bufor na napis.
 * @param[in] mem  – rozmiar bufora który ma zostać zaalokowany.
 * @return Wartość @p true jeśli alokacja zakończyła się
 * pomyślnie, a @p false w przeciwnym razie.
 */
static bool safe_challoc(char **tab, size_t mem)
{
    *tab = malloc(mem);
    if (mem > 0 && *tab == NULL)
        return false;
    return true;
}

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
                 int *arguments, bool *memory_error)
{
    // Alokacja pamięci na komendę i argumenty.
    size_t mem = read * sizeof(char);
    if (safe_challoc(command, mem) == false)
    {
        *memory_error = true;
        return -1;
    }
    if (safe_challoc(arg1, mem) == false)
    {
        free(command);
        *memory_error = true;
        return -1;
    }
    if (safe_challoc(arg2, mem) == false)
    {
        free(command);
        free(arg1);
        *memory_error = true;
        return -1;
    }
    if (safe_challoc(arg3, mem) == false)
    {
        free(command);
        free(arg1);
        free(arg2);
        *memory_error = true;
        return -1;
    }
    if (safe_challoc(arg4, mem) == false)
    {
        free(command);
        free(arg1);
        free(arg2);
        free(arg3);
        *memory_error = true;
        return -1;
    }

    // Sprawdzenie skrajnych przypadków - pomijanie linii z komendą.
    if (read == 0 || line[0] == '#' || (read == 1 && line[0] == '\n'))
        return -1;
    else if (line[read - 1] != '\n')
        return 0;

    for (int i = 0; i < read; ++i)
        if (wrong_sign(line[i]) && isspace(line[i]) == false)
            return 0;

    // Krojenie linii z getline'a na komendę i argumenty.
    // Stworzenie pomocniczych zmiennych liczących długość każdego argumentu i komendy.
    int i = 0;
    int cl = 0, al1 = 0, al2 = 0, al3 = 0, al4 = 0;

    // Białe znaki na poczatku.
    if (i < read && isspace(line[i]))
        return 0;

    while (i < read && isspace(line[i]) == false)
        (*command)[cl++] = line[i++];

    while (i < read && isspace(line[i]))
        i++;
    while (i < read && isspace(line[i]) == false)
        (*arg1)[al1++] = line[i++];

    while (i < read && isspace(line[i]))
        i++;
    while (i < read && isspace(line[i]) == false)
        (*arg2)[al2++] = line[i++];

    while (i < read && isspace(line[i]))
        i++;
    while (i < read && isspace(line[i]) == false)
        (*arg3)[al3++] = line[i++];

    while (i < read && isspace(line[i]))
        i++;
    while (i < read && isspace(line[i]) == false)
        (*arg4)[al4++] = line[i++];

    while (i < read && isspace(line[i]))
        i++;
    if (i != read)
        return 0;

    // Dodanie końcowki umożliwiającej poprawne porównywanie stringów
    (*command)[cl++] = '\0';
    (*arg1)[al1++] = '\0';
    (*arg2)[al2++] = '\0';
    (*arg3)[al3++] = '\0';
    (*arg4)[al4++] = '\0';
    *com_len = cl;
    *arg_len1 = al1;
    *arg_len2= al2;
    *arg_len3 = al3;
    *arg_len4 = al4;

    *arguments = (al1 > 1) + (al2 > 1) + (al3 > 1) + (al4 > 1);

    return cl == 2 && is_fine_number(arg1, al1) && is_fine_number(arg2, al2)
           && is_fine_number(arg3, al3) && is_fine_number(arg4, al4);
}