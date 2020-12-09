// Trata Os eventos do wifi.
static esp_err_t event_handler(void *ctx, system_event_t *event)
{
   switch (event->event_id)
   {
   case SYSTEM_EVENT_AP_START:
#ifdef DEBUG_WIFI
      ESP_LOGI(LOG_WIFI, "AP Start");
#endif
      break;
   case SYSTEM_EVENT_AP_STACONNECTED:
#ifdef DEBUG_WIFI
      ESP_LOGI(LOG_WIFI, "Disp. conectou no ESP");
      ESP_LOGI(LOG_WIFI, "station:" MACSTR " join, AID=%d", MAC2STR(event->event_info.sta_connected.mac),
               event->event_info.sta_connected.aid);
#endif
      break;
   case SYSTEM_EVENT_AP_STADISCONNECTED: // Dispositivo desconectou do ESP
#ifdef DEBUG_WIFI
      ESP_LOGI(LOG_WIFI, "Disp. desconectou do ESP");
      ESP_LOGI(LOG_WIFI, MACSTR "leave, AID=%d",
               MAC2STR(event->event_info.sta_disconnected.mac),
               event->event_info.sta_disconnected.aid);
#endif
      break;
   case SYSTEM_EVENT_STA_CONNECTED: // Conectou no roteador
      app_wifi_sta.status = AGUARDANDO_IP;
#ifdef DEBUG_WIFI
      ESP_LOGI(LOG_WIFI, "Conectou no Rot.");
#endif
      break;
   case SYSTEM_EVENT_AP_STAIPASSIGNED: // Dispositivo conectou no ESP e recebeu IP

#ifdef DEBUG_WIFI
      ESP_LOGI(LOG_WIFI, "Disp. Conectou.");
#endif
      break;
   case SYSTEM_EVENT_STA_START:
#ifdef DEBUG_WIFI
      ESP_LOGI(LOG_WIFI, "Evento STA start.");
#endif
      break;
   case SYSTEM_EVENT_STA_GOT_IP: // IPV4 ----------------------
      app_wifi_sta.status = CONECTADO;

      xEventGroupSetBits(app_event_group, CONECTADO_NO_WIFI);
#ifdef DEBUG_WIFI
      ESP_LOGI(LOG_WIFI, "Recebeu IPV4:%s", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
#endif

      app_wifi_sta.id_connected = app_wifi_sta.id_scan;

      break;
   case SYSTEM_EVENT_STA_DISCONNECTED: // Desconectou do Roteador
      app_wifi_sta.status = DESCONECTADO;
      //realtime.link_internet = 0;
      login_wifi[app_wifi_sta.id_scan].falhou_conectar = 1; // Seta que ja tentou conectar
      app_wifi_sta.id_connected = ID_NULL;
      app_wifi_sta.id_scan = ID_NULL;

      xEventGroupClearBits(app_event_group, CONECTADO_NO_WIFI);
#ifdef DEBUG_WIFI
      ESP_LOGI(LOG_WIFI, "Desconectou do Rot.");
#endif
      break;
   case SYSTEM_EVENT_SCAN_DONE:
      break;
   case SYSTEM_EVENT_STA_LOST_IP:
#ifdef DEBUG_WIFI
      ESP_LOGI(LOG_WIFI, "IP ruim.");
#endif
      esp_wifi_disconnect();
      break;
   default:
#ifdef DEBUG_WIFI
      ESP_LOGI(LOG_WIFI, "Evento Inválido ->%d", event->event_id);
#endif
      break;
   }
   return ESP_OK;
}

// Configura o ponto de acesso.
static esp_err_t wifi_ap_config()
{
   wifi_config_t wifi_conf = {
       .ap = {
           .max_connection = ESP_WIFI_SERVER_MAX_CON,
           .authmode = WIFI_AUTH_WPA_WPA2_PSK,
           //.channel = ESP_WIFI_SERVER_CHANNEL,
           .beacon_interval = 500},
   };

   // Monta nome do Wi-Fi do servidor
   if (strlen(wifi_ap.ssid) < 32)
      sprintf((char *)wifi_conf.ap.ssid, "%s", (char *)wifi_ap.ssid);
   else
      sprintf((char *)wifi_conf.ap.ssid, "%s", (char *)"WiFi-Manager-AP");

   if (strlen(wifi_ap.pass) < 8)
      strcpy((char *)wifi_conf.ap.password, "teste123");
   else
      strcpy((char *)wifi_conf.ap.password, wifi_ap.pass);

   int len = strlen((char *)wifi_conf.ap.ssid);
   wifi_conf.ap.ssid_len = len;
   if (len == 0)
      wifi_conf.ap.authmode = WIFI_AUTH_OPEN;

   return esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_conf);
}

