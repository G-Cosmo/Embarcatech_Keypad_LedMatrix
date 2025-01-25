#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"           // Biblioteca para manipulação de periféricos PIO
#include "ws2818b.pio.h"             // Programa para controle de LEDs WS2812B
#include "pico/bootrom.h" 


#define ROWS 4
#define COLS 4

#define LED_COUNT 25                // Número de LEDs na matriz
#define LED_PIN 7                   // Pino GPIO conectado aos LEDs

#define NOTE_DO1  261
#define NOTE_RE  294
#define NOTE_MI  329
#define NOTE_FA  349
#define NOTE_SOL  392
#define NOTE_LA  440
#define NOTE_SI  493
#define NOTE_DO2  523

const uint buzzer_pin = 21; // GPIO do buzzer
const uint col_pins[4] = {20,4,9,8}; 
const uint row_pins[4] = {16,17,18,19};
//const uint rgb_1[3] = {28, 27, 26};

// Estrutura para representar um pixel com componentes RGB
struct pixel_t { 
    uint8_t G, R, B;                // Componentes de cor: Verde, Vermelho e Azul
};


typedef struct pixel_t pixel_t;     // Alias para a estrutura pixel_t
typedef pixel_t npLED_t;            // Alias para facilitar o uso no contexto de LEDs

npLED_t leds[LED_COUNT];            // Array para armazenar o estado de cada LED
PIO np_pio;                         // Variável para referenciar a instância PIO usada
uint sm;                            // Variável para armazenar o número do state machine usado

int getIndex(int x, int y) {
    // Se a linha for par (0, 2, 4), percorremos da esquerda para a direita.
    // Se a linha for ímpar (1, 3), percorremos da direita para a esquerda.
    if (y % 2 == 0) {
        return 24-(y * 5 + x); // Linha par (esquerda para direita).
    } else {
        return 24-(y * 5 + (4 - x)); // Linha ímpar (direita para esquerda).
    }
}
//função para inicializar o buzzer
void pico_buzzer_init(uint gpio) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_enabled(slice_num, true);
}

//função para tocar uma nota no buzzer
void pico_buzzer_play(uint gpio, uint frequency) {
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    uint32_t clock = 125000000; 
    uint32_t divider = clock / (frequency * 4096); 
    uint32_t wrap = (clock / divider) / frequency - 1;
    uint32_t level = wrap / 2; 
    pwm_set_clkdiv(slice_num, divider);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, level);
    pwm_set_enabled(slice_num, true);
}

//função para parar o buzzer
void pico_buzzer_stop(uint gpio) {
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 0);
    pwm_set_enabled(slice_num, false);
}

//função para tocar uma melodia
void play_musica(uint gpio) {
    int melody[] = {
        NOTE_FA, NOTE_RE, NOTE_FA, NOTE_FA, NOTE_RE, NOTE_FA, NOTE_FA, NOTE_RE, NOTE_RE, NOTE_FA, NOTE_FA, NOTE_RE,
        NOTE_FA, NOTE_FA, NOTE_RE, NOTE_RE, NOTE_FA, NOTE_FA, NOTE_RE
    };
    int noteDurations[] = {
        2, 2, 4, 4, 2, 4, 4, 4, 4, 4, 4,
        2, 4, 4, 4, 4, 4, 4, 2
    };
    
    int length = sizeof(melody) /sizeof(melody)[0];
    for (int i = 0; i < length; i++) {
        int noteDuration = 1000 / noteDurations[i];
        pico_buzzer_play(gpio, melody[i]);
        sleep_ms(noteDuration);
        pico_buzzer_stop(gpio);
        sleep_ms(noteDuration * 0.3);
    }
}

