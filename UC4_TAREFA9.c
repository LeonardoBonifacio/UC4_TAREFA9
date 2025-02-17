#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "pico/bootrom.h"

// Constantes para display ssd1306 e comunicação i2c
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C // Endereço do display ssd1306
ssd1306_t ssd; // Inicializa a estrutura do display
static volatile int estado = 0; // Variável para controlar que borda do display está sendo mostrada ao apertar o botão do joystick
// Tamanho do quadrado e tela SSD1306
#define QUAD_SIZE 8  
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
// Posição inicial do quadrado (centro da tela)
int posX = SCREEN_WIDTH / 2 - QUAD_SIZE / 2;
int posY = SCREEN_HEIGHT / 2 - QUAD_SIZE / 2;


// Constantes e variáveis para botões, joystick e valores adc x e y
#define JOYSTICK_X_PIN 26  // GPIO para eixo X
#define JOYSTICK_Y_PIN 27  // GPIO para eixo Y
#define JOYSTICK_PUSHBUTTON 22 // GPIO para botão do Joystick
#define Botao_A 5 // GPIO para botão A
#define Botao_B 6// GPIO para botão B para entrar no modo de gravação
// Ajuste do centro real do joystick
#define CENTER_X 1939
#define CENTER_Y 2180
// Valores lidos pelos adc de x e y
uint16_t adc_value_x = 0;
uint16_t adc_value_y = 0;
volatile uint32_t ultimo_tempo_buttons = 0;// Para armazenar o tempo da última interrupção acionada pelo botôes da interrupção 



// Constantes dos pinos dos leds
#define LED_GREEN_PIN 11                    
#define LED_BLUE_PIN 12                   
#define LED_RED_PIN 13       




// Constantes e variáveis para controle pwm
const float DIVIDER_PWM = 16.0;          // Divisor fracional do clock para o PWM
const uint16_t PERIOD = 4096;            // Período do PWM (valor máximo do contador)
uint16_t led_blue_level, led_red_level = 0; // Inicialização dos níveis de PWM para os LEDs
uint slice_led_blue, slice_led_red;           // Variáveis para armazenar os slices de PWM correspondentes aos LEDs
bool ultimo_estado_pwm_leds = true; // Variável para controlar o último estado de controle dos leds azul e vermelho por pwm





void gpio_irq_handler(uint gpio, uint32_t events){
  uint32_t tempo_atual = time_us_32() / 1000;  // Obtém o tempo atual em milissegundos e o armazena
  if (tempo_atual - ultimo_tempo_buttons < 350) return;// Se o tempo passado for menor que o atraso  de debounce(350s) retorne imediatamente
  ultimo_tempo_buttons = tempo_atual;// O tempo atual corresponde ao último tempo que o botão foi pressionado, ja que ele passou pela verificação acima
  if (gpio == Botao_B){
    reset_usb_boot(0, 0);// Entra no mode de gravação após a interrupção ser ativada pelo botão B
  }
  else if(gpio == JOYSTICK_PUSHBUTTON){// Muda o esta do led verde e muda entre sem borda, borda simples, dupla e tripla no display 1306
    gpio_put(LED_GREEN_PIN, !gpio_get(LED_GREEN_PIN));
    // Atualiza o estado para o próximo ciclo das bordas
    estado = (estado + 1) % 3;
    
  }
  else if(gpio == Botao_A){ // Ativa o controle dos leds azul e vermelhos por pwm
    pwm_set_enabled(slice_led_blue,!ultimo_estado_pwm_leds);
    pwm_set_enabled(slice_led_red,!ultimo_estado_pwm_leds);
    ultimo_estado_pwm_leds = !ultimo_estado_pwm_leds;
  }
}


