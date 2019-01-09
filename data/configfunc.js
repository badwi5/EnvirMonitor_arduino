$(document).ready(function () {
	'use strict';
	//导航栏选中效果
	var spans = $('span');
	for (var i = 0; i < spans.length; i++) {
		spans[i].index = i;
		spans[i].onclick = function () {
			for (var i = 0; i < spans.length; i++) {
				spans[i].style.background = '';
				spans[i].style.color = '';
			}
			spans[this.index].style.backgroundColor = 'whitesmoke';
			spans[this.index].style.color = 'cadetblue';
		};
	}
	spans[0].click();

	var oldPassword = 'INIT_VALUE';
	var ipcheckbox = $('#inpEnableStaticIP');
	//初始化加载数据
	$.post('load',
		function (json) {
			oldPassword = json.psw;
			if ((json.netCfg.wifi.sIP.ip !== '0.0.0.0') ||
				(json.netCfg.wifi.sIP.gtw !== '0.0.0.0') ||
				(json.netCfg.wifi.sIP.sbn !== '0.0.0.0') ||
				(json.netCfg.wifi.sIP.dns !== '0.0.0.0')
			) {
				ipcheckbox.prop('checked', true);
			}
			$('#inpChipId').prop('value', json.devInf.chipId);
			$('#inpWebIp').prop('value', json.devInf.webIp);
			$('#inpHwVer').prop('value', json.devInf.hwVer);
			$('#inpSwVer').prop('value', json.devInf.swVer);
			$('#inpApId').prop('value', json.devInf.apid);
			$('#inpWifiName').prop('value', json.netCfg.wifi.ssid);
			$('#inpWifiPsw').prop('value', json.netCfg.wifi.psw);
			$('#inpStaticIp').prop('value', json.netCfg.wifi.sIP.ip);
			$('#inpStaticGtw').prop('value', json.netCfg.wifi.sIP.gtw);
			$('#inpStaticSbn').prop('value', json.netCfg.wifi.sIP.sbn);
			$('#inpStaticDns').prop('value', json.netCfg.wifi.sIP.dns);
			$('#inpServIp').prop('value', json.netCfg.serv.ip);
			$('#inpServPort').prop('value', json.netCfg.serv.port.toString());
			$('#inpServProdId').prop('value', json.netCfg.serv.prodID);
			$('#inpServDevId').prop('value', json.netCfg.serv.devID);
			$('#inpServAuth').prop('value', json.netCfg.serv.auth);
			$('#inpServTopic').prop('value', json.netCfg.serv.topic);
			$('#inpLoopTm').prop('value', json.args.loopTm.toString());
			$('#inpPostTm').prop('value', json.args.postTm.toString());
			$('#inpSda').prop('value', json.args.sda.toString());
			$('#inpScl').prop('value', json.args.scl.toString());
			$('#inpDht').prop('value', json.args.dht.toString());
			$('#inpBtn').prop('value', json.args.btn.toString());
			$('#inpLed').prop('value', json.args.led.toString());
		}, 'json');

	//接入点配置静态IP勾选效果
	var ipsettings = $('.staticipsettings');
	var ischecked = ipcheckbox.prop('checked');
	ipcheckbox.click(function () {
		ischecked = ipcheckbox.prop('checked');
		if (ischecked) {
			ipsettings[0].style.display = 'table-row';
			ipsettings[1].style.display = 'table-row';
			ipsettings[2].style.display = 'table-row';
			ipsettings[3].style.display = 'table-row';
		} else {
			ipsettings[0].style.display = 'none';
			ipsettings[1].style.display = 'none';
			ipsettings[2].style.display = 'none';
			ipsettings[3].style.display = 'none';
		}
	});

	$('.btnsubmit').click(function () {
		var id = $(this).attr('id');
		if (id === 'btnPswConfig') {
			var oldInp = $('#inpOldPsw');
			var newInp = $('#inpNewPsw');
			var confInp = $('#inpConfirmPsw');
			if (oldInp.val() !== oldPassword) {
				oldInp.prop('value', '');
				oldInp.focus();
				alert('密码错误！');
			} else if (newInp.val() !== confInp.val()) {
				newInp.prop('value', '');
				confInp.prop('value', '');
				newInp.focus();
				alert('新密码两次输入不一致！');
			} else {
				$.post('pswConfig', {
					psw: newInp.val()
				}, function (data) {
					if ('success' === data) {
						oldPassword = newInp.val();
						alert('设置成功');
					} else {
						alert('设置失败！Err:' + data);
					}
				});
			}
		} else if (id === 'btnHotSpot') {
			if ($('#inpWifiName').val() === '' || $('#inpWifiPsw').val() === '') {
				alert("请输入完整参数");
				return;
			}
			if (ischecked) {
				if ($('#inpStaticIp').val() === '' ||
					$('#inpStaticGtw').val() === '' ||
					$('#inpStaticSbn').val() === '' ||
					$('#inpStaticDns').val() === '') {
					alert("请输入完整参数");
					return;
				} else if ($('#inpStaticIp').css('color') === 'rgb(255, 0, 0)' ||
					$('#inpStaticGtw').css('color') === 'rgb(255, 0, 0)' ||
					$('#inpStaticSbn').css('color') === 'rgb(255, 0, 0)' ||
					$('#inpStaticDns').css('color') === 'rgb(255, 0, 0)') {
					alert("请检查输入参数");
					return;
				}
				
				$.post('hotSpot', {
					ssid: $('#inpWifiName').val(),
					psw: $('#inpWifiPsw').val(),
					sIp: $('#inpStaticIp').val(),
					sGtw: $('#inpStaticGtw').val(),
					sSbn: $('#inpStaticSbn').val(),
					sDns: $('#inpStaticDns').val()
				}, function (data) {
					if ('success' === data) {
						alert('设置成功');
					} else {
						alert('设置失败！Err:' + data);
					}
				});
			} else {
				$.post('hotSpot', {
					ssid: $('#inpWifiName').val(),
					psw: $('#inpWifiPsw').val(),
					sIp: '0.0.0.0',
					sGtw: '0.0.0.0',
					sSbn: '0.0.0.0',
					sDns: '0.0.0.0'
				}, function (data) {
					if ('success' === data) {
						alert('设置成功');
					} else {
						alert('设置失败！Err:' + data);
					}
				});
			}
		} else if (id === 'btnWebConfig') {
			$.post('webConfig', {
				ip: $('#inpServIp').val(),
				port: $('#inpServPort').val(),
				prodId: $('#inpServProdId').val(),
				devId: $('#inpServDevId').val(),
				auth: $('#inpServAuth').val(),
				topic: $('#inpServTopic').val(),
			}, function (data) {
				if ('success' === data) {
					alert('设置成功');
				} else {
					alert('设置失败！Err:' + data);
				}
			});
		} else if (id === 'btnArgsConfig') {
			$.post('argsConfig', {
				tm: $('#inpPostTm').val()
			}, function (data) {
				if ('success' === data) {
					alert('设置成功');
				} else {
					alert('设置失败！Err:' + data);
				}
			});
		}
	});

	$('.inppsw').bind('input propertychange', function () {
		var value = $(this).val();
		if (value.length > 15) {
			$(this).val(value.slice(0, 15));
		}
	});

	$('.inpipaddress').bind('input propertychange', function () {
		var value = $(this).val();
		var len = value.length;
		value = value.replace(/[^\-?\d.]/g, '');
		if (len > 15) {
			value = value.slice(0, 15);
		}
		$(this).val(value);
		if (!value.match(/^((25[0-5]|2[0-4]\d|[01]?\d\d?)($|(?!\.$)\.)){4}$/)) {
			$(this).css('color', '#ff0000');
		} else {
			$(this).css('color', 'inherit');
		}
	});
});