// Função para inicializar o PIO para controle dos LEDs
void npInit(uint pin) 
{
    uint offset = pio_add_program(pio0, &ws2818b_program); // Carregar o programa PIO
    np_pio = pio0;                                         // Usar o primeiro bloco PIO

    sm = pio_claim_unused_sm(np_pio, false);              // Tentar usar uma state machine do pio0
    if (sm < 0)                                           // Se não houver disponível no pio0
    {
        np_pio = pio1;                                    // Mudar para o pio1
        sm = pio_claim_unused_sm(np_pio, true);           // Usar uma state machine do pio1
    }

    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f); // Inicializar state machine para LEDs

    for (uint i = 0; i < LED_COUNT; ++i)                  // Inicializar todos os LEDs como apagados
    {
        leds[i].R = 0;
        leds[i].G = 0;
        leds[i].B = 0;
    }
}

// Função para definir a cor de um LED específico
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) 
{
    leds[index].R = r;                                    // Definir componente vermelho
    leds[index].G = g;                                    // Definir componente verde
    leds[index].B = b;                                    // Definir componente azul
}

// Função para limpar (apagar) todos os LEDs
void npClear() 
{
    for (uint i = 0; i < LED_COUNT; ++i)                  // Iterar sobre todos os LEDs
        npSetLED(i, 0, 0, 0);                             // Definir cor como preta (apagado)
}

