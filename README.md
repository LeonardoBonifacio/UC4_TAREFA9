# Projeto para Placa BitDogLab com Raspberry Pi Pico W

Este projeto foi desenvolvido para a placa **BitDogLab** utilizando o **Raspberry Pi Pico W** com a **Pico SDK**. Ele implementa um sistema que integra um joystick analógico, botões físicos, LEDs RGB e um display OLED SSD1306, utilizando comunicação I2C e controle PWM para os LEDs.

## Link para video de demonstração: https://youtu.be/YywET4KeBD4?si=ezjlV6TSY6d0_Ozu

## Funcionalidades

- **Leitura do joystick analógico** (eixos X e Y via ADC)
- **Controle de LEDs RGB** com PWM
- **Interrupções nos botões físicos** para ativar/desativar funcionalidades
- **Desenho dinâmico no display OLED SSD1306** baseado no movimento do joystick
- **Modo de gravação (USB Boot Mode)** ativado por um botão físico

## Requisitos de Hardware

- Placa **BitDogLab** com **Raspberry Pi Pico W**
- Display **SSD1306 OLED** (I2C)
- Joystick analógico (eixos X e Y + botão)
- Botões físicos adicionais (A e B)
- LEDs RGB conectados aos pinos PWM

## Conexões de Hardware

| Componente   | GPIO (Pico) |
|-------------|------------|
| Joystick X  | GP26 (ADC0) |
| Joystick Y  | GP27 (ADC1) |
| Botão do Joystick | GP22 |
| Botão A | GP5 |
| Botão B | GP6 |
| LED Verde | GP11 |
| LED Azul (PWM) | GP12 |
| LED Vermelho (PWM) | GP13 |
| I2C SDA (SSD1306) | GP14 |
| I2C SCL (SSD1306) | GP15 |

## Instalação e Compilação

### 1. Conectar a Raspberry Pi Pico W
Conecte a Raspberry Pi Pico W ao seu computador via cabo USB.

### 2. Compilação
Este projeto pode ser compilado usando o ambiente de desenvolvimento **CMake** e a extensão Raspberry pi pico no vscode, clonando ou baixando este repositorio como zip e importando ele através da extensão da raspberry pi pico . 

### 3. Upload e Execução
Após compilar, envie o código para a Raspberry Pi Pico W(que neste caso esta presente na placa BitDogLab), colocando a placa em modo de gravação.

## Uso

### Controle do Joystick
- Move um quadrado na tela OLED de acordo com os eixos X e Y do joystick.
- Ajusta a intensidade dos LEDs com base nos valores do joystick.

### Botões Físicos
- **Botão do Joystick (GP22)**: Alterna entre diferentes tipos de bordas no display OLED.
- **Botão A (GP5)**: Liga/desliga o controle de LEDs via PWM.
- **Botão B (GP6)**: Entra no modo de gravação (reset USB Boot).

## Bibliotecas Utilizadas

- **Pico SDK** (stdlib, adc, i2c, pwm, bootrom)
- **SSD1306** (driver para display OLED)
- **Fontes customizadas** para exibição no display

## Licença

Este projeto está sob a licença MIT. Sinta-se à vontade para modificar e distribuir conforme necessário.

---

Caso tenha dúvidas ou precise de mais informações, entre em contato!

