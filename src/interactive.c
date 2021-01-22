/** @file
 * Implementacja fukncji umożliwiajacych rozgrywkę
 * w gre Gamma w trybie interaktywnym.
 *
 * @author Grzegorz Bogusław Zaleski (418494)
 * @copyright Uniwersytet Warszawski
 * @date 22 maja 2020
 */

/**
 * Stała oznaczająca naciśnięcie kombinacji klawiszy Ctrl-D.
 */
#define INSTANT_END '\4'

/**
 * Stała wchodząca w skład strzałek w systemie ANSI.
 */
#define ESC '\033'

#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include "gamma.h"

/**
 * Numer wiersza na którym znajduje się aktualnie kursor.
 */
static uint cursor_x;

/**
 * Numer kolumny na którym znajduje się aktualnie kursor.
 */
static uint cursor_y;

/**
 * Szerekość planszy.
 */
static uint width;

/**
 * Wysokość planszy.
 */
static uint height;

/**
 * Pomocnicza zmienna do trzymania ustawień konsoli
 * sprzed wywowałaniem funkcji @ref interactive_game.
 */
static struct termios old_term;

/**
 * Pomocnicze zmienna do modyfikowania ustawień konsoli
 * podczas działania funkcji @ref interactive_game.
 */
static struct termios new_term;

/** @brief Przygotowywuje konsole do trybu interaktywnego.
 */
static void setup_console()
{
    tcgetattr(STDIN_FILENO, &old_term);
    new_term = old_term;
    new_term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
    printf("\x1b[?25l");
}

/** @brief Wycofuje zmiany w konsoli.
 */
static void restore_console()
{
    printf("\x1b[?25h");
    printf("\x1b[0m");
    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
}

/** @brief Czyści konsole.
 */
static void clear_screen()
{
    printf("\033[2J");
    printf("\033[J");
    printf("\033[H");
    printf("\033[1;1H");
}

/** @brief Funkcja pobiera wartość długości terminala.
 *  @return długość terminala.
 */
uint terminal_width()
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
}

/** @brief Funkcja pobiera wartość wysokości terminala.
 *  @return wysokość terminala.
 */
uint terminal_height()
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_row;
}

/** @brief Ruch kursora w lewo.
 */
inline static void left_move()
{
    if (cursor_y != 0)
        cursor_y--;
}

/** @brief Ruch kursora w prawo.
 */
inline static void right_move()
{
    if (cursor_y + 1 != width)
        cursor_y++;
}

/** @brief Ruch kursora w góre.
 */
inline static void up_move()
{
    if (cursor_x != 0)
        cursor_x--;
}

/** @brief Ruch kursora w dół.
 */
inline static void down_move()
{
    if (cursor_x + 1 != height)
        cursor_x++;
}

/** @brief Wyświetla aktualny stan planszy.
 * Funkcja najpierw czyści konsole, potem wyświetla aktualny stan planszy.
 * Część pomocnicza w @ref interactive_game.
 * @param[in] game             – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] current_player   – identyfikator gracza, którego jest tura.
 * @param[in] margin         – zmienna pomocnicza do środkowa planszy,
 *                           tj. odległość planszy od lewej ściany terminala.
 */
inline static void show(gamma_t *game, uint current_player, uint margin)
{
    clear_screen();
    show_board(game, cursor_x, cursor_y, current_player, margin);
}

/** @brief Sprawdzenie czy na planszy są pola niezajęte przez graczy.
 * Funkcja pomocnicza w @ref interactive_game.
 * @param[in] game   – wskaźnik na strukturę przechowującą stan gry.
 * @return Wartość @p true, jeśli są jeszcze niezajęte pola w tej rozgrywce,
 * a @p false w przeciwnym przypadku.
 */
inline static bool free_fields_available(gamma_t *game)
{
    muint area = width;
    area *= height;
    return gamma_all_busy_fields(game) < area;
}

/** @brief Sprawdzenie czy można kontynuowąć rozgrywkę.
 * Funkcja pomocnicza w @ref interactive_game.
 * @param[in] game     – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] players  – liczba graczy biorących udział w grze.
 * @return Wartość @p true, jeśli są jeszcze niezajęte pola w tej rozgrywce,
 * a @p false w przeciwnym przypadku.
 */
