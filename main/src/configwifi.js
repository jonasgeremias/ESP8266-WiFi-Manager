var quantidade_login_wifi = 0;
// Requisição dos wifi
var request = new XMLHttpRequest();
request.open('GET', 'jsonconfigwifi.json');
request.responseType = 'json';
request.send();
// Resposta do servidor
request.onload = function () {
   atualizaFormulario(request.response);
}

function validateIPaddress(value) {
   var ipformat = /^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/;
   if (value == null) return false;
   if (value.match(ipformat)) return true; else return false;
}
function validateIPNumber(charcode) { return (charcode == 46 || (charcode >= 48 && charcode <= 57)) }

function blockIPfix(checked, id) {
   var ip4_ip = document.getElementById('ip4_ip_' + id);
   var ip4_gw = document.getElementById('ip4_gw_' + id);
   var ip4_netmask = document.getElementById('ip4_netmask_' + id);
   var ip4_dns1 = document.getElementById('ip4_dns1_' + id);
   var ip4_dns2 = document.getElementById('ip4_dns2_' + id);

   if (!checked) {
      ip4_ip.disabled = true;
      ip4_gw.disabled = true;
      ip4_netmask.disabled = true;
      ip4_dns1.disabled = true;
      ip4_dns2.disabled = true;
   } else {
      ip4_ip.disabled = false;
      ip4_gw.disabled = false;
      ip4_netmask.disabled = false;
      ip4_dns1.disabled = false;
      ip4_dns2.disabled = false;
   }
}

