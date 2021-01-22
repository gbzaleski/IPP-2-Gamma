/** @file
 * Implementacja silnika gry Gamma.
 *
 * @author Grzegorz Bogusław Zaleski (418494)
 * @copyright Uniwersytet Warszawski
 * @date 22 maja 2020
 */
#include <stdlib.h>
#include "gamma.h"
#include "stdio.h"

/**
 * Liczba kierunków (wraz z zwrotami) według
 * których można się poruszać po planszy 2D.
 */
#define DIRECTIONS 4

/**
 * Stała oznaczająca puste pole na planszy.
 */
#define EMPTY 0

/**
 * Stała do poruszanie się po planszy horyzontalnie.
 * Wykorzystawane przy funkcjach wzorowanych na DFS.
 */
static const uint X[] = {1,0,-1,0};

/**
 * Stała do poruszanie się po planszy wertykalnie.
 * Wykorzystawane przy funkcjach wzorowanych na DFS.
 */
static const uint Y[] = {0,1,0,-1};

/**
 * Stała do bezpiecznej transformacji liczb do znaków.
 * Wykorzystywana w @ref gamma_board oraz @ref gamma_spaced_board
 */
static const char digits[] = "0123456789";

/** @brief Struktura gracza gry Gamma
 * Zawiera aktualny stan gry jednego gracza.
 */
typedef struct player
{
    muint fields; ///< Liczba pól które gracz aktualnie zajmuje.
    uint areas; ///< Liczba obszarów które gracz aktualnie zajmuje
    bool free_golden_move; ///< Czy gracz nie wykonał jeszcze złotego ruchu.
    muint next_ind; /**< Zmienna pomocniczna, wolny indeks, do zajęcia
        prez nastepny obszar zajęty przez tego gracza. */
    muint border; ///< Liczba pustych pól które graniczą z polami gracza.
} player;

/** @brief Główna struktura gry Gamma.
 * Zawiera wszystkie informacje o aktualnej rozgrywce.
 */
typedef struct gamma
{
    uint **board; /**< Plansza gry, przechowuje informacje który,
        gracza zajął dane pole lub czy jest ono puste.*/
    muint **indexes; /**< Pomocnicza tablica do zliczania obszarów
        zajętych przez graczy. */
    uint width; ///< Szerokość planszy.
    uint heigth; ///< Wysokość planszy.
    player *players; /**< Tablica z aktualnymi informacjami
        dotyczącymi każdego gracza. */
    uint number_of_players; /**< Liczba graczy grających w grę. */
    uint max_areas; /**< Maksymalna liczba obszarów które,
        może zająć jeden gracz. */
    muint busy_fields; /**< Liczba zajętych pól na planszy.
        przez wszystkich graczy */
    muint fields_of_wider_players; /**< Pomocnicza zmienna do zliczania pól
        zajętych przez graczy z przynajmniej dwucyfrowym indeksem. */
    uint golden_moves_used; /**< Pomocnicza zmienna do zliczania
        wykorzystanych złotych ruchów */
} gamma_t;

/** @brief Alokuje pamieć na plansze do gry.
 * Tworzy pustą planszę 2D, jeśli podczas alokacji
 *      pamięci zabraknie pamięci usuwa planszę jak i całą grę.
 * @param[in,out] game – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] n        – wysokość planszy, liczba dodatnia,
 * @param[in] m        – szerokość planszy, liczba dodatnia,
 * @return Wartość @p true, jeśli wszystkie alokacje zostały wykonane poprawnie,
 *      a @p false w przeciwnym przypadku.
 */
static bool init_board(gamma_t *game, uint n, uint m)
{
    if (game == NULL)
        return false;

    game->board = malloc(n * sizeof(uint*));
    if (game->board == NULL)
    {
        free(game);
        return false;
    }

    for (uint x = 0; x < n; ++x)
    {
        game->board[x] = malloc(m * sizeof(uint));
        if (game->board[x] == NULL)
        {
            for (uint i = 0; i < x-1; ++i)
            {
                free(game->board[i]);
            }
            free(game->board);
            free(game->players);
            free(game);
            return false;
        }
        for (uint y = 0; y < m; ++y)
            game->board[x][y] = EMPTY;
    }
    return true;
}

