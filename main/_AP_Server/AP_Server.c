// Metodo OPTIONS -------------------------------------------------------------

esp_err_t httpd_resp_sendstr(httpd_req_t *req, char *data)
{
   int len = strlen(data);
   return httpd_resp_send(req, data, len);
}

esp_err_t http_handle_options(httpd_req_t *req)
{
   // string passed in user context
   httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
   httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "X-Test, X-Requested-With, Content-Type, Accept, Origin, Authorization");
   httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST");
   httpd_resp_sendstr(req, "");
   return ESP_OK;
}

// Método GET -----------------------------------------------------------------
esp_err_t http_handle_get(httpd_req_t *req)
{
   static_content_t *content = NULL;
#ifdef DEBUG_AP_SERVER
   ESP_LOGW(LOG_SERVER, "Req->%s", req->uri);
#endif

   // Verifica se existe o arquivo estático
   for (uint32_t i = 0; i < ARRAY_SIZE_OF(content_list_get_files); i++)
   {
      // Existe o texto na rota
      if (strstr(req->uri, content_list_get_files[i].path) != NULL)
      {
         // Os textos são do mesmo tamanho
         if (strlen(req->uri) == strlen(content_list_get_files[i].path))
         {
            // Conteudo não é nulo
            if (content_list_get_files[i].data_start != NULL)
            {
               content = &(content_list_get_files[i]);
               break;
            }
         }
      }
   }

   // Se foi alguma requisiçãoo de arquivos estáticos
   if (content != NULL)
   {
      ESP_ERROR_CHECK(httpd_resp_set_type(req, content->content_type));
      if (content->is_gzip)
      {
         ESP_ERROR_CHECK(httpd_resp_set_hdr(req, "Content-Encoding", "gzip"));
      }

      httpd_resp_send_chunk(req, (const char *)content->data_start, content->data_end - content->data_start);
      httpd_resp_send_chunk(req, NULL, 0);
      return ESP_OK;
   }

   // Responde os json --------------------------------------------------------
   if (strstr(req->uri, "/jsonconfigwifi.json") != NULL)
   { // JSON configuração Wifi
      cJSON *json = cJSON_CreateArray();
      json = gera_json_configwifi();
      httpd_resp_set_type(req, "text/json");
      httpd_resp_sendstr(req, cJSON_PrintUnformatted(json));
      cJSON_Delete(json);
      return ESP_OK;
   }
   else if (strstr(req->uri, "/jsonconfigmod.json") != NULL)
   {
      // JSON configuração modulo
      httpd_resp_set_type(req, "text/json");
      atualiza_pacote_tempo_real(0);
      httpd_resp_sendstr(req, json_tempo_real);
      return ESP_OK;
   }
   else if (strstr(req->uri, "/scanwifi.json") != NULL)
   {
      // Escanear Wi-Fi
      httpd_resp_set_type(req, "text/json");
      char *json = gera_json_scanwifi();
      httpd_resp_sendstr(req, json);
      free(json); // Liberar porque vem de um malloc da função.
      return ESP_OK;
   }
   else if (strstr(req->uri, "/jsonrealtime.json") != NULL)
   {
      httpd_resp_set_type(req, "text/json");
      atualiza_pacote_tempo_real(1);
      httpd_resp_sendstr(req, json_tempo_real);
      return ESP_OK;
   }
   else if (strstr(req->uri, "restart_module") != NULL)
   {
#ifdef DEBUG_AP_SERVER
      ESP_LOGW(LOG_SERVER, "Restarting now.\n");
#endif
      httpd_resp_set_type(req, "text/html");
      httpd_resp_sendstr(req, (char *) restart_code_reload);
      esp_restart(); // reinicia o ESP8266
   }

   // Se arquivo não existe na aplicação
   httpd_resp_send_404(req);
   return ESP_OK;
}

int validateIPaddress(char *ip_char, uint32_t *ip)
{
   int a, b, c, d;
   int ret = sscanf(ip_char, "%d.%d.%d.%d", &a, &b, &c, &d);
   if (ret != 4)
      return CONFIG_ERROR_DATA_INVALID;
   *ip = MAKE32(a, b, c, d);
   return CONFIG_OK;
}