// Carrega valores iniciais na tela ---------------------------------
function atualizaFormulario(resposta) {
   if (resposta == null) {
      alert("Erro ao buscar wifi do servidor!");
      return;
   }
   // Referencia o container
   var container = document.getElementById('dataForm');
   quantidade_login_wifi = 0;

   // Campos temporários para copiar ja com as classes
   var input_ip_copy = document.createElement('input');
   input_ip_copy.type = 'text';
   input_ip_copy.maxlength = '15';
   input_ip_copy.maxlength = '7';
   input_ip_copy.size = '15';
   input_ip_copy.placeholder = 'xxx.xxx.xxx.xxx';
   input_ip_copy.setAttribute('class', 'row fill s-m');
   input_ip_copy.setAttribute('onblur', '{var ret=validateIPaddress(this.value); if(!ret)this.setCustomValidity("Invalid field.");else this.setCustomValidity(""); return ret; }');
   input_ip_copy.setAttribute('onkeypress', 'return validateIPNumber(event.charCode)');
   input_ip_copy.disabled = false;
   
   var label_ip_copy = document.createElement('label');
   label_ip_copy.setAttribute('class', 'row fill s-m');

   // loop que cria os campos
   for (var index in resposta) {
      quantidade_login_wifi++;
      
      // Label rede
      var label_rede = document.createElement('label');
      label_rede.innerHTML = 'Rede:';
      label_rede.className = 'col-sm-12';
      
      // Cria grupo de input para ssid e pesquisa
      var div_input_rede_find = document.createElement('div');
      div_input_rede_find.setAttribute('class', 'row block');
      
      // input rede
      var input_rede = document.createElement('input');
      input_rede.setAttribute('type', 'text');
      input_rede.setAttribute('maxlength', '32');
      input_rede.setAttribute('id', 'ssid' + index.toString(10));
      input_rede.setAttribute('class', 'col-sm-9 s-m');
      input_rede.setAttribute('placeholder', 'Rede');
      input_rede.value = resposta[index].ssid;
      
      // button find
      var button_find = document.createElement('button');
      button_find.type = 'button';
      button_find.setAttribute('id', 'btn_find' + index.toString(10));
      button_find.setAttribute('class', 'button primary col-sm-3 s-m s-p btn_find_class');
      button_find.setAttribute('onclick', 'procurarRede(' + index.toString(10) + ')');
      button_find.innerHTML = "Procurar";
      
      div_input_rede_find.appendChild(input_rede);
      div_input_rede_find.appendChild(button_find);
      
      // Div find
      var divfind = document.createElement('div');
      divfind.setAttribute('id', 'divfind' + index.toString(10));
      divfind.setAttribute('class', 'row col-sm-12');
      
      // Label senha
      var label_senha = document.createElement('label');
      label_senha.setAttribute('class', 'col-sm-12');
      label_senha.innerHTML = 'Senha:';
      
      // input senha
      var input_senha = document.createElement('input');
      input_senha.type = 'password';
      input_senha.maxlength = '64';
      input_senha.id = 'pass' + index.toString(10);
      input_senha.setAttribute('class', 'col-sm-10 s-m');
      if (resposta[index].pass == true) input_senha.setAttribute('placeholder', '********');
      else input_senha.setAttribute('placeholder', 'Insira a senha');
      input_senha.value = "";
      
      // Button show/hide
      var btn_show_hide = document.createElement('button');
      btn_show_hide.type = 'button';
      btn_show_hide.id = 'btn_show_hide' + index.toString(10);
      btn_show_hide.setAttribute('class', 'button primary col-sm-2 s-m s-p');
      btn_show_hide.setAttribute('onclick', 'exibirSenha(' + index.toString(10) + ')');
      btn_show_hide.innerHTML = "Exibir";
      
      var div_input_pass_group = document.createElement('div');
      div_input_pass_group.setAttribute('class', 'row col-sm-12 block');
      div_input_pass_group.appendChild(input_senha);
      div_input_pass_group.appendChild(btn_show_hide);
      
      // Adicionar os campos de IP fix
      // Add checkbox --------------------------------
      // div => input and label
      var div_checkbox_ipfix = document.createElement('div');
      div_checkbox_ipfix.setAttribute('class', 'row fill');
      
      var checkbox_ip4fix = document.createElement('input');
      checkbox_ip4fix.id = 'checkbox_ip4fix' + index.toString(10);
      checkbox_ip4fix.type = 'checkbox';
      checkbox_ip4fix.setAttribute('onclick', 'blockIPfix(this.checked,' + index + ')');
      checkbox_ip4fix.checked = resposta[index].ip4_fix;
      
      var label_ip4fix = document.createElement('label');
      label_ip4fix.id = 'label_ip4fix' + index.toString(10);
      label_ip4fix.innerHTML = "IP Fixo";
      label_ip4fix.setAttribute('class', 'check-send s-m s-p');
      label_ip4fix.setAttribute('for', 'checkbox_ip4fix' + index.toString(10))
      
      div_checkbox_ipfix.appendChild(checkbox_ip4fix);
      div_checkbox_ipfix.appendChild(label_ip4fix);
      
      var ip4_ip = input_ip_copy.cloneNode(true);
      ip4_ip.id = 'ip4_ip_' + index.toString(10);
      
      var ip4_gw = input_ip_copy.cloneNode(true);
      ip4_gw.id = 'ip4_gw_' + index.toString(10);
      
      var ip4_netmask = input_ip_copy.cloneNode(true);
      ip4_netmask.id = 'ip4_netmask_' + index.toString(10);
      
      var ip4_dns1 = input_ip_copy.cloneNode(true);
      ip4_dns1.id = 'ip4_dns1_' + index.toString(10);
      
      var ip4_dns2 = input_ip_copy.cloneNode(true);
      ip4_dns2.id = 'ip4_dns2_' + index.toString(10);
      
      // Testa se os IPs são validos e atribui
      if (validateIPaddress(resposta[index].ip4_ip)) ip4_ip.value = resposta[index].ip4_ip;
      if (validateIPaddress(resposta[index].ip4_gw)) ip4_gw.value = resposta[index].ip4_gw;
      if (validateIPaddress(resposta[index].ip4_netmask)) ip4_netmask.value = resposta[index].ip4_netmask;
      if (validateIPaddress(resposta[index].ip4_dns1)) ip4_dns1.value = resposta[index].ip4_dns1;
      if (validateIPaddress(resposta[index].ip4_dns2)) ip4_dns2.value = resposta[index].ip4_dns2;
      
      if (resposta[index].ip4_fix == false) {
         ip4_ip.disabled = true;
         ip4_gw.disabled = true;
         ip4_netmask.disabled = true;
         ip4_dns1.disabled = true;
         ip4_dns2.disabled = true;
      }
      
      var label_ip4_ip = label_ip_copy.cloneNode(true);
      label_ip4_ip.innerHTML = 'IP:';
      var label_ip4_gw = label_ip_copy.cloneNode(true);
      label_ip4_gw.innerHTML = 'Gateway:';
      var label_ip4_netmask = label_ip_copy.cloneNode(true);
      label_ip4_netmask.innerHTML = 'Netmask:';
      var label_ip4_dns1 = label_ip_copy.cloneNode(true);
      label_ip4_dns1.innerHTML = 'DNS1:';
      var label_ip4_dns2 = label_ip_copy.cloneNode(true);
      label_ip4_dns2.innerHTML = 'DNS2:';
      
      // Titulo Card - Ok
      var div_header = document.createElement('div');
      div_header.className = "section dark";
      div_header.id = 'div_header' + index.toString(10);
      var numerowifi = parseInt(index) + 1;
      div_header.innerText = 'Rede Wi-Fi ' + numerowifi.toString(10);
      
      // Verifica se está conectado na rede
      if (resposta[index].con == true) {
         div_header.innerHTML += " (Conectado)".bold();
      }
      
      // Titulo Conteudo
      var div_body = document.createElement('div');
      div_body.className = 'section row';
      div_body.setAttribute('id', 'div_rede' + index.toString(10));
      div_body.appendChild(label_rede);
      div_body.appendChild(div_input_rede_find);
      div_body.appendChild(divfind);
      div_body.appendChild(label_senha);
      
      // IP fixo
      div_body.appendChild(div_input_pass_group);
      div_body.appendChild(div_checkbox_ipfix);
      div_body.appendChild(label_ip4_ip);
      div_body.appendChild(ip4_ip);
      div_body.appendChild(label_ip4_gw);
      div_body.appendChild(ip4_gw);
      div_body.appendChild(label_ip4_netmask);
      div_body.appendChild(ip4_netmask);
      div_body.appendChild(label_ip4_dns1);
      div_body.appendChild(ip4_dns1);
      div_body.appendChild(label_ip4_dns2);
      div_body.appendChild(ip4_dns2);
      
      // Cria o card
      var div_card = document.createElement('div');
      div_card.className = "card fluid s-p s-m";
      div_card.setAttribute('id', 'div_card' + index.toString(10));
      div_card.appendChild(div_header);
      div_card.appendChild(div_body);
      
      // Separador
      var espaco = document.createElement('br');
      // Adiciona o card no container
      container.appendChild(div_card);
      container.appendChild(espaco);
   }
   var bt = document.getElementById('formbtsubmit');
   if (bt.hasAttribute('disabled')) bt.removeAttribute('disabled');
}