void init_gpios_and_adc_and_leds(){
  gpio_init(Botao_A);
  gpio_set_dir(Botao_A, GPIO_IN);
  gpio_pull_up(Botao_A);
  gpio_set_irq_enabled_with_callback(Botao_A,GPIO_IRQ_EDGE_FALL,true,&gpio_irq_handler);
  
  gpio_init(Botao_B);
  gpio_set_dir(Botao_B, GPIO_IN);
  gpio_pull_up(Botao_B);
  gpio_set_irq_enabled_with_callback(Botao_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
  
  adc_init();
  adc_gpio_init(JOYSTICK_X_PIN);
  adc_gpio_init(JOYSTICK_Y_PIN); 

  gpio_init(JOYSTICK_PUSHBUTTON);
  gpio_set_dir(JOYSTICK_PUSHBUTTON, GPIO_IN);
  gpio_pull_up(JOYSTICK_PUSHBUTTON); 
  gpio_set_irq_enabled_with_callback(JOYSTICK_PUSHBUTTON,GPIO_IRQ_EDGE_FALL,true,&gpio_irq_handler);

  gpio_init(LED_BLUE_PIN);
  gpio_init(LED_GREEN_PIN);
  gpio_init(LED_RED_PIN);
  
  gpio_set_dir(LED_BLUE_PIN,GPIO_OUT);
  gpio_set_dir(LED_RED_PIN,GPIO_OUT);
  gpio_set_dir(LED_GREEN_PIN,GPIO_OUT);

}

void init_i2c_and_display_ssd1306(){
    // I2C Initialisation. Using it at 400Khz.
  i2c_init(I2C_PORT, 400 * 1000);

  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
  gpio_pull_up(I2C_SDA); // Pull up the data line
  gpio_pull_up(I2C_SCL); // Pull up the clock line

  ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
  ssd1306_config(&ssd); // Configura o display
  ssd1306_fill(&ssd, false);// Limpa o display. O display inicia com todos os pixels apagados.
  ssd1306_send_data(&ssd); // Envia os dados para o display

}

void joystick_read_axis(uint16_t *adc_value_x, uint16_t *adc_value_y) {
  adc_select_input(1); 
  sleep_us(2);
  *adc_value_x = adc_read();

  adc_select_input(0); 
  sleep_us(2);
  *adc_value_y = adc_read();
}


// Função para configurar o PWM de um LED (genérica para azul e vermelho)
void setup_pwm_led(uint led, uint *slice, uint16_t level)
{
  gpio_set_function(led, GPIO_FUNC_PWM); // Configura o pino do LED como saída PWM
  *slice = pwm_gpio_to_slice_num(led);   // Obtém o slice do PWM associado ao pino do LED
  pwm_set_clkdiv(*slice, DIVIDER_PWM);   // Define o divisor de clock do PWM
  pwm_set_wrap(*slice, PERIOD);          // Configura o valor máximo do contador (período do PWM)
  pwm_set_gpio_level(led, level);        // Define o nível inicial do PWM para o LED
  pwm_set_enabled(*slice, true);         // Habilita o PWM no slice correspondente ao LED
}

void apaga_ou_aumenta_leds_conforme_joystick(uint16_t adc_value_x, uint16_t adc_value_y, uint16_t intensidade_x, uint16_t intensidade_y){
  // Se o joystick estiver na posição neutra, desliga os LEDs
  if ((adc_value_x >= CENTER_X && adc_value_x <= CENTER_Y) && (adc_value_y >= CENTER_X && adc_value_y <= CENTER_Y)) {
      pwm_set_gpio_level(LED_RED_PIN, 0);  // Desliga o LED vermelho
      pwm_set_gpio_level(LED_BLUE_PIN, 0); // Desliga o LED azul
  } else {

      // Mapeia a intensidade de 0 a PERIOD para o LED vermelho (Eixo X)
      if (adc_value_x < CENTER_X) {
          intensidade_x = ((CENTER_X - adc_value_x) * PERIOD) / CENTER_X;
      } else if (adc_value_x > CENTER_Y) {
          intensidade_x = ((adc_value_x - CENTER_Y) * PERIOD) / (4095 - CENTER_Y);
      }

      // Mapeia a intensidade de 0 a PERIOD para o LED azul (Eixo Y)
      if (adc_value_y < CENTER_X) {
          intensidade_y = ((CENTER_X - adc_value_y) * PERIOD) / CENTER_X;
      } else if (adc_value_y > CENTER_Y) {
          intensidade_y = ((adc_value_y - CENTER_Y) * PERIOD) / (4095 - CENTER_Y);
      }

      // Limita ao máximo do PWM
      if (intensidade_x > PERIOD) intensidade_x = PERIOD;
      if (intensidade_y > PERIOD) intensidade_y = PERIOD;

      // Aplica os valores aos LEDs
      pwm_set_gpio_level(LED_RED_PIN, intensidade_x);
      pwm_set_gpio_level(LED_BLUE_PIN, intensidade_y);
    }
}

// Mapeia valores do ADC para a tela SSD1306
int posicao_adc_pra_displayssd1306(int adc_value, int center_value, int screen_max) {
  int range_min = center_value;     
  int range_max = 4095 - center_value;
  
  int offset = adc_value - center_value; 

  int mapped_value;
  if (offset < 0) {
      mapped_value = ((offset * (screen_max / 2)) / range_min) + (screen_max / 2);
  } else {
      mapped_value = ((offset * (screen_max / 2)) / range_max) + (screen_max / 2);
  }

  if (mapped_value < 0) mapped_value = 0;
  if (mapped_value > screen_max) mapped_value = screen_max;

  return mapped_value;
}


// Atualiza a posição do quadrado no display
void update_square_position(uint16_t *adc_value_x, uint16_t *adc_value_y) {
  uint16_t adc_x = *adc_value_x;
  uint16_t adc_y = *adc_value_y;

  int limite_borda = 5;

  posX = posicao_adc_pra_displayssd1306(adc_x, CENTER_X, SCREEN_WIDTH - QUAD_SIZE - limite_borda);
  posY = SCREEN_HEIGHT - QUAD_SIZE - posicao_adc_pra_displayssd1306(adc_y, CENTER_Y, SCREEN_HEIGHT - QUAD_SIZE - limite_borda);

  if (posX < limite_borda) posX = limite_borda;
  if (posY < limite_borda) posY = limite_borda;
}

// Baseado em qual número a váriavel *estado*(que muda conforme interrupção acionada pelo botão do joystick) está, desenha uma borda mais fina ou mais grossa
void desenha_borda(int estado){
  switch (estado){
    case 0:  // Borda simples
    ssd1306_rect(&ssd,3,3,122,60,true,false);
    break;
    case 1: // Borda dupla
    ssd1306_rect(&ssd,3,3,122,60,true,false);
    ssd1306_rect(&ssd,4,4,120,58,true,false);
    break;
    case 2: // Borda tripla
      ssd1306_rect(&ssd,3,3,122,60,true,false);
      ssd1306_rect(&ssd,4,4,120,58,true,false);
      ssd1306_rect(&ssd,5,5,118,56,true,false);
    break;
    
}
}

// Atualiza o display
void desenha_display(ssd1306_t *ssd) {
  ssd1306_fill(ssd, false);
  desenha_borda(estado);
  ssd1306_rect(ssd, posY, posX, QUAD_SIZE, QUAD_SIZE, true, true);
  ssd1306_send_data(ssd);
}



int main()
{
    init_gpios_and_adc_and_leds();
    init_i2c_and_display_ssd1306();
    setup_pwm_led(LED_BLUE_PIN,&slice_led_blue,0);
    setup_pwm_led(LED_RED_PIN,&slice_led_red,0);
    stdio_init_all();

    while (true) {
      uint16_t intensidade_x = 0;
      uint16_t intensidade_y = 0;
      joystick_read_axis(&adc_value_x, &adc_value_y); 
      update_square_position(&adc_value_x, &adc_value_y);
      desenha_display(&ssd);
      apaga_ou_aumenta_leds_conforme_joystick(adc_value_x,adc_value_y,intensidade_x,intensidade_y);
      sleep_ms(50);
    }
}
