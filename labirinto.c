#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define SCREEN_WIDTH 40
#define SCREEN_HEIGHT 20
#define CELL_SIZE 2
#define NUM_ROWS (SCREEN_HEIGHT / CELL_SIZE)
#define NUM_COLS (SCREEN_WIDTH / CELL_SIZE)

#define EMPTY_CELL 0
#define WALL_CELL '#'
#define PLAYER1_CELL '1'
#define PLAYER2_CELL '2'
#define EXIT_CELL 'X'

int maze[NUM_ROWS][NUM_COLS];
int player_x, player_y;
int exit_x, exit_y;
int score1 = 0, score2 = 0;
double total_time1 = 0.0, total_time2 = 0.0;
pthread_mutex_t lock;
pthread_cond_t player_cond;
int current_player = 1;
int games_played = 0;

void initialize_maze()
{
    for (int i = 0; i < NUM_ROWS; i++)
    {
        for (int j = 0; j < NUM_COLS; j++)
        {
            maze[i][j] = WALL_CELL;
        }
    }
}

void carve_passages(int cx, int cy)
{
    int directions[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};

    // Shuffle directions
    for (int i = 0; i < 4; i++)
    {
        int r = rand() % 4;
        int temp[2];
        temp[0] = directions[i][0];
        temp[1] = directions[i][1];
        directions[i][0] = directions[r][0];
        directions[i][1] = directions[r][1];
        directions[r][0] = temp[0];
        directions[r][1] = temp[1];
    }

    for (int i = 0; i < 4; i++)
    {
        int nx = cx + directions[i][0] * 2;
        int ny = cy + directions[i][1] * 2;

        if (nx >= 0 && nx < NUM_COLS && ny >= 0 && ny < NUM_ROWS && maze[ny][nx] == WALL_CELL)
        {
            maze[cy + directions[i][1]][cx + directions[i][0]] = EMPTY_CELL;
            maze[ny][nx] = EMPTY_CELL;
            carve_passages(nx, ny);
        }
    }
}

void generate_maze()
{
    initialize_maze();

    int start_x = rand() % (NUM_COLS / 2) * 2;
    int start_y = rand() % (NUM_ROWS / 2) * 2;
    maze[start_y][start_x] = EMPTY_CELL;

    carve_passages(start_x, start_y);

    do
    {
        exit_x = rand() % (NUM_COLS / 2) * 2;
        exit_y = rand() % (NUM_ROWS / 2) * 2;
    } while (maze[exit_y][exit_x] != EMPTY_CELL || (exit_x == start_x && exit_y == start_y));

    maze[exit_y][exit_x] = EXIT_CELL;

    do
    {
        player_x = rand() % NUM_COLS;
        player_y = rand() % NUM_ROWS;
    } while (maze[player_y][player_x] != EMPTY_CELL);

    maze[player_y][player_x] = EMPTY_CELL; // Inicialmente vazio, sera preenchido depois
}

void print_maze()
{
    system("clear");

    for (int i = 0; i < NUM_ROWS; i++)
    {
        for (int j = 0; j < NUM_COLS; j++)
        {
            if (maze[i][j] == WALL_CELL)
                printf("%c", WALL_CELL);
            else if (maze[i][j] == EMPTY_CELL)
                printf(" ");
            else if (maze[i][j] == PLAYER1_CELL)
                printf("%c", PLAYER1_CELL);
            else if (maze[i][j] == PLAYER2_CELL)
                printf("%c", PLAYER2_CELL);
            else if (maze[i][j] == EXIT_CELL)
                printf("%c", EXIT_CELL);
        }
        printf("\n");
    }
}