/** @brief Alokuje pamieć na pomocniczą plansze indeksów.
 * Funkcja analogiczna do init_board, tworzy pustą planszę 2D,
 * jeśli podczas alokacji pamięci zabraknie pamięci usuwa planszę i całą grę.
 * @param[in,out] game – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] n        – wysokość planszy, liczba dodatnia,
 * @param[in] m        – szerokość planszy, liczba dodatnia,
 * @return Wartość @p true, jeśli wszystkie alokacje zostały wykonane poprawnie,
 *      a @p false w przeciwnym przypadku.
 */
static bool init_indexes(gamma_t *game, uint n, uint m)
{
    if (game == NULL)
        return false;

    game->indexes = malloc(n * sizeof(muint*));
    if (game->indexes == NULL)
    {
        free(game);
        return false;
    }

    for (uint x = 0; x < n; ++x)
    {
        game->indexes[x] = malloc(m * sizeof(muint));

        if (game->indexes[x] == NULL)
        {
            for (uint i = 0; i < x - 1; ++i)
                free(game->indexes[i]);
            for (uint i = 0; i < n; ++i)
                free(game->board[i]);
            free(game->board);
            free(game->indexes);
            free(game->players);
            free(game);
            return false;
        }

        for (uint y = 0; y < m; ++y)
            game->indexes[x][y] = EMPTY;
    }
    return true;
}

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
gamma_t *gamma_new(uint width, uint height, uint players, uint areas)
{
    if (width == 0 || height == 0 || players == 0 || areas == 0)
        return NULL;

    gamma_t *game = malloc(sizeof(gamma_t));
    if (game == NULL)
    {
        return NULL;
    }

    // Alokacja na (players+1) graczy jest niezbędna
    // ponieważ, są oni indeksowani od 1.
    // Konwersja na muint jest wymagana na wypadek jeśli iloczyn liczby
    // graczy+1 i rozmiaru struktury gracza przekracza zakres zmiennej uint.
    game->players = malloc((((muint) players)+1) * sizeof(player));
    if (game->players == NULL)
    {
        free(game);
        return NULL;
    }
    for (uint i = 1; i <= players; ++i)
    {
        game->players[i].areas = 0;
        game->players[i].fields = 0;
        game->players[i].free_golden_move = true;
        game->players[i].next_ind = 1;
        game->players[i].border = 0;
    }

    if (init_board(game, width, height) == false
        || init_indexes(game, width, height) == false)
    {
        return NULL;
    }

    game->width = width;
    game->heigth = height;
    game->max_areas = areas;
    game->number_of_players = players;
    game->busy_fields = 0;
    game->fields_of_wider_players = 0;
    game->golden_moves_used = 0;
    return game;
}

/** @brief Usuwa strukturę przechowującą stan gry.
 * Usuwa z pamięci strukturę wskazywaną przez @p g.
 * Nic nie robi, jeśli wskaźnik ten ma wartość NULL.
 * @param[in] game    – wskaźnik na usuwaną strukturę.
 */
void gamma_delete(gamma_t *game)
{
    if (game != NULL)
    {
        for (uint i = 0; i < game->width; ++i)
        {
            free(game->board[i]);
            free(game->indexes[i]);
        }
        free(game->board);
        free(game->indexes);
        free(game->players);
        free(game);
    }
}

/** @brief Sprawdza czy podano poprawny indeks gracza.
* Sprawdza czy podana liczba jest zgodna ze specyfikacja ogolną gry oraz
*       czy taki gracz bierze udział w aktualnej grze
* @param[in] player   – numer gracza, liczba dodatnia niewiększa od wartości
*                      @p number_of_players z struktury @ref gamma_t,
* @param[in] game    – wskaźnik na strukturę przechowującą stan gry,
* @return Wartość @p true, jeśli taki gracz bierze udział
*     w grze, a @p false, w przeciwnym przypadku
*/
inline static bool player_is_fine(uint player, gamma_t *game)
{
    return 0 < player && player <= game->number_of_players;
}