inline static bool game_in_progress(gamma_t *game, uint players)
{
    if (free_fields_available(game))
        return true;
    for (uint i = 1; i <= players; ++i)
        if (gamma_golden_possible(game, i))
            return true;
    return false;
}

/** @brief Wyświetla aktualny stan gry gracza @p current_player.
 * Wyświetlana jest identyfikator @p current_player, liczba zajętych przez tego
 * gracza obszarów, maksymalna liczba obszarów (@p max_areas) oraz liczba
 * zdobytych punktów (pól na mapie).
 * Funkcja pomocnicza w @ref interactive_game.
 * @param[in] game           – wskaźnik na strukturę przechowującą stan gry.
 * @param[in] current_player – identyfikator gracza,
 * @param[in] current_player – identyfikator gracza,
 * @param[in] max_areas      – maksymalna liczba obszarów,
 *                              jakie może zająć jeden gracz,
 * @param[in] margin         – zmienna pomocnicza do środkowa planszy,
 *                              tj. odległość planszy od lewej ściany terminala.
 */
static void general_info(gamma_t *game, uint current_player, uint max_areas, uint margin)
{
    uint cur_areas = gamma_areas(game, current_player);
    uint cur_fields = gamma_busy_fields(game, current_player);

    // Poniższe spacje przed kodem koloru
    // mają na celu zrównąć połeżenie wszystkich poleceń.
    push(margin);
    printf("        \e[1;36mGame Status:\n");

    push(margin);
    printf("         \e[0;37mPlayer:\e[1;37m %d\n", current_player);

    push(margin);
    printf("         \e[0;37mPoints:\e[1;37m %d\n", cur_fields);

    push(margin);
    printf("          \e[0;37mAreas: \e[1;32m");
    if (cur_areas == max_areas)
        printf("\e[1;31m");
    printf("%d/%d\n", cur_areas, max_areas);

    push(margin);
    if (gamma_golden_possible(game, current_player))
        printf("\e[0;37mGolden Possible: \e[1;32mYes\n");
    else
        printf("\e[0;37mGolden Possible: \e[1;31mNo\n");
    printf("\033[0m");
}

/** @brief Funkcja wywołuje funkcje @ref gamma_move.
 * Na pozycji kursora wywoływania jest funkcja @ref gamma_move.
 * dla gracza @p current_player.
 * Funkcja pomocnicza w @ref interactive_game.
 * @param[in,out] game           – wskaźnik na strukturę przechowującą stan gry.
 * @param[in] current_player – identyfikator gracza,
 * @return Wartość @p true, jeśli ruch został poprawnie wykonany,
 * a @p false w przeciwnym razie.
 */
inline static bool hit(gamma_t *game, uint current_player)
{
    return gamma_move(game, current_player, cursor_y, cursor_x);
}

/** @brief Funkcja wywołuje funkcje @ref gamma_golden_move.
 * Na pozycji kursora wywoływania jest funkcja @ref gamma_golden_move.
 * dla gracza @p current_player.
 * Funkcja pomocnicza w @ref interactive_game.
 * @param[in,out] game       – wskaźnik na strukturę przechowującą stan gry.
 * @param[in] current_player – identyfikator gracza,
 * @return Wartość @p true, jeśli złoty ruch został poprawnie wykonany,
 * a @p false w przeciwnym razie.
 */
inline static bool golden_hit(gamma_t *game, uint current_player)
{
    return gamma_golden_move(game, current_player, cursor_y, cursor_x);
}

/** @brief Funkcja pomocznicza do wywoływania ruchu kolejnego gracza.
 * Wywałowana jeśli jeśli @p current_player poprawnie wykonał swój ruch
 * i następna iteracja pętli powinna dotyczyć kolejnego gracza.
 * Funkcja dba także o to żeby po ostatnim graczu,
 * o identyfikatorze @p players, kolejka zapętliła się.
 * Funkcja pomocnicza w @ref interactive_game.
 * @param[in, out] current_player – wskaźnik na identyfikator gracza,
 * @param[in] players             – liczba gracy biorących udział w grze.
 */
static void next_turn(uint *current_player, uint players)
{
    (*current_player)++;
    if (*current_player > players)
    {
        *current_player = 1;
    }
}