// Procura de rede ---------------------------------

function procurarRede(id) {
   // Deletar pesquisas abertas
   var list = document.getElementsByClassName('view_search');
   for (var item = 0; item < list.length; item++) {
      while (list[item].childElementCount > 0) {
         list[item].removeChild(list[item].firstChild);
      }
   }
   // Habilita novamente pesquisa
   var list = document.getElementsByClassName('btn_find_class');
   let len = list.length;
   for (var item = 0; item < len; item++) habilitaBotaoPesquisa(item, 0);

   // Muda botão para Cancelar
   var btn_find = document.getElementById('btn_find' + id);
   btn_find.removeAttribute('class');
   btn_find.setAttribute('class', 'button secondary col-sm-3 s-m s-p btn_find_class');
   btn_find.innerText = "Cancelar";
   btn_find.setAttribute('onclick', 'deletarPesquisa(' + id + ')');
   btn_find.removeAttribute('disabled', 'disabled');

   // referencia a div container
   var divfind = document.getElementById('divfind' + id);
   // texto enquanto não recebe json
   var label_msg = document.getElementById('label_msg' + id);
   if (label_msg === null) {
      label_msg = document.createElement('label');
      label_msg.type = 'text';
      label_msg.id = 'label_msg' + id;
      label_msg.className = 'block s-m text-center';
      label_msg.innerHTML = '<div class="spinner primary"></div>';
      divfind.appendChild(label_msg);
   }

   // Solicita pesquisa Wi-Fi
   request.open('GET', 'scanwifi.json');
   request.responseType = 'json';
   request.send();
   request.onload = function () {
      var res = request.response;
      if (res == null) {
         label_msg.innerHTML = ('Erro com Servidor!').bold();
      }
      // Remove o label temporario - (Procurando)
      while (divfind.childElementCount > 0) {
         divfind.removeChild(divfind.firstChild);
      }
      divfind.className = 'row col-sm-12';

      // Resposta do Servidor
      for (var index in res) {
         var div = document.createElement('div');
         div.setAttribute('class', 'row col-sm-12 list overflow-h');
         div.style.height = '30';
         div.setAttribute('onclick', 'atribuiRedeaoID(' + index + ',' + id + ')');

         // SSID
         var rede_find = document.createElement('p');
         rede_find.setAttribute('class', 'row col-sm-7 s-m overflow-h');
         rede_find.setAttribute('id', 'rede_find' + index);
         rede_find.innerHTML = res[index].ssid;
         rede_find.value = res[index].ssid;
         rede_find.style.paddingLeft = '8px';

         // RSSI
         var div_status_wifi = document.createElement('div');
         div_status_wifi.className = 'row col-sm-5 s-m';

         // AUTH
         var text_auth = document.createElement('div');
         text_auth.setAttribute('class', 'col-sm-7 s-m overflow-h');
         text_auth.style.padding = 'auto';

         var txt = "";
         if (res[index].authmode == 0) txt = "Open";
         else if (res[index].authmode == 1) txt = "WEP";
         else if (res[index].authmode == 2) txt = "WPA_PSK";
         else if (res[index].authmode == 3) txt = "WPA2_PSK";
         else if (res[index].authmode == 4) txt = "WPA_WPA2_PSK";
         else if (res[index].authmode == 5) txt = "WPA2_ENTERPRISE";
         else if (res[index].authmode == 6) txt = "WPA3_PSK";
         else txt = "Unknown";
         text_auth.innerHTML = "<mark style='font-size:xx-small'>" + txt + "</mark>";


         var div_img = document.createElement('div');
         div_img.setAttribute('class', 'col-sm-5 rssi s-m');

         var img = document.createElement('img');
         if (res[index].rssi < -60) img.src = "iconwifi_sm.png";
         else if (res[index].rssi < -40) img.src = "iconwifi_md.png";
         else img.src = "iconwifi_lg.png";
         img.width = "30";
         div_img.appendChild(img);

         div.appendChild(rede_find);
         div_status_wifi.appendChild(text_auth);
         div_status_wifi.appendChild(div_img);
         div.appendChild(div_status_wifi);
         divfind.appendChild(div);
      }
   }
}