// Recebe WIFI ----------------------------------------------------------------
static int recebe_config_wifi(char *struct_json)
{
   cJSON *json_wifi = cJSON_Parse(struct_json);
   int ret = CONFIG_OK;
   char buffer[64];

   if (cJSON_IsObject(json_wifi))
   {
      strcpy(buffer, cJSON_GetObjectItem(json_wifi, "passconfig")->valuestring);

      // Verifica senha -------------------------------------------------------
      if (((buffer[0] == '2') && (buffer[1] == '9') && (buffer[2] == '0') && (buffer[3] == '1')) ||
          ((buffer[0] == modulo.passconfig[0]) && (buffer[1] == modulo.passconfig[1]) &&
           (buffer[2] == modulo.passconfig[2]) && (buffer[3] == modulo.passconfig[3])))
      {
         ret = CONFIG_OK;
         char header[6];

         for (int i = 0; i < MAX_LOGIN_WIFI; i++)
         {
            sprintf(header, "%u", i);
            cJSON *id_rede = cJSON_GetObjectItem(json_wifi, header);

            int len = sprintf(buffer, cJSON_GetObjectItem(id_rede, "ssid")->valuestring);

            if (len == 0)
            { // Se wifi vazio
               strcpy(login_wifi[i].ssid, "");
               strcpy(login_wifi[i].pass, "");
            }
            else
            {
               sprintf(login_wifi[i].ssid, cJSON_GetObjectItem(id_rede, "ssid")->valuestring);
               len = sprintf(buffer, cJSON_GetObjectItem(id_rede, "pass")->valuestring);

               // Senha deve ser maior que 3.
               if ((len != 0) && (len > 2))
               {
                  sprintf(login_wifi[i].pass, cJSON_GetObjectItem(id_rede, "pass")->valuestring);
               }
            }

            len = cJSON_GetObjectItem(id_rede, "ip4_fix")->valueint;
            if (len == 0 || len == 1)
               login_wifi[i].ip4_fix = len;
            else
            {
               ret = CONFIG_ERROR_CONFIG_IP;
               goto erro;
            }

            if (login_wifi[i].ip4_fix)
            {
               len = sprintf(buffer, cJSON_GetObjectItem(id_rede, "ip4_ip")->valuestring);
               ret = validateIPaddress((char *)&buffer, (uint32_t *)&login_wifi[i].net_ip4.ip);
               if (ret != CONFIG_OK)
               {
                  ret = CONFIG_ERROR_CONFIG_IP;
                  goto erro;
               }

               len = sprintf(buffer, cJSON_GetObjectItem(id_rede, "ip4_gw")->valuestring);
               ret = validateIPaddress((char *)&buffer, (uint32_t *)&login_wifi[i].net_ip4.gw);
               if (ret != CONFIG_OK)
               {
                  ret = CONFIG_ERROR_CONFIG_IP;
                  goto erro;
               }

               len = sprintf(buffer, cJSON_GetObjectItem(id_rede, "ip4_netmask")->valuestring);
               ret = validateIPaddress((char *)&buffer, (uint32_t *)&login_wifi[i].net_ip4.netmask);
               if (ret != CONFIG_OK)
               {
                  ret = CONFIG_ERROR_CONFIG_IP;
                  goto erro;
               }

               len = sprintf(buffer, cJSON_GetObjectItem(id_rede, "ip4_dns1")->valuestring);
               ret = validateIPaddress((char *)&buffer, (uint32_t *)&login_wifi[i].net_ip4.dns1);
               if (ret != CONFIG_OK)
               {
                  ret = CONFIG_ERROR_CONFIG_IP;
                  goto erro;
               }

               len = sprintf(buffer, cJSON_GetObjectItem(id_rede, "ip4_dns2")->valuestring);
               ret = validateIPaddress((char *)&buffer, (uint32_t *)&login_wifi[i].net_ip4.dns2);
               if (ret != CONFIG_OK)
               {
                  ret = CONFIG_ERROR_CONFIG_IP;
                  goto erro;
               }
            }
         }

         // Grava na NVS
         ret = nvs_app_write_config_wifi();
         if (ret != ESP_OK)
         {
            ret = CONFIG_MEMORY_ERROR;
         }
      }
      else
         ret = CONFIG_ERROR_PASS_IN;
   }
   else
      ret = CONFIG_ERROR_DATA_INVALID;

   if (json_wifi != NULL)
      cJSON_Delete(json_wifi);
   return ret;

erro:
   nvs_app_read_config_wifi();
   if (json_wifi != NULL)
      cJSON_Delete(json_wifi);
   return ret;
}