// Configura o IP do ponto de acesso como fixo.
static void config_tcpip_ap_fix()
{
   // Atribui ip fixo para o AP
   IP4_ADDR(&wifi_ap.net.ip, 10, 0, 0, 1);
   IP4_ADDR(&wifi_ap.net.gw, 10, 0, 0, 1);
   IP4_ADDR(&wifi_ap.net.netmask, 255, 255, 255, 248);           //6 hosts => 0:rede, 7:broadcast, 1:gw(esp), sobrou 5, o esp conecta no maximo 4.
   tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP);                // Para servi�o DHCP
   tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &wifi_ap.net); // Seta o IP
   tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_AP, "wifimanager");
   tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP); // Inicia servi�o DHCP
}

// Configura o login e senha para conectar o ESP (STA) em um Wi-Fi disponivel.
static esp_err_t wifi_sta_config(int id)
{
   wifi_config_t wifi_conf_sta = {
       .sta = {
           .scan_method = WIFI_FAST_SCAN,
           .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
       }};

#ifdef DEBUG_WIFI
   ESP_LOGI(LOG_WIFI, "wifi_sta_config");
#endif

   app_wifi_sta.id_scan = id;
   app_wifi_sta.id_connected = ID_NULL;

   if (id >= 0)
   {
      // Verifica se as configurações são válidas e atribui a struct de configuração do wifi.
      if ((strlen(login_wifi[id].ssid) > 0) && (strlen(login_wifi[id].pass) > 7))
      {
#ifdef DEBUG_WIFI
         ESP_LOGI(LOG_WIFI, "Config -> %s", login_wifi[id].ssid);
#endif

         sprintf((char *)wifi_conf_sta.sta.ssid, "%s", (char *)login_wifi[id].ssid);
         sprintf((char *)wifi_conf_sta.sta.password, "%s", (char *)login_wifi[id].pass);
      }
   }

   // Verifica se o Cliente DHCP está ativado.
   tcpip_adapter_dhcp_status_t status = 0;
   tcpip_adapter_dhcpc_get_status(TCPIP_ADAPTER_IF_STA, &status);
#ifdef DEBUG_WIFI
   ESP_LOGI(LOG_WIFI, "Status DHCPC:%d", status);
#endif

   if ((login_wifi[id].ip4_fix == 1) && (id >= 0))
   { // DHCPC
      if (status != TCPIP_ADAPTER_DHCP_STOPPED)
      {
         tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA);
      }

      tcpip_adapter_dns_info_t dns;
      tcpip_adapter_ip_info_t ipv4_config;
      memset(&ipv4_config, 0, sizeof(ipv4_config));
      memset(&dns, 0, sizeof(dns));

#ifdef DEBUG_WIFI
      ESP_LOGI(LOG_WIFI, "IP: %d.%d.%d.%d",
               MAKE8(login_wifi[id].net_ip4.ip, 0),
               MAKE8(login_wifi[id].net_ip4.ip, 1),
               MAKE8(login_wifi[id].net_ip4.ip, 2),
               MAKE8(login_wifi[id].net_ip4.ip, 3));
      ESP_LOGI(LOG_WIFI, "GW: %d.%d.%d.%d",
               MAKE8(login_wifi[id].net_ip4.gw, 0),
               MAKE8(login_wifi[id].net_ip4.gw, 1),
               MAKE8(login_wifi[id].net_ip4.gw, 2),
               MAKE8(login_wifi[id].net_ip4.gw, 3));
      ESP_LOGI(LOG_WIFI, "NETMASK: %d.%d.%d.%d",
               MAKE8(login_wifi[id].net_ip4.netmask, 0),
               MAKE8(login_wifi[id].net_ip4.netmask, 1),
               MAKE8(login_wifi[id].net_ip4.netmask, 2),
               MAKE8(login_wifi[id].net_ip4.netmask, 3));
      ESP_LOGI(LOG_WIFI, "DNS: %d.%d.%d.%d",
               MAKE8(login_wifi[id].net_ip4.dns1, 0),
               MAKE8(login_wifi[id].net_ip4.dns1, 1),
               MAKE8(login_wifi[id].net_ip4.dns1, 2),
               MAKE8(login_wifi[id].net_ip4.dns1, 3));
      ESP_LOGI(LOG_WIFI, "DNS2: %d.%d.%d.%d",
               MAKE8(login_wifi[id].net_ip4.dns2, 0),
               MAKE8(login_wifi[id].net_ip4.dns2, 1),
               MAKE8(login_wifi[id].net_ip4.dns2, 2),
               MAKE8(login_wifi[id].net_ip4.dns2, 3));
#endif

      IP4_ADDR(&ipv4_config.ip,
               MAKE8(login_wifi[id].net_ip4.ip, 0),
               MAKE8(login_wifi[id].net_ip4.ip, 1),
               MAKE8(login_wifi[id].net_ip4.ip, 2),
               MAKE8(login_wifi[id].net_ip4.ip, 3));
      IP4_ADDR(&ipv4_config.gw,
               MAKE8(login_wifi[id].net_ip4.gw, 0),
               MAKE8(login_wifi[id].net_ip4.gw, 1),
               MAKE8(login_wifi[id].net_ip4.gw, 2),
               MAKE8(login_wifi[id].net_ip4.gw, 3));
      IP4_ADDR(&ipv4_config.netmask,
               MAKE8(login_wifi[id].net_ip4.netmask, 0),
               MAKE8(login_wifi[id].net_ip4.netmask, 1),
               MAKE8(login_wifi[id].net_ip4.netmask, 2),
               MAKE8(login_wifi[id].net_ip4.netmask, 3));
      IP_ADDR4(&dns.ip,
               MAKE8(login_wifi[id].net_ip4.dns1, 0),
               MAKE8(login_wifi[id].net_ip4.dns1, 1),
               MAKE8(login_wifi[id].net_ip4.dns1, 2),
               MAKE8(login_wifi[id].net_ip4.dns1, 3));

      ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &ipv4_config));
      ESP_ERROR_CHECK(tcpip_adapter_set_dns_info(TCPIP_ADAPTER_IF_STA, TCPIP_ADAPTER_DNS_MAIN, &dns));