function habilitaBotaoPesquisa(id, clickenable) {
   var bt = document.getElementById('btn_find' + id.toString(10));
   if (!clickenable) bt.setAttribute('disabled', 'disabled');
   else {
      bt.removeAttribute('disabled');
      bt.setAttribute("class", "btn_find_class button primary col-sm-3 s-m s-p");
      bt.setAttribute('onclick', 'procurarRede(' + id.toString(10) + ')');
   }
   bt.innerHTML = "Procurar";
}

function hab_all_btn_find() {
   // Habilita novamente pesquisa
   var list = document.getElementsByClassName('btn_find_class');
   let len = list.length;
   for (var item = 0; item < len; item++) habilitaBotaoPesquisa(item, 1);
}

function deletarPesquisa(id) {
   var divfind = document.getElementById('divfind' + id.toString(10));
   while (divfind.childElementCount > 0) {
      divfind.removeChild(divfind.firstChild);
   }
   request.abort();
   hab_all_btn_find();
}

function atribuiRedeaoID(id_pesquisa, id) {
   var ssid = document.getElementById('ssid' + id);
   var rede_find = document.getElementById('rede_find' + id_pesquisa);
   ssid.value = rede_find.value;
   var divfind = document.getElementById('divfind' + id);
   while (divfind.childElementCount > 0) {
      divfind.removeChild(divfind.firstChild);
   }
   hab_all_btn_find();
}