/** @brief Wstępnie sprawdza, czy gracz może wykonać złoty ruch.
 * Sprawdza, czy gracz @p player jeszcze nie wykonał w tej rozgrywce złotego
 * ruchu i jest przynajmniej jedno pole zajęte przez innego gracza.
 * Funkcja pomocnicza w
 * @param[in] game    – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new.
 * @return Wartość @p true, jeśli gracz jeszcze nie wykonał w tej rozgrywce
 * złotego ruchu i jest przynajmniej jedno pole zajęte przez innego gracza,
 * a @p false w przeciwnym przypadku.
 */
static bool gamma_golden_possible_con(gamma_t *game, uint player)
{
    return game != NULL && player_is_fine(player, game)
           && game->players[player].free_golden_move
           && game->players[player].fields < game->busy_fields;
}

/** @brief Sprawdza czy podano poprawne położenie na planszy gry.
* Sprawdza czy podana para liczb jest zgodna ze specyfikajca ogólną gry oraz
*       czy takie pole znajduje się w bieżącej rozgrywce.
* @param[in] x        – numer kolumny, liczba nieujemna mniejsza od wartości
*                      @p width z funkcji @ref gamma_new,
* @param[in] y        – numer wiersza, liczba nieujemna mniejsza od wartości
*                      @p height z funkcji @ref gamma_new.
* @param[in] game – wskaźnik na strukturę przechowującą stan gry,
* @return Wartość @p true, jeśli ta para poprawnie opisuje
*        pole z gry @p game a @p false, w przeciwnym przypadku.
*/
inline static bool coords_are_fine(uint x, uint y, gamma_t *game)
{
    return x < game->width && y < game->heigth;
}

/** @brief Sprawdza czy koło podanego pola jest pole gracza @p player.
 * Sprawdzane jest czy jedno sąsiądnich pól wzgledem
 * pola (@p x, @p y) nalezy do gracza @p player.
 * Funkcja pomocniczna w @ref gamma_golden_possible.
 * @param[in] game   – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player – numer gracza, liczba dodatnia niewiększa od wartości
 *                          @p players z funkcji @ref gamma_new,
 * @param[in] x      – numer kolumny, liczba nieujemna mniejsza od wartości
 *                          @p width z funkcji @ref gamma_new,
 * @param[in] y      – numer wiersza, liczba nieujemna mniejsza od wartości
 *                         @p height z funkcji @ref gamma_new,
 * @return Wartość @p true, jeśli takie pole istieje, a @p false w
 * przeciwnym przypadku.
 */
static bool neighbourhood_is_fine(gamma_t *game, uint player, uint x, uint y)
{
    for (uint i = 0; i != DIRECTIONS; ++i)
    {
        uint _x = x + X[i];
        uint _y = y + Y[i];
        if (coords_are_fine(_x, _y, game))
        {
            if (game->board[_x][_y] == player)
                return true;
        }
    }
    return false;
}

/** @brief Przeindeksowywuje obszar.
 * Zmienia indeks pola (@p x, @p y) z @p from na @p to o ile jest to możliwe,
 * tj. pole jest poprawne a jego włascicielem jest gracz @p player.
 * Funkcja pomocniczna w @ref gamma_move i @ref gamma_golden_move,
 * jest rekursywnie wykonywana dla całego obszaru o tym samym indeksie.
 * @param[in,out] game – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player   – numer gracza, liczba dodatnia niewiększa od wartości
 *                          @p players z funkcji @ref gamma_new,
 * @param[in] x        – numer kolumny, liczba nieujemna mniejsza od wartości
 *                          @p width z funkcji @ref gamma_new,
 * @param[in] y        – numer wiersza, liczba nieujemna mniejsza od wartości
 *                         @p height z funkcji @ref gamma_new,
 * @param[in] from     – indeks który jest usuwany,
 * @param[in] to       – nowy indeks dla pól których identyfikator jest usuwany.
 */
