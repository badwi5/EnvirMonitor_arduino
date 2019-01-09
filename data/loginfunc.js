$(document).ready(function () {
	'use strict';
	$('#inpPassword').keydown(
		function () {
			if (event.keyCode === 13) {
				funcLogin();
			}
		});
	$('#btnLogin').click(
		function () {
			funcLogin();
		}
	);

});

function funcLogin() {
	'use strict';
	var inpPassword = $('#inpPassword');
	$.post('login', {
		psw: inpPassword.val()
	}, function (data) {
		if ('success' === data) {
			window.location.href = "/8266Config.html";
		} else {
			inpPassword.prop('value', '');
			inpPassword.focus();
			alert('登陆失败！Err:'+ data);
		}
	});
}