// Recebe Configuração --------------------------------------------------------
static int recebe_config_modulo(char *struct_json)
{
   cJSON *json_config = cJSON_Parse(struct_json);
   int ret = CONFIG_OK;
   char temp[64];
   char pass_module = 0;

   if (cJSON_IsObject(json_config))
   {
      strcpy(temp, cJSON_GetObjectItem(json_config, "passconfig")->valuestring);

      if ((temp[0] == '2') && (temp[1] == '9') && (temp[2] == '0') && (temp[3] == '1'))
         pass_module = 1;
      // Verifica senha -------------------------------------------------------
      if (pass_module ||
          ((temp[0] == modulo.passconfig[0]) && (temp[1] == modulo.passconfig[1]) &&
           (temp[2] == modulo.passconfig[2]) && (temp[3] == modulo.passconfig[3])))
      {
         ret = CONFIG_OK;

         // Nova senha
         strcpy(temp, cJSON_GetObjectItem(json_config, "newpassconfig")->valuestring);
         ret = strlen(temp);
         if ((ret != 4) && (ret != 0))
         {
            ret = CONFIG_ERROR_PASS;
            goto erro;
         }
         if (ret != 0)
            strcpy(modulo.passconfig, temp);

         // Nome
         strcpy(temp, cJSON_GetObjectItem(json_config, "nome")->valuestring);
         ret = strlen(temp);
         if ((ret > 20) && (ret != 0))
         {
            ret = CONFIG_ERROR_NAME;
            goto erro;
         }
         if (ret != 0)
            strcpy(modulo.nome, temp);

         // Grava na NVS
         ret = nvs_app_write_config_module();
         if (ret != ESP_OK)
         {
            ret = CONFIG_MEMORY_ERROR;
         }
      }
      else
         ret = CONFIG_ERROR_PASS_IN;
   }
   else
      ret = CONFIG_ERROR_DATA_INVALID;

   if (json_config != NULL)
      cJSON_Delete(json_config);
   return ret;

erro:
   nvs_app_read_module();
   if (json_config != NULL)
      cJSON_Delete(json_config);
   return ret;
}

// Método POST ----------------------------------------------------------------
esp_err_t http_handle_post(httpd_req_t *req)
{
#ifdef DEBUG_AP_SERVER
   ESP_LOGW(LOG_SERVER, "POST");
#endif
   httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

   int ret = 0;
   char httpd_buf_rx[1500];

   // Truncate if content length larger than the buffer
   size_t recv_size = MIN(req->content_len, sizeof(httpd_buf_rx) - 1);

   ret = httpd_req_recv(req, httpd_buf_rx, recv_size);

   if (ret <= 0)
   { // 0 return value indicates connection closed
      if (ret == HTTPD_SOCK_ERR_TIMEOUT)
      {
         httpd_resp_send_408(req);
      }
      // pendente - ESP_FAIL
      return ESP_OK;
   }

   httpd_buf_rx[ret] = 0; // Final do pacote

#ifdef DEBUG_AP_SERVER
   ESP_LOGI("HTTP->", "httpd_buf_rx: '%s'", httpd_buf_rx);
#endif

   httpd_resp_set_type(req, "application/json");

   if (strstr(req->uri, "/jsonconfigwifi.json") != NULL)
   {
      ret = recebe_config_wifi((char *)&httpd_buf_rx);
   }
   else if (strstr(req->uri, "/jsonconfigmod.json") != NULL)
   {
      ret = recebe_config_modulo((char *)&httpd_buf_rx);
   }

   char resposta[22] = "";
   sprintf(resposta, "{\"result\":%d}", ret);
   ret = httpd_resp_sendstr(req, resposta);

   if (ret == 0)
      tempo_estrobo_status = 250; // Sinaliza o LED VD

#ifdef DEBUG_AP_SERVER
   ESP_LOGI(LOG_SERVER, "POST - ret =%d", ret);
#endif

   return ESP_OK;
}
void register_routes()
{

   // Regstra as rotas do metodo GET.
   for (uint32_t i = 0; i < ARRAY_SIZE_OF(content_list_get_files); i++)
   {
      httpd_uri_t http_uri_get_static = {
          .uri = NULL,
          .method = HTTP_GET,
          .handler = http_handle_get,
          .user_ctx = NULL};

      http_uri_get_static.uri = content_list_get_files[i].path;

#ifdef DEBUG_SERVER
      ESP_LOGW(LOG_SERVER, "%s", http_uri_get_static.uri);
#endif
      httpd_register_uri_handler(server, &http_uri_get_static);
   }

   // Registra as rotas do metodo POST
   for (uint32_t i = 0; i < ARRAY_SIZE_OF(content_list_post_files); i++)
   {
      httpd_uri_t http_uri_post_static = {
          .uri = NULL,
          .method = HTTP_POST,
          .handler = http_handle_post,
          .user_ctx = NULL};

      http_uri_post_static.uri = content_list_post_files[i].path;
#ifdef DEBUG_SERVER
      ESP_LOGW(LOG_SERVER, "%s", http_uri_post_static.uri);
#endif
      httpd_register_uri_handler(server, &http_uri_post_static);
   }
}

// Função de inicio da aplicaçõo de servidor
void start_webserver(void)
{
   // Configuração padrão
   httpd_config_t config = HTTPD_DEFAULT_CONFIG();
   config.max_uri_handlers = 64;
   config.recv_wait_timeout = 15;
   config.send_wait_timeout = 15;
   // No Esp32 tem suporte e nao precisa declarar as rotas,
   // apenas a rota inicial / * e usar a linha abaixo
   // config.uri_match_fn = httpd_uri_match_wildcard;

   esp_err_t err;
   err = httpd_start(&server, &config);

   if (err == ESP_OK)
   {
      register_routes();
      ESP_ERROR_CHECK(httpd_register_uri_handler(server, &http_uri_options)); // Este para resposta às requisições Cors
   }
}