static void reindexify(gamma_t *game, uint player, uint x, uint y, muint from, muint to)
{
    if (coords_are_fine(x, y, game)
        && game->board[x][y] == player
        && game->indexes[x][y] == from)
    {
        game->indexes[x][y] = to;

        for (uint i = 0; i != DIRECTIONS; ++i)
        {
            uint _x = x + X[i];
            uint _y = y + Y[i];
            reindexify(game, player, _x, _y, from, to);
        }
    }
}

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
bool gamma_golden_possible(gamma_t *game, uint player)
{
    // Czy zachodzi warunek konieczny.
    if (gamma_golden_possible_con(game, player) == false)
        return false;

    // Czy gracz ma jeszcze zapas obszarów i moze
    // zajać jakies pole nierozpinające.
    if (game->players[player].areas < game->max_areas)
        return true;

    for (uint x = 0; x < game->width; ++x)
    {
        for (uint y = 0; y < game->heigth; ++y)
        {
            // Zajmuje inny gracz to pole.
            if (game->board[x][y] != EMPTY && game->board[x][y] != player)
            {
                // Gracz może dołączyć to pole do swoich obszarów.
                if (neighbourhood_is_fine(game, player, x, y))
                {
                    uint outsider = game->board[x][y];
                    uint old_ind = game->indexes[x][y];
                    uint new_ind = game->players[outsider].next_ind++;
                    game->indexes[x][y] = new_ind;
                    int cnt = -1;
                    for (int i = 0; i < DIRECTIONS; ++i)
                    {
                        uint _x = x + X[i];
                        uint _y = y + Y[i];
                        if (coords_are_fine(_x, _y, game)
                            && game->board[_x][_y] == outsider
                            && game->indexes[_x][_y] == old_ind)
                        {
                            cnt++;
                            reindexify(game, outsider, _x, _y, old_ind, new_ind);
                        }
                    }

                    if (game->players[outsider].areas + cnt <= game->max_areas)
                        return true;
                }
            }
        }
    }

    return false;
}

/** @brief Podaje liczbę pól zajętych przez gracza.
 * Podaje liczbę pól zajętych przez gracza @p player.
 * @param[in] game    – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new.
 * @return Liczba pól zajętych przez gracza lub zero,
 * jeśli któryś z parametrów jest niepoprawny.
 */
muint gamma_busy_fields(gamma_t *game, uint player)
{
    if (game != NULL && player_is_fine(player, game))
    {
        return game->players[player].fields;
    }
    return 0;
}

/** @brief Podaje liczbę pól, jakie jeszcze gracz może zająć.
 * Podaje liczbę wolnych pól, na których w danym stanie gry gracz @p player może
 * postawić swój pionek w następnym ruchu.
 * @param[in] game    – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new.
 * @return Liczba pól, jakie jeszcze może zająć gracz lub zero,
 * jeśli któryś z parametrów jest niepoprawny.
 */
muint gamma_free_fields(gamma_t *game, uint player)
{
    if (game != NULL && player_is_fine(player, game))
    {
        if (game->players[player].areas < game->max_areas)
        {
            muint free_fields = game->width;
            free_fields *= game->heigth;
            free_fields -= game->busy_fields;
            return free_fields;
        }
        else
        {
            return game->players[player].border;
        }
    }
    return 0;
}

/** @brief Aktualizuje liczbę sąsiednich wolnych pól.
 * Subfunkcja pomocniczna dla @ref update_positive_border.
 * @param[in,out] game   – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player     – numer gracza, liczba dodatnia niewiększa od wartości
 *                          @p players z funkcji @ref gamma_new,
 * @param[in] x          – numer kolumny, liczba nieujemna mniejsza od wartości
 *                          @p width z funkcji @ref gamma_new,
 * @param[in] y          – numer wiersza, liczba nieujemna mniejsza od wartości
 *                         @p height z funkcji @ref gamma_new.
 */
static void update_positive_blank(gamma_t *game, uint player, uint x, uint y)
{
    if (game->board[x][y] != EMPTY)
        return;

    for (uint i = 0; i != DIRECTIONS; ++i)
    {
        uint _x = x + X[i];
        uint _y = y + Y[i];
        if (coords_are_fine(_x, _y, game) && game->board[_x][_y] == player)
            return;
    }
    game->players[player].border++;
}