#ifdef DEBUG_WIFI
      ESP_LOGI(LOG_WIFI, "Configurou o IPV4:%s", ip4addr_ntoa(&ipv4_config.ip));
#endif
   }
   else
   {
      if (status != TCPIP_ADAPTER_DHCP_STARTED)
      {
         tcpip_adapter_dhcpc_start(TCPIP_ADAPTER_IF_STA);
      }
   }

   return esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_conf_sta);
}

// Faz a procura de Wifis e compara se existe um Wi-Fi conhecido para conectar.
static int wifi_app_start_scan()
{
   int id = ID_NULL;
   int i = 0;
   int j = 0;
   int count = 0;
   int ret = ESP_FAIL;

   // Verifica se todos ja foram testados e limpa limpa status de procura ------
   for (i = 0, count = 0; i < MAX_LOGIN_WIFI; i++)
   {
      if (login_wifi[i].falhou_conectar)
         count++;
      login_wifi[i].status_procura = 0; // limpa status procura

#ifdef DEBUG_WIFI
      ESP_LOGW(LOG_WIFI, "AP%02d:'%s'-'%s'-%d-%d", i, login_wifi[i].ssid, login_wifi[i].pass, login_wifi[i].status_procura, login_wifi[i].falhou_conectar);
#endif
   }

   // Se todos ja testados, limpa pra tentar novamente nos que deram errado
   if (count >= MAX_LOGIN_WIFI)
   {
#ifdef DEBUG_WIFI
      ESP_LOGI(LOG_WIFI, "Limpando!");
#endif
      for (i = 0; i < MAX_LOGIN_WIFI; i++)
      {
         login_wifi[i].falhou_conectar = 0;
      }
   }

   // Inicia pesquisa --------------------
   wifi_scan_config_t scan_conf = {
       .ssid = NULL,
       .bssid = NULL,
       //.channel = 0,
       .show_hidden = true,
       .scan_time.passive = app_wifi_sta.tempo_scan, // Tempo muda se algum dispositivo conectado no AP do esp.
       .scan_type = WIFI_SCAN_TYPE_PASSIVE};

   uint16_t length = 0;

   // inicia pesquisa se o mutex estiver liberado -----------------------------
   xSemaphoreTake(mutex_pesquisa_wifi, portMAX_DELAY);

   // Inicia pesquisa
   ret = esp_wifi_scan_start(&scan_conf, true);
   if (ret != ESP_OK)
      goto fim;

   // Busca encontrados
   ret = esp_wifi_scan_get_ap_num((uint16_t *)&length);
   if (ret != ESP_OK)
      goto fim;

   // Se nenhum Wi-Fi encontrado
   if (length == 0)
      goto fim;

   wifi_ap_record_t *list = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * length);

   // printf("Number of access points found: %d\n", event->event_info.scan_done.number);
   ret = esp_wifi_scan_get_ap_records(&length, list);
   if (ret != ESP_OK)
      goto fim2;

   xSemaphoreGive(mutex_pesquisa_wifi);

   // -------------------------------------------------------------------------

   // Verifica todos os AP encontrados
   for (j = 0; j < length; j++)
   {
#ifdef DEBUG_WIFI
      ESP_LOGI(LOG_WIFI, "AP%02d:'%s'-%d-%d-%d", j, list[j].ssid, list[j].primary, list[j].rssi, list[j].authmode);
#endif

      // Rejeita Wi-Fi sem nome
      if (strlen((char *)list[j].ssid) < 4)
         continue;

      // Verifica se existe destre os configurados
      for (i = 0; i < MAX_LOGIN_WIFI; i++)
      {
         if (strlen((char *)login_wifi[i].ssid) != strlen((char *)list[j].ssid))
            continue;
         if (strstr((char *)list[j].ssid, (char *)login_wifi[i].ssid) == NULL)
            continue;
         login_wifi[i].status_procura = 1;
      }
   }

   // Se encontrado e ainda não falhou ao conectar, tenta conexão
   for (i = 0; i < MAX_LOGIN_WIFI; i++)
   {
      if ((login_wifi[i].falhou_conectar == 0))
      {
         if (login_wifi[i].status_procura == 1)
         {
            id = i;
            goto fim2;
         }
         else
         {
            login_wifi[i].falhou_conectar = 1;
         }
      }
   }

   // Ainda tem algum que não tentou conectar, porem não está na pesquisa...
   for (i = 0; i < MAX_LOGIN_WIFI; i++)
   {
      if ((login_wifi[i].falhou_conectar == 0) && (login_wifi[i].status_procura == 0))
      {
         id = i;
         goto fim2;
      }
   }

