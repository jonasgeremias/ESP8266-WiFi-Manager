
#ifndef __LED_STATUS__

#include "driver/gpio.h"

uint16_t pisca_led_status_timeout = 0;
uint16_t led_status_anterior = 0xffff;
uint16_t tempo_estrobo_status = 0;
uint16_t tempo_estrobo_erro = 0;
uint16_t troca_modo_led_status_timeout = 0;
void controle_led_status(uint8_t modo);


enum {
   LED_DESLIGADO = 0,
   LED_LIGADO,
   LED_PISCA_LENTO,
   LED_PISCA_RAPIDO,
   LED_ESTROBO1,
   LED_ESTROBO2
};

void led_status_timer() {
   if (pisca_led_status_timeout) pisca_led_status_timeout--;
   if (tempo_estrobo_status) tempo_estrobo_status--;
   if (troca_modo_led_status_timeout) troca_modo_led_status_timeout--;
	if (tempo_estrobo_erro) tempo_estrobo_erro--;
}

void led_status_configurar_pinos() {
   //gpio_pad_select_gpio(LED_STATUS);
   gpio_set_direction(LED_STATUS, GPIO_MODE_OUTPUT);
   gpio_set_level(LED_STATUS, 0);
}

// Saidas LEDs ----------------------------------------------------------------
void controle_led_status(uint8_t modo) {
   static bool s_ledstatus = 0;
	static uint8_t led_status_anterior = 0;

	// Se mudar de status tem que atualizar na hora
   if (modo != led_status_anterior) {
      pisca_led_status_timeout = 0;
      troca_modo_led_status_timeout = 0;
   }
   led_status_anterior = modo;


   if (modo == LED_DESLIGADO) { // Desliga
      s_ledstatus = 0;
   }
   else if (modo == LED_LIGADO) {  // Liga direto verde
      s_ledstatus = 1;
   }
   else if (modo == LED_PISCA_LENTO) { // Pisca lento verde
      if (!pisca_led_status_timeout) {
         pisca_led_status_timeout = (350 / LED_STATUS_BASE_TEMPO_MS);
			s_ledstatus = !s_ledstatus;
      }
   }
   else if (modo == LED_PISCA_RAPIDO) { // Pisca rápido verde
      if (!pisca_led_status_timeout) {
         pisca_led_status_timeout = (50 / LED_STATUS_BASE_TEMPO_MS);
			s_ledstatus = !s_ledstatus;
      }
   }
   else if (modo == LED_ESTROBO1) { // Estrobo por 0,25s a cada 1s
      if (!troca_modo_led_status_timeout || (troca_modo_led_status_timeout >= (1000 / LED_STATUS_BASE_TEMPO_MS))) troca_modo_led_status_timeout = (1000 / LED_STATUS_BASE_TEMPO_MS);
      if (troca_modo_led_status_timeout <= (250 / LED_STATUS_BASE_TEMPO_MS)) {
         if (!pisca_led_status_timeout) {
            pisca_led_status_timeout = (50 / LED_STATUS_BASE_TEMPO_MS);
            s_ledstatus = !s_ledstatus;
         }
      }
      else {
         s_ledstatus = 0;
      }
   }
   else if (modo == LED_ESTROBO2) { // Estrobo por 0,25s a cada 0,5s
      if (!troca_modo_led_status_timeout || (troca_modo_led_status_timeout >= (500 / LED_STATUS_BASE_TEMPO_MS))) troca_modo_led_status_timeout = (500 / LED_STATUS_BASE_TEMPO_MS);
      if (troca_modo_led_status_timeout <= (100 / LED_STATUS_BASE_TEMPO_MS)) {
         if (!pisca_led_status_timeout) {
            pisca_led_status_timeout = (100 / LED_STATUS_BASE_TEMPO_MS);
            s_ledstatus = !s_ledstatus;
         }
      }
      else {
         s_ledstatus = 0;
      }
   }

	gpio_set_level(LED_STATUS, s_ledstatus);
}
#define __LED_STATUS__
#endif