/** @brief Aktualizuje liczbę sąsiednich wolnych pól.
 * Sprawdza jak zwolnienie pola (@p x, @p y) wpływa na liczbę
 * dostępnych wolnych pól sąsiadów tego pola, nastepnie
 * aktualizuje ten parametr u wszystkich sąsiadów tego pola.
 * Funkcja pomocnicza dla @ref gamma_move.
 * @param[in,out] game   – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player     – numer gracza, liczba dodatnia niewiększa od wartości
 *                          @p players z funkcji @ref gamma_new,
 * @param[in] x          – numer kolumny, liczba nieujemna mniejsza od wartości
 *                          @p width z funkcji @ref gamma_new,
 * @param[in] y          – numer wiersza, liczba nieujemna mniejsza od wartości
 *                         @p height z funkcji @ref gamma_new.
 */
static void update_positive_border(gamma_t *game, uint player, uint x, uint y)
{
    uint neighbours[DIRECTIONS];

    for (uint i = 0; i != DIRECTIONS; ++i)
    {
        uint _x = x + X[i];
        uint _y = y + Y[i];
        if (coords_are_fine(_x, _y, game))
        {
            neighbours[i] = game->board[_x][_y];
            update_positive_blank(game, player, _x, _y);
        }
        else
        {
            neighbours[i] = EMPTY;
        }
    }

    for (uint i = 0; i != DIRECTIONS; ++i)
    {
        uint current_neighbour = neighbours[i];
        if (current_neighbour != EMPTY)
        {
            game->players[current_neighbour].border--;
            for (uint j = i; j < DIRECTIONS; ++j)
            {
                if (neighbours[j] == current_neighbour)
                    neighbours[j] = EMPTY;
            }
        }
    }
}

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
bool gamma_move(gamma_t *game, uint player, uint x, uint y)
{
    if (game != NULL && coords_are_fine(x, y, game)
        && player_is_fine(player, game)
        && game->board[x][y] == EMPTY)
    {
        for (uint i = 0; i < DIRECTIONS; ++i)
        {
            uint _x = x + X[i];
            uint _y = y + Y[i];
            if (coords_are_fine(_x, _y, game) && game->board[_x][_y] == player)
            {
                if (game->indexes[x][y] == EMPTY)
                {
                    game->indexes[x][y] = game->indexes[_x][_y];
                    game->board[x][y] = player;
                    game->players[player].fields++;
                    game->busy_fields++;
                    if (WIDE < player)
                        game->fields_of_wider_players++;
                }
                else if (game->indexes[x][y] != game->indexes[_x][_y])
                {
                    reindexify(game, player, _x, _y, game->indexes[_x][_y], game->indexes[x][y]);
                    game->players[player].areas--;
                }
            }
        }
        if (game->indexes[x][y] != EMPTY)
        {
            game->board[x][y] = EMPTY;
            update_positive_border(game, player, x, y);
            game->board[x][y] = player;
            return true;
        }
        else if (game->players[player].areas < game->max_areas)
        {
            game->indexes[x][y] = game->players[player].next_ind++;
            game->players[player].areas++;
            game->players[player].fields++;
            game->busy_fields++;
            if (WIDE < player)
                game->fields_of_wider_players++;
            game->board[x][y] = EMPTY;
            update_positive_border(game, player, x, y);
            game->board[x][y] = player;
            return true;
        }
    }
    return false;
}

/** @brief Oblicza logarytm dziesiętny zaokrąglony w góre do liczby całkowitej.
 * Funkcja pomocnicza do szacowania rozmiaru tablicy w @ref gamma_spaced_board,
 * @ref show_board oraz @ref interactive_game.
 * @param[in] x    – liczba logarytmowana.
 * @return Wynik tego działania.
 */
uint ceil_log(uint x)
{
    uint res = 0;
    while (x != 0)
    {
        res++;
        x /= 10;
    }
    return res;
}