/** @brief Funkcja sprawdza czy dany gracz może brać udział w grze.
 * Sprawdzane jest czy gracz @p current_player ma niewykorzystany złoty ruch
 * oraz czy dostępne wolne pola na których może wykonać @ref hit.
 * Funkcja pomocnicza w @ref interactive_game.
 * @param[in] game           – wskaźnik na strukturę przechowującą stan gry.
 * @param[in] current_player – identyfikator gracza,
 * @return Wartość @p true, jeśli gracz może brać aktywny udział w grze,
 * a @p false w przeciwnym razie.
 */
inline static bool player_can_play(gamma_t *game, uint current_player)
{
    return gamma_golden_possible(game, current_player)
        || gamma_free_fields(game, current_player);
}

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
 *                      jakie może zająć jeden gracz.
 * @return Wartość @p true, jeśli rozgrywka została pomyślnie przeprowadzona,
 * albo @p false jeśli nastąpił niespodziewany błąd.
 */
bool interactive_game(gamma_t *game, uint _width, uint _height, uint players, uint max_areas)
{
    // Sprawdzenie czy gra się miesci w terminalu
    uint game_len = ((players > WIDE) + ceil_log(players)) * _width + 2;
    if (terminal_width() < game_len + 4 || terminal_height() < _height + players + 6)
    {
       printf("\e[1;31mError - Terminal screen not large enough.\n\033[0m");
       return false;
    }

    // Inicjalizacja zmiennych dla trybu interaktywnego.
    width = _width;
    height = _height;
    cursor_y = width/2;
    cursor_x = height/2;
    setup_console();
    bool fine_move;
    char command;
    uint current_player = 1;
    uint margin = (terminal_width() - game_len + 1)/2;

    // Gra sie toczy dopóki są możliwe ruchy.
    while (game_in_progress(game, players))
    {
        show(game, current_player, margin);
        general_info(game, current_player, max_areas, terminal_width()/2 - 15);
        fine_move = false;

        // Sprawdzenie czy gracz może grać w tej turze.
        if (player_can_play(game, current_player))
        {
            // Jeśli może to wczytanie jego ruchu.
            command = getchar();
        }
        else
        {
            // W przeciwnym razie jego tura jest pomijana.
            command = 'c';
        }

        // Analiza wczytanego ruchu:
        // Przemieszczenie kursora w odpowiednią stronę.
        if (command == ESC)
        {
            command = getchar();
            if (command == '[')
            {
                command = getchar();
                if (command == 'A')
                {
                    down_move();
                }
                else if (command == 'B')
                {
                    up_move();
                }
                else if (command == 'C')
                {
                    right_move();
                    // Zmiana wartości jest niezbędna, żeby nie
                    // wykonywało się pominięcie tury.
                    command = 'x';
                }
                else if (command == 'D')
                {
                    left_move();
                }
            }
        }
        // Złoty ruch.
        if (command == 'G' || command == 'g')
        {
            fine_move = golden_hit(game, current_player);
        }
        // Zwykły ruch.
        else if (command == ' ')
        {
            fine_move = hit(game, current_player);
        }
        // Przerwanie gry.
        else if (command == INSTANT_END)
        {
            break;
        }
        // Pominięcie tury.
        else if (command == 'C' || command == 'c')
        {
            fine_move = true;
        }
        // Sprawdzenie czy w kolejnej iteracji turę ma następny gracz.
        if (fine_move)
        {
            next_turn(&current_player, players);
        }
    }

    // Wypisanie wyników.
    uint best_result = gamma_best_result(game);
    clear_screen();
    show_board(game, width, height, players+1, margin);

    push(terminal_width()/2 - 6);
    printf("\e[1;36mFinal Results:\033[0m\n");
    for (current_player = 1; current_player <= players; ++current_player)
    {
        push(terminal_width()/2 - 8);
        if (gamma_busy_fields(game, current_player) != best_result)
        {
            printf("Player %d - %ld pts.\n", current_player,
                   gamma_busy_fields(game, current_player));
        }
        else
        {
            // Gracz lub gracze z najlepszym wynikiem są podkreśleni na złoto.
            printf("\e[1;33mPlayer %d - %ld pts.\n", current_player,
                   gamma_busy_fields(game, current_player));
            printf("\x1b[0m");
        }
    }

    // Przywrócenie zwykłych ustawienień konsoli.
    restore_console();
    return true;
}