int main()
{
    srand(time(NULL));
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&player_cond, NULL);

    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    pthread_t thread1, thread2;
    int player1 = 1;
    int player2 = 2;

    int fd1[2], fd2[2];
    pipe(fd1);
    pipe(fd2);

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("Erro ao criar processo filho");
        return 1;
    }

    if (pid == 0)
    {
        close(fd1[1]);
        close(fd2[0]);

        int player_num = player2;
        char player_cell = (player_num == 1) ? PLAYER1_CELL : PLAYER2_CELL;

        pthread_mutex_lock(&lock);

        generate_maze();
        maze[player_y][player_x] = player_cell;

        clock_t start_time = clock();

        pthread_mutex_unlock(&lock);

        while (true)
        {
            pthread_mutex_lock(&lock);
            print_maze();
            pthread_mutex_unlock(&lock);

            char input;
            printf("Jogador %d (%c), sua vez\n", player_num, player_cell);
            printf("Jogador %d (%c), use as teclas do teclado W (cima)/A (esquerda)/S (baixo)/D (direita) para mover: ", player_num, player_cell);

            scanf(" %c", &input);

            int next_x = player_x;
            int next_y = player_y;
            switch (input)
            {
            case 'W':
                next_y--;
                break;
            case 'w':
                next_y--;
                break;
            case 'S':
                next_y++;
                break;
            case 's':
                next_y++;
                break;
            case 'A':
                next_x--;
                break;
            case 'a':
                next_x--;
                break;
            case 'D':
                next_x++;
                break;
            case 'd':
                next_x++;
                break;
            default:
                printf("Entrada invalida!\n");
                continue;
            }

            pthread_mutex_lock(&lock);
            if (next_x >= 0 && next_x < NUM_COLS && next_y >= 0 && next_y < NUM_ROWS && maze[next_y][next_x] != WALL_CELL)
            {
                maze[player_y][player_x] = EMPTY_CELL;
                player_x = next_x;
                player_y = next_y;
                maze[player_y][player_x] = player_cell;
            }

            if (player_x == exit_x && player_y == exit_y)
            {
                clock_t end_time = clock();
                double time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;
                total_time2 += time_taken;
                score2++;
                printf("Jogador %d venceu %d partida em %.2f segundos\n", player_num, score2, time_taken);

                games_played++;
                pthread_cond_signal(&player_cond);
                pthread_mutex_unlock(&lock);
                break;
            }
            pthread_mutex_unlock(&lock);
        }

        pthread_mutex_lock(&mutex);
        write(fd2[1], &total_time2, sizeof(total_time2));
        write(fd2[1], &score2, sizeof(score2));
        read(fd1[0], &total_time1, sizeof(total_time1));
        read(fd1[0], &score1, sizeof(score1));
        pthread_mutex_unlock(&mutex);

        close(fd1[0]);
        close(fd2[1]);
    }
    else
    {
        close(fd1[0]);
        close(fd2[1]);

        int player_num = player1;
        char player_cell = (player_num == 1) ? PLAYER1_CELL : PLAYER2_CELL;

        pthread_mutex_lock(&lock);

        generate_maze();
        maze[player_y][player_x] = player_cell;

        clock_t start_time = clock();

        pthread_mutex_unlock(&lock);

        while (true)
        {
            pthread_mutex_lock(&lock);
            print_maze();
            pthread_mutex_unlock(&lock);

            char input;
            printf("Jogador %d (%c), sua vez\n", player_num, player_cell);
            printf("Jogador %d (%c), use as teclas do teclado W (cima)/A (esquerda)/S (baixo)/D (direita) para mover: ", player_num, player_cell);

            scanf(" %c", &input);

            int next_x = player_x;
            int next_y = player_y;
            switch (input)
            {
            case 'W':
                next_y--;
                break;
            case 'w':
                next_y--;
                break;
            case 'S':
                next_y++;
                break;
            case 's':
                next_y++;
                break;
            case 'A':
                next_x--;
                break;
            case 'a':
                next_x--;
                break;
            case 'D':
                next_x++;
                break;
            case 'd':
                next_x++;
                break;
            default:
                printf("Entrada invalida!\n");
                continue;
            }

            pthread_mutex_lock(&lock);
            if (next_x >= 0 && next_x < NUM_COLS && next_y >= 0 && next_y < NUM_ROWS && maze[next_y][next_x] != WALL_CELL)
            {
                maze[player_y][player_x] = EMPTY_CELL;
                player_x = next_x;
                player_y = next_y;
                maze[player_y][player_x] = player_cell;
            }

            if (player_x == exit_x && player_y == exit_y)
            {
                clock_t end_time = clock();
                double time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;
                total_time1 += time_taken;
                score1++;
                printf("Jogador %d venceu %d partida em %.2f segundos\n", player_num, score1, time_taken);

                games_played++;
                pthread_cond_signal(&player_cond);
                pthread_mutex_unlock(&lock);
                break;
            }
            pthread_mutex_unlock(&lock);
        }

        pthread_mutex_lock(&mutex);
        write(fd1[1], &total_time1, sizeof(total_time1));
        write(fd1[1], &score1, sizeof(score1));
        read(fd2[0], &total_time2, sizeof(total_time2));
        read(fd2[0], &score2, sizeof(score2));
        pthread_mutex_unlock(&mutex);

        close(fd1[1]);
        close(fd2[0]);
    }

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&player_cond);

    printf("\nPontuação final: Jogador 1: %d (total de tempo: %.2f segundos), Jogador 2: %d (total de tempo: %.2f segundos)\n", score1, total_time1, score2, total_time2);

    if (total_time1 < total_time2)
    {
        printf("\nJogador 1 é o vencedor!\n");
    }
    else if (total_time2 < total_time1)
    {
        printf("\nJogador 2 é o vencedor!\n");
    }
    else
    {
        printf("Houve um empate!\n");
    }

    return 0;
}