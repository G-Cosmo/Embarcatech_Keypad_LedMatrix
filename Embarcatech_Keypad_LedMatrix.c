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

const uint buzzer_pin = 14; // GPIO do buzzer
const uint row_pins[4] = {28, 27, 26, 22}; 
const uint col_pins[4] = {21, 20, 19, 18};
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
    int melody[] = {294,330,349,440,392,440,262,294,330,349,330,392,440,392,349,349,349,349,440,440,392,349,
    440,440,440,392,440,392,349,349,349,349,440,440,392,349,440,440,440,554,554,554,349,349,349,440,440,392,349,
    466,466,466,392,523,440,659,698,587,698,880,659,554,880,1109,1175};

    int noteDurations[] = {600,300,600,300,300,300,600,600,300,300,300,300,300,300,300,150,150,150,150,150,150,300,
    150,150,150,150,150,150,300,150,150,150,150,150,150,300,150,150,300,150,150,300,150,150,150,150,150,150,300,
    150,150,150,300,300,300,300, 75, 75, 75, 75, 75, 75, 75, 75,1200};
    
    int pausa[ ] = {300, 0 ,600,150,150, 0, 600,300, 0, 300,300,300,300,300,600,150,150,150,150,150,150,300,150, 
    150,150,150,150,150,300,150,150,150,150,150,150,300,150,150,300,150,150,300,150,150,150,150,150,150,300,150,
    150,150,300,300,300,300, 75, 75, 75, 75, 75, 75, 75,  75, 1200 }; 
    
    int length = sizeof(melody) /sizeof(melody)[0];
    for (int i = 0; i < length; i++) {
        int noteDuration = noteDurations[i];
        int frequency = melody[i];
        int halfDuration = noteDuration / 2;
        int amplitude = (frequency % 5) + 1; // Altura da oscilação baseada na frequência

        // Mapeia a frequência para uma cor
        int red = 255 - (frequency % 256);
        int blue = frequency % 256;
        int green = 0; // se quiser dar uma variada na cor, basta alterar o valor de green

        pico_buzzer_play(gpio, frequency);

        for (int t = 0; t < noteDuration; t += 50) { 
            for (int y = 0; y < 5; y++) {
                for (int x = 0; x < 5; x++) {
                    int offset = (t < halfDuration) ? (t * amplitude / halfDuration) : ((noteDuration - t) * amplitude / halfDuration);
                    if (x == 2) { // Coluna central
                        if (y == 2 - offset || y == 2 + offset) {
                            npSetLED(y * 5 + x, red, green, blue); // Acende o LED com a cor calculada
                        } else {
                            npSetLED(y * 5 + x, 0, 0, 0); // Apaga o LED
                        }
                    } else if (x < 2) { // Colunas à esquerda
                        int delay = (2 - x) * 50; // Atraso para criar um efeito de cascata
                        if (t >= delay) {
                            int localOffset = ((t - delay) < halfDuration) ? ((t - delay) * amplitude / halfDuration) : ((noteDuration - (t - delay)) * amplitude / halfDuration);
                            if (y == 2 - localOffset || y == 2 + localOffset) {
                                npSetLED(y * 5 + x, red, green, blue); // Acende o LED com a cor calculada
                            } else {
                                npSetLED(y * 5 + x, 0, 0, 0); 
                            }
                        } else {
                            npSetLED(y * 5 + x, 0, 0, 0); // Apaga o LED
                        }
                    } else { // Colunas à direita
                        int delay = (x - 2) * 50; // Atraso para criar um efeito de cascata
                        if (t >= delay) {
                            int localOffset = ((t - delay) < halfDuration) ? ((t - delay) * amplitude / halfDuration) : ((noteDuration - (t - delay)) * amplitude / halfDuration);
                            if (y == 2 - localOffset || y == 2 + localOffset) {
                                npSetLED(y * 5 + x, red, green, blue); // Acende o LED com a cor calculada
                            } else {
                                npSetLED(y * 5 + x, 0, 0, 0); // Apaga o LED
                            }
                        } else {
                            npSetLED(y * 5 + x, 0, 0, 0); // Apaga o LED
                        }
                    }
                }
            }
            npWrite(); // Atualiza os LEDs
            sleep_ms(50); // Espera 50ms
        }
        
        pico_buzzer_stop(gpio);

        // Apaga todos os LEDs após a duração da nota
        for (int y = 0; y < 5; y++) {
            for (int x = 0; x < 5; x++) {
                npSetLED(y * 5 + x, 0, 0, 0);
            }
        }
        npWrite();
        sleep_ms(pausa[i]);
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
            break;
        case '3':
            npClear();
            for (int i = 0; i < 25; i++)
            {
                npSetLED(i, 0, 0, 255);
            }
            npWrite();
            break;
        case 'A':
            npClear();
            npWrite();
            break;
        case '4':
            npClear(); 
            for (int i = 0; i < 25; i++)
            {
                npSetLED(i, 255, 255, 0);
            }
            npWrite();
            break;
        case '5':
            npClear();
            for (int i = 0; i < 25; i++)
            {
                npSetLED(i, 255, 0, 255);
            }
            npWrite();
            break;
        case '6':
            npClear();
            for (int i = 0; i < 25; i++)
            {
                npSetLED(i, 0, 255, 255);
            }
            npWrite();
            break;
        case 'B':
            npClear(); 
            for (int i = 0; i < 25; i++)
            {
                npSetLED(i, 0, 0, 255);
            }
            npWrite();
            break;
        case '7':
            npClear();
            for (int i = 0; i < 25; i++)
            {
                npSetLED(i, 255, 255, 255);
            }
            npWrite();
            break;
        case '8':
            npClear();
            for (int i = 0; i < 25; i++)
            {
                npSetLED(i, 100, 255, 100);
            }
            npWrite();
            break;
        case '9':
            npClear();
            for (int i = 0; i < 25; i++)
            {
                npSetLED(i, 255, 100, 100);
            }
            npWrite();
            break;
        case 'C':
            npClear(); 
            for (int i = 0; i < 25; i++)
            {
                npSetLED(i, 204, 0, 0);
            }
            npWrite();
            break;
        case 'D':
            npClear(); 
            for (int i = 0; i < 25; i++)
            {
                npSetLED(i, 0, 127, 0);
            }
            npWrite();
            break;
        case '0': // Desliga todos os LEDS de uma vez
            play_musica(buzzer_pin);
            npClear();
            break;
        case '#':
            npClear(); 
            for (int i = 0; i < 25; i++)
            {
                npSetLED(i, 051, 051, 051);
            }
            npWrite();
            break;
        case '*': //Reset
            sleep_ms(1000); // Espera 1 segundo antes de reiniciar no modo bootset
            reset_usb_boot(0, 0); // Reinicia o dispositivo no modo bootset
            break;
        default:
            printf("Tecla '%c' não mapeada.\n", key);
            break;
    }
}


int main()
{
    char key;
    //stdio_init_all();
    pico_init_keypad(row_pins, col_pins);
    pico_buzzer_init(buzzer_pin);                         // Inicializar o buzzer
    npInit(LED_PIN);                                      // Inicializar os LEDs
    npClear();                                            // Apagar todos os LEDs
    npWrite();                                        // Atualizar o estado inicial dos LEDs

    while (true) {
        key = pico_scan_keypad(); 
        if (key != '\0') {
            pico_keypad_control_led(key); // Executa a ação correspondente no modo padrão (LEDs)
        }
        sleep_ms(100);
    }
}