fim2:
   free(list);
   return id;
fim:
   xSemaphoreGive(mutex_pesquisa_wifi);
   return id;
}

// Inicia o modulo Wi-Fi do ESP no modo AP-STA.
static void wifi_init()
{
#ifdef DEBUG_WIFI
   ESP_LOGI(LOG_WIFI, "APP init.");
#endif

   mutex_pesquisa_wifi = xSemaphoreCreateMutex();

#ifdef DEBUG_WIFI
   if (mutex_pesquisa_wifi != NULL)
   {
      ESP_LOGI(LOG_WIFI, "mutex ok");
   }
   else
      ESP_LOGI(LOG_WIFI, "mutex erro");
#endif

   // Inicia o servi�o TCPIP
   tcpip_adapter_init();

   // Inicializa driver TCP/IP
   config_tcpip_ap_fix();

   wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

   ESP_ERROR_CHECK(esp_wifi_init(&cfg));
   ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

#ifdef DEBUG_WIFI
   ESP_LOGI(LOG_WIFI, "APP config");
#endif

   ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
   ESP_ERROR_CHECK(wifi_ap_config());
   ESP_ERROR_CHECK(wifi_sta_config(app_wifi_sta.id_connected));
   ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

#ifdef DEBUG_WIFI
   ESP_LOGI(LOG_WIFI, "APP start.");
#endif

   ESP_ERROR_CHECK(esp_wifi_start());
}

// Função para desconectar do Wi-Fi e desabilitar a reconexão.
static void desconectar_do_wifi()
{
   app_wifi_sta.status = DESCONECTADO;
   login_wifi[app_wifi_sta.id_scan].falhou_conectar = 1; // Seta que ja tentou conectar
   app_wifi_sta.id_connected = ID_NULL;
   app_wifi_sta.id_scan = ID_NULL;
   esp_wifi_disconnect();
}

// Função deve ser adicionada ao um timer de 1ms para contar corretamente.
static void wifi_timer_ms()
{
   if (app_wifi_sta.retri_scan_timeout)
      app_wifi_sta.retri_scan_timeout--;
   if ((app_wifi_sta.status == CONECTANDO) && (conectar_ao_ap_timeout))
      conectar_ao_ap_timeout--;
}

