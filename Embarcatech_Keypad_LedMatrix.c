#include <stdio.h>
#include "pico/stdlib.h"


#define ROWS 4
#define COLS 4

const uint col_pins[4] = {4, 3, 2, 1}; 
const uint row_pins[4] = {8, 7, 6, 5};
const uint rgb_1[3] = {28, 27, 26};

// Matriz de mapeamento de teclas
const char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

void pico_init_keypad() {
    // Configura os pinos das linhas como saída e os pinos das colunas como entrada
    for (int i = 0; i < ROWS; i++) {
        gpio_init(row_pins[i]);
        gpio_set_dir(row_pins[i], GPIO_OUT);
        gpio_put(row_pins[i], 1); // Inicializa as linhas com nível alto
    }

    for (int i = 0; i < COLS; i++) {
        gpio_init(col_pins[i]);
        gpio_set_dir(col_pins[i], GPIO_IN);
        gpio_pull_up(col_pins[i]); // Ativa o pull-up nas colunas
    }
}

char pico_scan_keypad() {
    for (int r = 0; r < ROWS; r++) {
        gpio_put(row_pins[r], 0); // Ativa a linha (coloca em nível baixo)
        for (int c = 0; c < COLS; c++) {
            if (!gpio_get(col_pins[c])) { // Verifica se o botão foi pressionado
                while (!gpio_get(col_pins[c])) {
                    // Espera até que o botão seja liberado
                }
                return keys[r][c]; // Retorna a tecla pressionada
            }
        }
        gpio_put(row_pins[r], 1); // Desativa a linha (coloca em nível alto)
    }
    return '\0'; // Retorna null se nenhum botão for pressionado
}

int main()
{
    char key;
    stdio_init_all();
    pico_init_keypad(row_pins, col_pins);
    pico_rgb_init(rgb_1);


    while (true) {
        key = pico_scan_keypad(row_pins, col_pins, &keys); 



        sleep_ms(100);
    }
}
