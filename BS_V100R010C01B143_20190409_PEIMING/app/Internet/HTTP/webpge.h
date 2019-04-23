
/**
******************************************************************************
* @file   webpge.h
* @author  WIZnet Software Team 
* @version V1.0
* @date    2015-xx-xx
* @brief   web服务器现在的内容
* @attention  
******************************************************************************
**/
#ifndef __WEBPAGE_H
#define __WEBPAGE_H


#define CONFIG_HTML  "<!DOCTYPE html>"\
"<html>"\
"<head>"\
"<title>Gateway::IP配置</title>"\
"<meta http-equiv='Content-Type' content='text/html; charset=GB2312'/>"\
"<style type='text/css'>"\
"body {text-align:left; background-color:#c0deed;font-family:Verdana;}"\
"#main {margin-right:auto;margin-left:auto;margin-top:30px;}"\
"label{display:inline-block;width:150px;}"\
"#main h3{color:#66b3ff; text-decoration:underline;}"\
"</style>"\
"<script>"\
"function $(id) { return document.getElementById(id); };"\
"function settingsCallback(o) {"\
"if ($('txtVer')) $('txtVer').value = o.ver;"\
"if ($('txtMac')) $('txtMac').value = o.mac;"\
"if ($('txtIp')) $('txtIp').value = o.ip;"\
"if ($('txtSub')) $('txtSub').value = o.sub;"\
"if ($('txtGw')) $('txtGw').value = o.gw;"\
"if ($('txtsvr')) $('txtsvr').value = o.svr;"\
"if ($('txtport')) $('txtport').value = o.port;"\
"};"\
"</script>"\
"</head>"\
"<body>"\
"<div id='main'>"\
"<div style='background:snow; display:block;padding:10px 50px;'>"\
"<h3>配置本地IP地址</h3>"\
"<form id='frmSetting' method='POST' action='config.cgi'>"\
"<p><label for='txtVer'>固件版本号:</label><input type='text' id='txtVer' name='ver' size='25' disabled='disabled' /></p>"\
"<p><label for='txtMac'>网关MAC地址:</label><input type='text' id='txtMac' name='mac' size='25' disabled='disabled' /></p>"\
"<p><label for='txtIp'>本地IP地址:</label><input type='text' id='txtIp' name='ip' size='25' /></p>"\
"<p><label for='txtSub'>子网掩码:</label><input type='text' id='txtSub' name='sub' size='25' /></p>"\
"<p><label for='txtGw'>默认网关:</label><input type='text' id='txtGw' name='gw' size='25' /></p>"\
"<p><input type='submit' value='保存并重启' /></p>"\
"</form>"\
"</div>"\
"</div>"\
"<div style='margin:5px 5px;'>"\
"&copy; 2018 12"\
"</div>"\
"<script type='text/javascript' src='w5500.js'></script>"\
"</body>"\
"</html>"

#endif
