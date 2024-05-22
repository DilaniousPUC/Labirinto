#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define SCREEN_WIDTH 40
#define SCREEN_HEIGHT 20
#define CELL_SIZE 2
#define NUM_ROWS (SCREEN_HEIGHT / CELL_SIZE)
#define NUM_COLS (SCREEN_WIDTH / CELL_SIZE)

#define EMPTY_CELL 0
#define WALL_CELL '#'
#define PLAYER_CELL '@'
#define EXIT_CELL 'X'

int maze[NUM_ROWS][NUM_COLS];
int player_x, player_y;
int exit_x, exit_y;

void generate_maze() {
    // Preencher o labirinto com células vazias
    for (int i = 0; i < NUM_ROWS; i++) {
        for (int j = 0; j < NUM_COLS; j++) {
            maze[i][j] = EMPTY_CELL;
        }
    }

    // Colocar as paredes em células aleatórias
    for (int i = 0; i < NUM_ROWS * NUM_COLS / 2; i++) {
        int x = rand() % NUM_COLS;
        int y = rand() % NUM_ROWS;
        maze[y][x] = WALL_CELL;
    }

    // Escolher uma posição aleatória para a saída
    do {
        exit_x = rand() % NUM_COLS;
        exit_y = rand() % NUM_ROWS;
    } while (maze[exit_y][exit_x] == WALL_CELL);
    maze[exit_y][exit_x] = EXIT_CELL;

    // Colocar o jogador em uma posição inicial que não seja bloqueada por paredes
    do {
        player_x = rand() % NUM_COLS;
        player_y = rand() % NUM_ROWS;
    } while (maze[player_y][player_x] == WALL_CELL || (player_x == exit_x && player_y == exit_y));
    maze[player_y][player_x] = PLAYER_CELL;
}

void print_maze() {
    // Limpar a tela
    system("cls");

    // Imprimir o labirinto
    for (int i = 0; i < NUM_ROWS; i++) {
        for (int j = 0; j < NUM_COLS; j++) {
            if (maze[i][j] == WALL_CELL)
                printf("%c", WALL_CELL);
            else if (maze[i][j] == EMPTY_CELL)
                printf("%d", EMPTY_CELL);
            else if (maze[i][j] == PLAYER_CELL)
                printf("%c", PLAYER_CELL);
            else if (maze[i][j] == EXIT_CELL)
                printf("%c", EXIT_CELL);
        }
        printf("\n");
    }
}

int main() {
    srand(time(NULL)); // Inicializar a semente para números aleatórios

    generate_maze(); // Gerar o labirinto

    // Loop principal do jogo
    while (true) {
        // Imprimir o labirinto
        print_maze();

        // Capturar a entrada do jogador
        char input;
        printf("Use as teclas do teclado W (cima)/ A (esquerda)/ S (baixo)/ D (direita) para mover o jogador: ");
        scanf(" %c", &input);

        // Calcular a próxima posição do jogador
        int next_x = player_x;
        int next_y = player_y;
        switch (input) {
            case 'W':
                next_y--; // Mover para cima
                break;
            case 'w':
                next_y--; // Mover para cima
                break;
            case 'S':
                next_y++; // Mover para baixo
                break;
            case 's':
                next_y++; // Mover para baixo
                break;
            case 'A':
                next_x--; // Mover para a esquerda
                break;
            case 'a':
                next_x--; // Mover para a esquerda
                break;
            case 'D':
                next_x++; // Mover para a direita
                break;
            case 'd':
                next_x++; // Mover para a direita
                break;
            default:
                printf("Entrada invalida!\n");
                break;
        }

        // Verificar se o próximo movimento é válido
        if (next_x >= 0 && next_x < NUM_COLS && next_y >= 0 && next_y < NUM_ROWS && maze[next_y][next_x] != WALL_CELL) {
            // Atualizar a posição do jogador
            maze[player_y][player_x] = EMPTY_CELL;
            player_x = next_x;
            player_y = next_y;
            maze[player_y][player_x] = PLAYER_CELL;
        }

        // Verificar se o jogador alcançou a saída
        if (player_x == exit_x && player_y == exit_y) {
            printf("Voce venceu!\n");
            break; // O jogo acabou
        }
    }

    return 0;
}