/** @brief Daje napis opisujący stan plansz.
 * Alokuje w pamięci bufor, w którym umieszcza napis zawierający tekstowy
 * opis aktualnego stanu planszy. Ta funkcja w przeciwieństwie
 * do @ref gamma_board umieszcza spacje między kolejnymi identyfikatorami
 * graczy. Funkcja jest wywoływana kiedy w grze aktywnie biorą udział
 * gracze o wielocyfrowych identyfikatorach, których wypisanie
 * ciągiem byłoby dwuznaczne.
 * Funkcja wywołująca musi zwolnić ten bufor.
 * @param[in] game    – wskaźnik na strukturę przechowującą stan gry.
 * @return Wskaźnik na zaalokowany bufor zawierający napis opisujący stan
 * planszy lub NULL, jeśli nie udało się zaalokować pamięci.
 */
static char *gamma_spaced_board(gamma_t *game)
{
    //Alokacja szacunkowej ilości pamięci.
    muint len = game->width+1;
    len *= game->heigth+1;
    uint cell_len = ceil_log(game->number_of_players) + 1;
    len *= cell_len;
    char *board_display = malloc(len * sizeof(char));
    if (board_display == NULL)
        return NULL;

    uint pos = 0;
    for (uint x = 0; x < game->heigth; ++x)
    {
        for (uint y = 0; y < game->width; ++y)
        {
            uint val = game->board[y][game->heigth - x - 1];
            uint move = cell_len - 1;
            //Sprawdzenie czy pole jest puste - wtedy należy postawić kropke.
            bool is_empty = val == 0;

            for (uint i = 0; i < cell_len; ++i)
            {
                if (val != 0)
                {
                    board_display[move + pos++] = digits[val % 10];
                }
                else
                {
                    board_display[move + pos++] = ' ';
                }
                val /= 10;
                move -= 2;
            }
            if (is_empty)
            {
                board_display[pos - 1] = '.';
            }
        }
        board_display[pos++] = '\n';
    }

    board_display[pos++] = '\0';
    //Uwolnienie nadmiernej pamięci zaalokowanej przy szacowanie rozmiaru.
    board_display = realloc(board_display, pos * sizeof(char));
    return board_display;
}

/** @brief Daje napis opisujący stan planszy.
 * Alokuje w pamięci bufor, w którym umieszcza napis zawierający tekstowy
 * opis aktualnego stanu planszy.
 * Funkcja wywołująca musi zwolnić ten bufor.
 * @param[in] game    – wskaźnik na strukturę przechowującą stan gry.
 * @return Wskaźnik na zaalokowany bufor zawierający napis opisujący stan
 * planszy lub NULL, jeśli nie udało się zaalokować pamięci.
 */
char *gamma_board(gamma_t *game)
{
    if (game == NULL)
        return NULL;

    if (0 < game->fields_of_wider_players)
        return gamma_spaced_board(game);

    muint len = game->width+1;
    len *= game->heigth+1;
    char *board_display = malloc(len * sizeof(char));
    if (board_display == NULL)
        return NULL;

    uint pos = 0;
    for (uint x = 0; x < game->heigth; ++x)
    {
        for (uint y = 0; y < game->width; ++y)
        {
            uint val = game->board[y][game->heigth - x - 1];
            if (val == EMPTY)
            {
                board_display[pos++] = '.';
            }
            else
            {
                board_display[pos++] = digits[val];
            }
        }
        board_display[pos++] = '\n';
    }
    board_display[pos++] = '\0';
    // Uwolnienie nadmiernej pamięci zaalokowanej przy szacowanie rozmiaru.
    board_display = realloc(board_display, pos * sizeof(char));
    return board_display;
}

/** @brief Aktualizuje liczbę sąsiednich wolnych pól.
 * Sprawdza jak zwolnienie pola (@p x, @p y) wpływa na liczbę wolnych pól wokół tego pola,
 * nastepnie aktualizuje ten parametr u wszystkich sąsiadów tego pola.
 * Funkcja pomocnicza @ref gamma_golden_move.
 * @param[in,out] game   – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] x          – numer kolumny, liczba nieujemna mniejsza od wartości
 *                          @p width z funkcji @ref gamma_new,
 * @param[in] y          – numer wiersza, liczba nieujemna mniejsza od wartości
 *                         @p height z funkcji @ref gamma_new.
 */
