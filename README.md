# Embarcatech_Keypad_LedMatrix
Repositório criado para a tarefa 1 de Microcontroladores - GPIO

Grupo 2 - Subgrupo 6

Membros:

*Alisson Silva de Pinho*

*Eder Renato da Silva Cardoso Casar*

*Gabriel Cosmo Oliveira*

*Guilherme Fontana Cardoso*

*Helena Filemon Almeida Silva dos Santos*

*Melk Silva Braga*

*Sarah Modesto Sanches*

*Werliarlinson de Lima Sá Teles*

# Instruções de compilação

Para compilar o código, são necessárias as seguintes extensões: 

*Wokwi Simulator*

*Raspberry Pi Pico*

*Cmake*

Após instalá-las basta buildar o projeto pelo CMake. A partir daí, abra o arquivo 
diagram.json e clique no botão verde para iniciar a simulação.

Enquanto na simulação, o usuário pode clicar nos botões do teclado da simulação
a fim de acender as luzes conectadas à placa ou iniciar o buzzer.

As teclas *1*, *2*, *3*, *4* e *5* usam a matriz leds para gerar, cada tecla, uma animação diferente.

A tecla *A* desliga todos os leds.

As teclas *B*, *C*, *D* e *#* acendem todas as leds nas cores azul, vermelho, verde e branco,
respectivamente, com a intensidade das luzes variando de acordo com a tecla pressionada.

A tecla *0* inicia o buzzer, o qual toca uma música enquanto a matriz de leds faz uma animação.