// Função que verifique periodicamente os status do wifi e tarfas relacionadas.
static void wifi_app_control(void *pvParameters)
{
   int ret = 0;

   // Inicia variaveis do wifi
   app_wifi_sta.retri_scan_timeout = WIFI_RETRI_SCAN_TIMEOUT_AP_OFF;
   app_wifi_sta.tempo_scan = WIFI_TEMPO_SCAN_AP_OFF;
   app_wifi_sta.status = DESABILITADO;
   ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

   wifi_init(); // Inicia o Wi-Fi

   esp_wifi_get_mac(ESP_IF_WIFI_STA, mac_address);

   // Inicia Serviço AP
   start_webserver();
   int conta_tentativas_conexao_ap = 0;
   
   while (1)
   {
      switch (app_wifi_sta.status)
      {
      case DESCONECTADO:
      case DESABILITADO:
         if (!app_wifi_sta.retri_scan_timeout)
         {
            app_wifi_sta.status = PROCURANDO_WIFI;

            if ((ret = wifi_app_start_scan()) >= 0)
            {
               wifi_sta_config(ret); // Configura o Wi-Fi e tenta conectar
               conectar_ao_ap_timeout = WIFI_TEMPO_CONECTAR_AO_AP;
               app_wifi_sta.status = CONECTANDO;
               esp_wifi_connect();
            }
            else
            {  
               // Não encontrou na pesquisa, recarrega tempo
               app_wifi_sta.status = DESCONECTADO;
               if (++conta_tentativas_conexao_ap >= WIFI_QUANTIDADE_DE_PROCURAS)
               {
                  app_wifi_sta.retri_scan_timeout = WIFI_TEMPO_RETORNO_DE_PROCURA;
               }
               else {
                  app_wifi_sta.retri_scan_timeout = WIFI_RETRI_SCAN_TIMEOUT_AP_OFF;
               }
            }
         }
         break;
      case CONECTANDO:
         if (!conectar_ao_ap_timeout)
            desconectar_do_wifi();
         break;
      case CONECTADO:
         // Pendente - identificar que o servi�o de envio travou (talvez seja o timeout do wifi)
         // if ((!rest_http.envio_timeout) && (rest_http.enviando_mensagem)) desconectar_do_wifi();
         break;
      default:
         break;
      }

      vTaskDelay(250 / portTICK_PERIOD_MS);
   }
}

// Inicia tarefa de cntole do Wi-Fi .
static void wifi_app_init()
{
   xTaskCreate(wifi_app_control, "wifi_app_control", 10000, NULL, 2, &taskhandle_wifi_app_control);
}

static char *gera_json_scanwifi()
{
   // Se uma procura ja estiver acontecendo, retorna erro.
   while ((app_wifi_sta.status == CONECTANDO) || (app_wifi_sta.status == AGUARDANDO_IP))
      ;

   // Inicia pesquisa ---------------------------------------------------------
   wifi_scan_config_t scan_conf = {
       .ssid = NULL,
       .bssid = NULL,
       .show_hidden = true,
       .scan_time.passive = app_wifi_sta.tempo_scan, // Tempo muda se algum dispositivo conectado no AP do esp.
       .scan_type = WIFI_SCAN_TYPE_PASSIVE};
   uint16_t apCount = 0;

   xSemaphoreTake(mutex_pesquisa_wifi, portMAX_DELAY);
   esp_err_t ret = ESP_FAIL;

   // Inicia pesquisa e aguarda
   ret = esp_wifi_scan_start(&scan_conf, true);
   if (ret != ESP_OK)
      goto fim;

   // Inicia pesquisa e aguarda
   ret = esp_wifi_scan_get_ap_num(&apCount);
   if (ret != ESP_OK)
      goto fim;

   // Reserva espaço dinamicamente para a lista de Wi-Fi
   wifi_ap_record_t *list = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * apCount);
   if (list == NULL)
      goto fim;

   ret = esp_wifi_scan_get_ap_records(&apCount, list);
   if (ret == ESP_OK)
   {
      char *p = (char *)malloc(2000 * sizeof(char)); // Aloca dinamicamente

      if (p == NULL)
         goto fim2;
      char *p_ini = (char *)p;

      p += sprintf(p, "{"); // Abre Json
      for (int i = 0; i < apCount; i++)
      {
         if (strlen((char *)list[i].ssid) == 0)
            continue; // Remover da lista os SSID sem nome.
         if (p - p_ini > 1850)
            break; // garantir que não vai estourar o vetor de 2000, 64 + 32 + header(50) = 150 de garantia;
         p += sprintf(p, "\"%u\":{", i);
         p += sprintf(p, "\"ssid\":\"%s\",", list[i].ssid);
         p += sprintf(p, "\"ch\":%d,", list[i].primary);
         p += sprintf(p, "\"rssi\":%d,", list[i].rssi);
         p += sprintf(p, "\"authmode\":%d},", list[i].authmode);
      }
      p += sprintf(p - 1, "}"); // Fecha Json
      free(list);
      xSemaphoreGive(mutex_pesquisa_wifi);
      return p_ini;
   }

