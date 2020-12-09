// Requisição dos wifi
var request = new XMLHttpRequest();
request.open('GET', 'jsonconfigmod.json');
request.responseType = 'json';
request.send();

// Resposta do servidor
request.onload = function () {
   atualizaFormulario(request.response);
}

function atualizaFormulario(resposta) {
   if (resposta == null) {
      alert("Erro ao buscar wifi do servidor!");
      return;
   }
   console.log(resposta);

   var p_mac = document.getElementById('mac');
   p_mac.innerHTML = resposta.mac;

   var p_vsw = document.getElementById('vsw');
   p_vsw.innerHTML = resposta.vsw.toString(10);

   // input nome
   var input_nome = document.getElementById('nome');
   input_nome.setAttribute('placeholder', resposta.nome);
   input_nome.value = resposta.nome;
   input_nome.setAttribute('maxlength', '32');

   // Libera o botao de envio
   var bt = document.getElementById('formbtsubmit');
   if (bt.hasAttribute('disabled')) {
      bt.removeAttribute('disabled');
   }
}
function enviarFormulario() {
   var passconfig = document.getElementById('senhaconfig');
   var newpassconfig = document.getElementById('pass1');
   var nome = document.getElementById('nome');

   if (passconfig.value < 4) {
      alert("Senha inválida");
      return;
   }

   var form = {
      passconfig: passconfig.value,
      newpassconfig: newpassconfig.value,
      nome: nome.value,
   };

   request.open('POST', 'jsonconfigmod.json');
   request.responseType = 'json';
   request.send(JSON.stringify(form));
   request.onload = function () {
      var resposta = request.response;
      if (resposta == null) {
         alert("Erro com servidor!");
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