static void update_blank_all_neighbours(gamma_t *game, uint x, uint y)
{
    uint neighbours[DIRECTIONS];

    for (uint i = 0; i != DIRECTIONS; ++i)
    {
        uint _x = x + X[i];
        uint _y = y + Y[i];
        if (coords_are_fine(_x, _y, game))
        {
            neighbours[i] = game->board[_x][_y];
        }
        else
        {
            neighbours[i] = EMPTY;
        }
    }

    for (uint i = 0; i != DIRECTIONS; ++i)
    {
        uint current_neighbour = neighbours[i];
        if (current_neighbour != EMPTY)
        {
            game->players[current_neighbour].border++;
            for (uint j = i; j < DIRECTIONS; ++j)
                if (neighbours[j] == current_neighbour)
                    neighbours[j] = EMPTY;
        }
    }
}

/** @brief Aktualizuje liczbę pustych pól sąsiadnich danego gracza.
 * Sprawdza jak utrata pola (@p x, @p y) wpływa na liczbę wolnych pól
 * wokoł obszarów zajętych przez gracza @p player_out.
 * Funkcja pomocnicza dla @ref gamma_golden_move.
 * @param[in,out] game   – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player_out – numer gracza, liczba dodatnia niewiększa od wartości
 *                         @p players z funkcji @ref gamma_new,
 * @param[in] x          – numer kolumny, liczba nieujemna mniejsza od wartości
 *                          @p width z funkcji @ref gamma_new,
 * @param[in] y          – numer wiersza, liczba nieujemna mniejsza od wartości
 *                         @p height z funkcji @ref gamma_new.
 */
static void lost_independent_borders(gamma_t *game, uint player_out, uint x, uint y)
{
    for (uint i = 0; i < DIRECTIONS; ++i)
    {
        uint _x = x + X[i];
        uint _y = y + Y[i];
        if (coords_are_fine(_x, _y, game) && game->board[_x][_y] == EMPTY)
        {
            bool field_is_connected = false;
            for (uint j = 0; j < DIRECTIONS && field_is_connected == false; ++j)
            {
                uint _x2 = _x + X[j];
                uint _y2 = _y + Y[j];
                if (coords_are_fine(_x2, _y2, game) && game->board[_x2][_y2] == player_out)
                    field_is_connected = true;
            }

            if (field_is_connected == false)
            {
                game->players[player_out].border--;
            }
        }
    }
}

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
bool gamma_golden_move(gamma_t *game, uint player, uint x, uint y)
{
    if (gamma_golden_possible_con(game, player) == false
        || coords_are_fine(x, y, game) == false
        || game->board[x][y] == EMPTY
        || game->board[x][y] == player)
        return false;

    uint player_out = game->board[x][y];
    uint index_out = game->indexes[x][y];
    uint new_index = game->players[player_out].next_ind;

    for (uint i = 0; i < DIRECTIONS; ++i)
    {
        uint _x = x + X[i];
        uint _y = y + Y[i];
        if (coords_are_fine(_x, _y, game) && game->board[_x][_y] == player_out)
        {
            if (game->board[x][y] == player_out)
            {
                game->board[x][y] = EMPTY;
                update_blank_all_neighbours(game, x, y);
                lost_independent_borders(game, player_out, x, y);
                game->indexes[x][y] = EMPTY;
                game->players[player_out].fields--;
                game->busy_fields--;
                if (WIDE < player)
                    game->fields_of_wider_players--;
                reindexify(game, player_out, _x, _y, index_out, new_index);
                game->players[player_out].next_ind++;
            }
            else if (game->board[x][y] == EMPTY && game->indexes[_x][_y] == index_out)
            {
                new_index = game->players[player_out].next_ind;
                reindexify(game, player_out, _x, _y, index_out, new_index);
                game->players[player_out].next_ind++;
                game->players[player_out].areas++;
            }
        }
    }

    if (game->board[x][y] == player_out)
    {
        game->board[x][y] = EMPTY;
        lost_independent_borders(game, player_out, x, y);
        update_blank_all_neighbours(game, x, y);
        game->indexes[x][y] = EMPTY;
        game->players[player_out].fields--;
        game->players[player_out].areas--;
        game->busy_fields--;
        if (WIDE < player)
            game->fields_of_wider_players--;
    }

    if (game->max_areas < game->players[player_out].areas)
    {
        gamma_move(game, player_out, x, y);
        return false;
    }
    else
    {
        if (gamma_move(game, player, x, y))
        {
            game->players[player].free_golden_move = false;
            game->golden_moves_used++;
            return true;
        }
        else
        {
            gamma_move(game, player_out, x, y);
            return false;
        }
    }
}