function enviarFormulario() {
   // Se erro recarrega página
   if (quantidade_login_wifi == 0) {
      alert("Erro ao buscar wifi do servidor!");
      document.location.reload(true);
      return;
   }
   var senha = document.getElementById("senhaconfig");
   if (senha.value.length != 4) {
      alert("Digite a senha de configuração");
      return;
   }

   var form = {
      passconfig: senha.value
   }

   for (var id = 0; id < quantidade_login_wifi; id++) {
      var ssid = document.getElementById('ssid' + id.toString()).value;
      var pass = document.getElementById('pass' + id.toString()).value;
      var ip4_fix = document.getElementById('checkbox_ip4fix' + id.toString()).checked;
      var ip4_ip = document.getElementById('ip4_ip_' + id.toString());
      var ip4_gw = document.getElementById('ip4_gw_' + id.toString());
      var ip4_netmask = document.getElementById('ip4_netmask_' + id.toString());
      var ip4_dns1 = document.getElementById('ip4_dns1_' + id.toString());
      var ip4_dns2 = document.getElementById('ip4_dns2_' + id.toString());
      if ((ssid.length != 0) && (pass.length < 4) && (pass.length > 0)) {
         alert("Wi-Fi " + (id + 1).toString() + " deve possuir uma senha maior que 3 dígitos");
         return;
      }
      
      form[id.toString()] = {
         ssid: ssid,
         pass: pass,
         ip4_fix : ip4_fix
      };

      // Testa se os IPs são validos e atribui
      if (ip4_fix) {
         var erro = 0;
         if (!validateIPaddress(ip4_ip.value)) { ip4_ip.setCustomValidity("Invalid field."); erro = 1;} else form[id.toString()].ip4_ip = ip4_ip.value;
         if (!validateIPaddress(ip4_gw.value)) { ip4_gw.setCustomValidity("Invalid field.");  erro = 1;} else form[id.toString()].ip4_gw = ip4_gw.value;
         if (!validateIPaddress(ip4_netmask.value)) { ip4_netmask.setCustomValidity("Invalid field."); erro = 1;} else form[id.toString()].ip4_netmask = ip4_netmask.value;
         if (!validateIPaddress(ip4_dns1.value)) { ip4_dns1.setCustomValidity("Invalid field."); erro = 1; }else form[id.toString()].ip4_dns1 = ip4_dns1.value;
         if (!validateIPaddress(ip4_dns2.value)) { ip4_dns2.setCustomValidity("Invalid field."); erro = 1; }else form[id.toString()].ip4_dns2 = ip4_dns2.value;
         if (erro) {
            alert("Wi-Fi " + (id + 1).toString() + " deve possuir configuração válida para ip fixo.");
            return;
         }
      }
      else {
         form[id.toString()].ip4_ip = "";
         form[id.toString()].ip4_gw = "";
         form[id.toString()].ip4_netmask = "";
         form[id.toString()].ip4_dns1 = "";
         form[id.toString()].ip4_dns2 = "";
      }
   }
   request.open('POST', 'jsonconfigwifi.json');
   request.responseType = 'json';
   request.send(JSON.stringify(form));
   request.onload = function () {
      var resposta = request.response;
      if (resposta == null) {
         alert("Erro com o servidor! Tente novamente!");
         return;
      }
      switch (resposta.result) {
         case 0: alert("Configurado! Reinicie o módulo para aplicar as alterações."); break;
         case -1: alert("Nova senha é inválida."); break;
         case -2: alert("Novo nome é inválido."); break;
         case -3: alert("Erro ao gravar, tente novamente."); break;
         case -4: alert("Senha inválida! insira a senha correta."); break;
         case -5: alert("Configuração de IP inválida!"); break;
         case -6: alert("Formato inválido."); break;
         default: alert("Erro! Retorno desconhecido."); break;
      }
   }
}