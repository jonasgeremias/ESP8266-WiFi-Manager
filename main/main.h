#define DEBUG
#ifdef DEBUG
   #define DEBUG_NVS
   #define DEBUG_WIFI
   #define DEBUG_AP_SERVER
   #include "esp_log.h"
#endif


#define SENSORES

// Funçoes genéricas
#define MAKE8(a, i)        ((u8_t)(a >> (8 * (3 - i)) & 0xff))
#define MAKE32(a, b, c, d) (((u32_t)((a) &0xff) << 24) | ((u32_t)((b) &0xff) << 16) | ((u32_t)((c) &0xff) << 8) | (u32_t)((d) &0xff))
#define BIT_TEST(a, p)     ((a & ((int) 1 << p)) >> p)
#define BIT_SET(a, b)      (a |= 1 << b)
#define BIT_CLEAR(a, b)    (a &= ~(1 << b))
#define MIN(a,b) (b-a)

// Controle e monitoramento das tarefas
static TaskHandle_t taskhandle_app_controller;
static EventGroupHandle_t app_event_group;
const int CONECTADO_NO_WIFI = BIT0;

static struct {
   char nome[32];
   char passwifi[64];
   char versao_sw[10];
   char key[16];
   char passconfig[6];
} modulo = {
   .nome = "WiFi-Manager-AP",
   .passwifi = "wifimanager",
   .versao_sw = "1.0",
   .key = "sdhjkqh1215a6sd",
   .passconfig = "1234"
};

// Definições ------------------------------------------------------------------
#define MAX_LOGIN_WIFI                 5
#define ID_NULL                        -1
#define WIFI_TEMPO_SCAN_AP_OFF         300
#define WIFI_RETRI_SCAN_TIMEOUT_AP_OFF 2500
#define WIFI_TEMPO_CONECTAR_AO_AP      10000 // Timeout de tentativa de conexão
#define ESP_WIFI_SERVER_MAX_CON        4

// Variáveis ------------------------------------------------------------------
typedef enum {
   DESCONECTADO,
   PROCURANDO_WIFI,
   CONECTANDO,
   AGUARDANDO_IP,
   CONECTADO,
   DESABILITADO
} app_wifi_sta_status_t;

static TaskHandle_t taskhandle_wifi_app_control;
static SemaphoreHandle_t mutex_pesquisa_wifi;
static SemaphoreHandle_t mutex_pacote_tempo_real;
uint8_t mac_address[6] = {0, 0, 0, 0, 0, 0};

static struct {
   app_wifi_sta_status_t status;
   int id_scan;
   int id_connected;
   int retri_scan_timeout;
   int tempo_scan;
	uint32_t tempo_envio_timeout;
   uint32_t tempo_erro_envio_timeout;
} app_wifi_sta = {
   .status = DESABILITADO,
   .id_scan = ID_NULL,
   .id_connected = ID_NULL,
   .retri_scan_timeout = WIFI_RETRI_SCAN_TIMEOUT_AP_OFF,
   .tempo_scan = WIFI_TEMPO_SCAN_AP_OFF,
	.tempo_envio_timeout = 0,
   .tempo_erro_envio_timeout = 0};

static int conectar_ao_ap_timeout = WIFI_TEMPO_CONECTAR_AO_AP; // Pendente

typedef struct {
   uint32_t ip;
   uint32_t netmask;
   uint32_t gw;
   uint32_t dns1;
   uint32_t dns2;
} net_ipv4;

typedef struct login_wifi_t{
   char ssid[32];
   char pass[64];
   bool falhou_conectar;
   bool status_procura;
   net_ipv4 net_ip4;
   bool ip4_fix;
} login_wifi_t;
login_wifi_t login_wifi[MAX_LOGIN_WIFI];

// Variavel para criação do ponto de acesso Wi-Fi
struct {
   char ssid[32];
   char pass[64];
   tcpip_adapter_ip_info_t net;
} wifi_ap = {
   .ssid = "WiFi-Manager-Ap",
   .pass = "teste123",
};

// Para montar Json de tempo real
char json_tempo_real[650] = "{}";
uint32_t contagem_teste = 0;

// Protótipos do main.c
static void atualiza_pacote_tempo_real(bool realtime);

// Bibliotecas ----------------------------------------------------------------

// TIMER ------------------------------
void timer_isr_app(void);
#define TIMER_ISR_VOID timer_isr_app()
#define APP_COUNT_TIMER_MS 1000 // 10 ms
#include "_libs/_TIMER_V1.0.c"

// LED Status -------------------------
#define LED_STATUS (GPIO_NUM_2)
#define LED_STATUS_BASE_TEMPO_MS 1
#include "_libs/_LED_STATUS.c"

// Biblioteca NVS ---------------------
#include "_libs/_NVS_V1.0.c"

// Primeiro cria as variáveis e protótipos das funções
#include "_Wifi/Wifi.h"
#include "_AP_Server/AP_Server.h"

// Depois de desclarar todas as variáveis e protótipos.
#include "_Wifi/Wifi.c"
#include "_AP_Server/AP_Server.c"