/** @brief Funkcja pomocznicza do przesuwania wyświetlanej treści.
 * Wypisuje na stdout @p margin spacji dzięki czemu następny stdout
 * jest przesunięty w prawo.
 * @param[in] margin – długośc przesunięcia, tj. liczba spacji,
*/
void push (uint margin)
{
    for (uint _ = 0; _ < margin; _++)
        printf(" ");
}

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
void show_board(gamma_t *game, uint cur_x, uint cur_y, uint player, uint margin)
{
    uint cell_size = ceil_log(game->number_of_players)
            + (WIDE < game->number_of_players);

    printf("\n");
    push(margin);
    printf("╔");
    for (uint i = 0; i < game->width * cell_size; i++)
        printf("═");
    printf("╗\n");

    for (uint x = 0; x < game->heigth; ++x)
    {
        push(margin);
        printf("║");
        for (uint y = 0; y < game->width; ++y)
        {
            uint val = game->board[y][game->heigth - x - 1];
            if (val == player)
            {
                printf("\e[0;97m");
                if (gamma_areas(game, player) < game->max_areas)
                    printf("\e[42m");
                else
                    printf("\e[41m");
            }

            if (game->heigth - x - 1 == cur_x && y == cur_y)
            {
                printf("\e[0;97m");
                printf("\x1b[44m");
                if (val == EMPTY)
                {
                    for (uint i = 1; i < cell_size; ++i)
                        printf(" ");
                    printf(".");
                }
                else
                {
                    for (uint i = ceil_log(val); i < cell_size; ++i)
                        printf(" ");
                    printf("%d", val);
                }
                printf("\x1b[0m");
            }
            else if (val == EMPTY)
            {
                for (uint i = 1; i < cell_size; ++i)
                    printf(" ");
                printf(".");
            }
            else
            {
                for (uint i = ceil_log(val); i < cell_size; ++i)
                    printf(" ");
                printf("%d", val);
            }

            printf("\033[0m");
        }
        printf("║\n");
    }

    push(margin);
    printf("╚");
    for (uint i = 0; i < game->width * cell_size; i++)
        printf("═");
    printf("╝\n");
}

/** @brief Zwraca liczbę wszystkich zajętych pól na planszy.
 * Funkcja pomocnicza w @ref interactive_game.
 * @param[in] game – wskaźnik na strukturę przechowującą stan gry.
 * @return Liczba zajętych pól.
 */
muint gamma_all_busy_fields(gamma_t *game)
{
    return game->busy_fields;
}

/** @brief Zwraca liczbę wszystkich obszarów gracza @p player.
 * Funkcja pomocnicza w @ref interactive_game.
 * @param[in] game   – wskaźnik na strukturę przechowującą stan gry.
 * @param[in] player – identyfikator gracza.
 * @return Liczba obszarów danego gracza.
 */
uint gamma_areas(gamma_t *game, uint player)
{
    return game->players[player].areas;
}

/** @brief Zwraca liczbe pól zajętych przez gracza, który zajął
 * ich najwięcej.
 * Funkcja pomocnicza w @ref interactive_game.
 * @param[in] game   – wskaźnik na strukturę przechowującą stan gry.
 * @return Liczba pól tego gracza.
 */
uint gamma_best_result(gamma_t *game)
{
    uint result = 0;
    for (uint i = 1; i <= game->number_of_players; ++i)
    {
        if (result < game->players[i].fields)
            result = game->players[i].fields;
    }
    return result;
}
