
function update_saidas(key) {
	var saidas = key.split('-');
	console.log('saidas ' + saidas);
	var s1 = document.getElementById('s1');
	var s2 = document.getElementById('s2');
	var s3 = document.getElementById('s3');
	var s4 = document.getElementById('s4');

	if (saidas[0] == 1) {
		s1.classList.remove("primary");
		s1.classList.add("secondary");
		s1.innerHTML = "Desligar";
	} else {
		s1.classList.remove("secondary");
		s1.classList.add("primary");
		s1.innerHTML = "Ligar";
	}

	if (saidas[1] == 1) {
		s2.classList.remove("primary");
		s2.classList.add("secondary");
		s2.innerHTML = "Desligar";
	} else {
		s2.classList.remove("secondary");
		s2.classList.add("primary");
		s2.innerHTML = "Ligar";
	}

	if (saidas[2] == 1) {
		s3.classList.remove("primary");
		s3.classList.add("secondary");
		s3.innerHTML = "Desligar";
	} else {
		s3.classList.remove("secondary");
		s3.classList.add("primary");
		s3.innerHTML = "Ligar";
	}

	if (saidas[3] == 1) {
		s4.classList.remove("primary");
		s4.classList.add("secondary");
		s4.innerHTML = "Desligar";
	} else {
		s4.classList.remove("secondary");
		s4.classList.add("primary");
		s4.innerHTML = "Ligar";
	}
}

function atualizaFormulario(resposta) {
	if (resposta == null) {
		alert("Erro ao buscar wifi do servidor!");
		return;
	}

	// Referencia o container
	var container = document.getElementById('dataForm');

	while (container.lastElementChild) {
		container.removeChild(container.lastElementChild);
	}

	// Constroi a tela de atualização
	for (key in resposta) {
		if (key == 'saidas') {
			update_saidas(resposta[key])
		}
		
		// conteudo
		var div = document.createElement('div');
		div.className = "row inline";

		// key
		var lkey = document.createElement('label');
		lkey.innerHTML = key + ':';
		lkey.className = "block text-center col-sm-3 bold";
		
		div.appendChild(lkey);
		
		// value
		var lval = document.createElement('input');
		lval.setAttribute('type', 'text');
		lval.setAttribute('placeholder', resposta[key]);
		lval.className = "block text-center col-sm-9";
		lval.value = resposta[key];
		lval.disabled = "disabled";
		var lval2 = lval.cloneNode(true);
		var lconv = lkey.cloneNode(true);
		var append = 0;
		for (var x = 1; x <= 4; x++) {
			if (key == ('ad' + x)) {
				var constante = document.getElementById(('const' + x)).value;
				var v = Number(resposta[key]) * Number(constante);
				lval.className = "block text-center col-sm-3";
				lval2.className = "block text-center col-sm-4";
				lval2.setAttribute('placeholder', v );
				lval2.value = v;
				lval2.disabled = "disabled";
				lconv.innerHTML = "valor" + x;
				lconv.className = "block text-center col-sm-2 bold";
				append = 1;
			}
		}
		
		div.appendChild(lval);
		if (append) {
			div.appendChild(lconv);
			div.appendChild(lval2);
		}
		
		container.appendChild(div);
	}
}

var reqjson = () => {
	fetch('jsonrealtime.json')
		.then(response => response.json())
		.then(data => {
			atualizaFormulario(data);
		});
}
setInterval(reqjson, 1000);
reqjson();


function sendOutputs(id) {
	const req = {};

	const s1 = document.getElementById('s1').innerHTML;
	const s2 = document.getElementById('s2').innerHTML;
	const s3 = document.getElementById('s3').innerHTML;
	const s4 = document.getElementById('s4').innerHTML;

	if ((s1 == 'Ligar') && (id == 1)) req.s1 = 1;
	if ((s2 == 'Ligar') && (id == 2)) req.s2 = 1;
	if ((s3 == 'Ligar') && (id == 3)) req.s3 = 1;
	if ((s4 == 'Ligar') && (id == 4)) req.s4 = 1;
	if ((s1 == 'Desligar') && (id == 1)) req.s1 = 0;
	if ((s2 == 'Desligar') && (id == 2)) req.s2 = 0;
	if ((s3 == 'Desligar') && (id == 3)) req.s3 = 0;
	if ((s4 == 'Desligar') && (id == 4)) req.s4 = 0;

	fetch('updateoutputs.json', {
		headers: {
			'Content-type': 'application/json'
		},
		method: 'POST',
		body: JSON.stringify(req)
	}).then(response => response.json())
		.then(data => {
			console.log(data)
			if ('saidas' in data) {
				update_saidas(data.saidas);
			}
		}).catch(function (error) {
			console.log("Erro");
		});
}