// Função para atualizar os LEDs no hardware
void npWrite() 
{
    for (uint i = 0; i < LED_COUNT; ++i)                  // Iterar sobre todos os LEDs
    {
        pio_sm_put_blocking(np_pio, sm, leds[i].G);       // Enviar componente verde
        pio_sm_put_blocking(np_pio, sm, leds[i].R);       // Enviar componente vermelho
        pio_sm_put_blocking(np_pio, sm, leds[i].B);       // Enviar componente azul
    }
}

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
void animacao1(){
    int matriz[5][5][3] = {
                {{0, 101, 4}, {0, 101, 4}, {0, 101, 4}, {0, 101, 4}, {0, 101, 4}},
                {{0, 101, 4}, {0, 0, 0}, {0, 101, 4}, {0, 0, 0}, {0, 101, 4}},
                {{0, 101, 4}, {0, 101, 4}, {0, 0, 0}, {0, 101, 4}, {0, 101, 4}},
                {{0, 101, 4}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 101, 4}},
                {{0, 101, 4}, {0, 0, 0}, {0, 101, 4}, {0, 0, 0}, {0, 101, 4}}
                };
    // Desenhando Sprite contido na matriz.c
    for(int linha = 0; linha < 5; linha++){
        for(int coluna = 0; coluna < 5; coluna++){
            int posicao = getIndex(linha, coluna);
            npSetLED(posicao, matriz[coluna][linha][0], matriz[coluna][linha][1], matriz[coluna][linha][2]);
        }
    }
    npWrite();
    sleep_ms(500);
    npClear();
    int matriz2[5][5][3] = {
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {255, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
        };
    for(int linha = 0; linha < 5; linha++){
        for(int coluna = 0; coluna < 5; coluna++){
            int posicao = getIndex(linha, coluna);
            npSetLED(posicao, matriz2[coluna][linha][0], matriz2[coluna][linha][1], matriz2[coluna][linha][2]);
        }
    }
    npWrite();
    sleep_ms(250);
    npClear();
    int matriz3[5][5][3] = {
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {255, 0, 0}, {255, 255, 0}, {255, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
        };
    for(int linha = 0; linha < 5; linha++){
        for(int coluna = 0; coluna < 5; coluna++){
            int posicao = getIndex(linha, coluna);
            npSetLED(posicao, matriz3[coluna][linha][0], matriz3[coluna][linha][1], matriz3[coluna][linha][2]);
        }
    }
    npWrite();
    sleep_ms(250);
    npClear();
    int matriz4[5][5][3] = {
        {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
        {{255, 0, 0}, {255, 255, 0}, {255, 255, 0}, {255, 255, 0}, {255, 0, 0}},
        {{255, 0, 0}, {255, 255, 0}, {255, 0, 0}, {255, 255, 0}, {255, 0, 0}},
        {{255, 0, 0}, {255, 255, 0}, {255, 255, 0}, {255, 255, 0}, {255, 0, 0}},
        {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}}
        };
    for(int linha = 0; linha < 5; linha++){
        for(int coluna = 0; coluna < 5; coluna++){
            int posicao = getIndex(linha, coluna);
            npSetLED(posicao, matriz4[coluna][linha][0], matriz4[coluna][linha][1], matriz4[coluna][linha][2]);
        }
    }
    npWrite();
    sleep_ms(250);
    npClear();
    int matriz5[5][5][3] = {
        {{255, 255, 0}, {255, 255, 0}, {255, 255, 0}, {255, 255, 0}, {255, 255, 0}},
        {{255, 255, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 255, 0}},
        {{255, 255, 0}, {255, 0, 0}, {0,0,0}, {255, 0, 0}, {255, 255, 0}},
        {{255, 255, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 255, 0}},
        {{255, 255, 0}, {255, 255, 0}, {255, 255, 0}, {255, 255, 0}, {255, 255, 0}}
        };
    for(int linha = 0; linha < 5; linha++){
        for(int coluna = 0; coluna < 5; coluna++){
            int posicao = getIndex(linha, coluna);
            npSetLED(posicao, matriz5[coluna][linha][0], matriz5[coluna][linha][1], matriz5[coluna][linha][2]);
        }
    }
    npWrite();
    sleep_ms(250);
    npClear();
    int matriz6[5][5][3] = {
        {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
        {{255, 0, 0}, {0,0,0}, {0,0,0}, {0,0,0}, {255, 0, 0}},
        {{255, 0, 0}, {0,0,0}, {0,0,0}, {0,0,0}, {255, 0, 0}},
        {{255, 0, 0}, {0,0,0}, {0,0,0}, {0,0,0}, {255, 0, 0}},
        {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}}
        };
    for(int linha = 0; linha < 5; linha++){
        for(int coluna = 0; coluna < 5; coluna++){
            int posicao = getIndex(linha, coluna);
            npSetLED(posicao, matriz6[coluna][linha][0], matriz6[coluna][linha][1], matriz6[coluna][linha][2]);
        }
    }
    npWrite();
    sleep_ms(250);
    npClear();
    int matriz7[5][5][3] = {
        {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}},
        {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}},
        {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}},
        {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}},
        {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}}
        };
    for(int linha = 0; linha < 5; linha++){
        for(int coluna = 0; coluna < 5; coluna++){
            int posicao = getIndex(linha, coluna);
            npSetLED(posicao, matriz7[coluna][linha][0], matriz7[coluna][linha][1], matriz7[coluna][linha][2]);
        }
    }
    npWrite();
    sleep_ms(250);
    npClear();
}
void animacao2(){
    for (int k = 0; k < 3; k++) {
        for (int i = 0; i < 25; i++) {
            npClear();
            if (k == 0) {
                npSetLED(i, 255, 0, 0); // Vermelho
            } else if (k == 1) {
                npSetLED(i, 0, 255, 0); // Verde
            } else {
                npSetLED(i, 0, 0, 255); // Azul
            }
            npWrite();
            sleep_ms(200);
        }
    }
}
void pico_keypad_control_led(char key) {
    switch (key) {
        case '1':
            animacao1();
            break;
        case '2':
            animacao2();
        default:
            printf("Tecla '%c' não mapeada.\n", key);
            break;
    }
}


int main()
{
    char key;
    stdio_init_all();
    pico_init_keypad(row_pins, col_pins);
    pico_buzzer_init(buzzer_pin);                         // Inicializar o buzzer
    npInit(LED_PIN);                                      // Inicializar os LEDs
    npClear();                                            // Apagar todos os LEDs
    npWrite();                                            // Atualizar o estado inicial dos LEDs

    while (true) {
        key = pico_scan_keypad(row_pins, col_pins, &keys); 
        if (key != '\0') {
            pico_keypad_control_led(key); // Executa a ação correspondente no modo padrão (LEDs)
        }

        sleep_ms(100);
    }
}