fim2:
   free(list);

fim:
   xSemaphoreGive(mutex_pesquisa_wifi);
   return NULL;
}

static cJSON *gera_json_configwifi()
{
   cJSON *json_configwifi = cJSON_CreateObject();

   uint8_t i = 0;
   char buffer[20];

   for (i = 0; i < MAX_LOGIN_WIFI; i++)
   {
      cJSON *rede_wifi = cJSON_CreateObject();

      cJSON_AddStringToObject(rede_wifi, "ssid", login_wifi[i].ssid);

      if (strlen(login_wifi[i].pass) < 4)
         cJSON_AddBoolToObject(rede_wifi, "pass", 0);
      else
         cJSON_AddBoolToObject(rede_wifi, "pass", 1);

      cJSON_AddBoolToObject(rede_wifi, "con", (i == app_wifi_sta.id_connected));

      cJSON_AddBoolToObject(rede_wifi, "ip4_fix", login_wifi[i].ip4_fix);

      if (login_wifi[i].net_ip4.ip != 0)
      {
         sprintf(buffer, "%d.%d.%d.%d",
                 MAKE8(login_wifi[i].net_ip4.ip, 0),
                 MAKE8(login_wifi[i].net_ip4.ip, 1),
                 MAKE8(login_wifi[i].net_ip4.ip, 2),
                 MAKE8(login_wifi[i].net_ip4.ip, 3));
         cJSON_AddStringToObject(rede_wifi, "ip4_ip", buffer);
      }
      if (login_wifi[i].net_ip4.gw != 0)
      {
         sprintf(buffer, "%d.%d.%d.%d",
                 MAKE8(login_wifi[i].net_ip4.gw, 0),
                 MAKE8(login_wifi[i].net_ip4.gw, 1),
                 MAKE8(login_wifi[i].net_ip4.gw, 2),
                 MAKE8(login_wifi[i].net_ip4.gw, 3));
         cJSON_AddStringToObject(rede_wifi, "ip4_gw", buffer);
      }
      if (login_wifi[i].net_ip4.netmask != 0)
      {
         sprintf(buffer, "%d.%d.%d.%d",
                 MAKE8(login_wifi[i].net_ip4.netmask, 0),
                 MAKE8(login_wifi[i].net_ip4.netmask, 1),
                 MAKE8(login_wifi[i].net_ip4.netmask, 2),
                 MAKE8(login_wifi[i].net_ip4.netmask, 3));
         cJSON_AddStringToObject(rede_wifi, "ip4_netmask", buffer);
      }
      if (login_wifi[i].net_ip4.dns1 != 0)
      {
         sprintf(buffer, "%d.%d.%d.%d",
                 MAKE8(login_wifi[i].net_ip4.dns1, 0),
                 MAKE8(login_wifi[i].net_ip4.dns1, 1),
                 MAKE8(login_wifi[i].net_ip4.dns1, 2),
                 MAKE8(login_wifi[i].net_ip4.dns1, 3));
         cJSON_AddStringToObject(rede_wifi, "ip4_dns1", buffer);
      }

      if (login_wifi[i].net_ip4.dns2 != 0)
      {
         sprintf(buffer, "%d.%d.%d.%d",
                 MAKE8(login_wifi[i].net_ip4.dns2, 0),
                 MAKE8(login_wifi[i].net_ip4.dns2, 1),
                 MAKE8(login_wifi[i].net_ip4.dns2, 2),
                 MAKE8(login_wifi[i].net_ip4.dns2, 3));
         cJSON_AddStringToObject(rede_wifi, "ip4_dns2", buffer);
      }

      sprintf(buffer, "%u", i);
      cJSON_AddItemToObject(json_configwifi, buffer, rede_wifi);
   }

   return json_configwifi;
}
