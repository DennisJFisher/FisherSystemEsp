const char SITE_index[] PROGMEM = R"=====(    
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="utf-8"/>
    <title id="PageTitle">
        junkage
    </title>
    <script>
        var connection = new WebSocket('ws://' + location.hostname + ':81/', ['arduino']);
        connection.onopen = function() {
            connection.send('Connect ' + new Date());
        }
        connection.onerror = function(error) {
            console.log('WebSocket Error ', error);
        }
        connection.onmessage = function(e) {
            var PageTitle = document.getElementById('PageTitle');
            var BodyTitle = document.getElementById('BodyTitle');
            var SDKVersion = document.getElementById('SDKVersion');
            var BootVersion = document.getElementById('BootVersion');
            var CPUFreq = document.getElementById('CPUFreq');
            var FreeHeap = document.getElementById('FreeHeap');
            var FirmwareVersion = document.getElementById('FirmwareVersion');
            var ResetReason = document.getElementById('ResetReason');
            var UpTime = document.getElementById('UpTime');
            var CurrentTimestamp = document.getElementById('CurrentTimestamp');
            var Ssid = document.getElementById('Ssid');
            var Rssi = document.getElementById('Rssi');
            var MqttBroker = document.getElementById('MqttBroker');
            var MqttDomainName = document.getElementById('MqttDomainName');
            var MqttBadConnects = document.getElementById('MqttBadConnects');
            var ProcessInterval = document.getElementById('ProcessInterval');
            var InfoTable = document.getElementById('InfoTable');
            var FunctionTable = document.getElementById('FunctionTable');
            console.log('Server: ', e.data);
            // Check for json data.
            if (e.data[0] === '{') {
                var msg = JSON.parse(e.data);
                if (msg.Title !== undefined) {PageTitle.innerHTML = msg.Title; BodyTitle.innerHTML = msg.Title};
                if (msg.SDKVersion !== undefined) SDKVersion.innerHTML = msg.SDKVersion;
                if (msg.BootVersion !== undefined) BootVersion.innerHTML = msg.BootVersion;
                if (msg.FlashSize !== undefined) FlashSize.innerHTML = msg.FlashSize;
                if (msg.CPUFreq !== undefined) CPUFreq.innerHTML = msg.CPUFreq;
                if (msg.FreeHeap !== undefined) FreeHeap.innerHTML = msg.FreeHeap;
                if (msg.FirmwareVersion !== undefined) FirmwareVersion.innerHTML = msg.FirmwareVersion;
                if (msg.ResetReason !== undefined) ResetReason.innerHTML = msg.ResetReason;
                if (msg.CurrentTimestamp !== undefined) CurrentTimestamp.innerHTML = msg.CurrentTimestamp;
                if (msg.Ssid !== undefined) Ssid.innerHTML = msg.Ssid;
                if (msg.Rssi !== undefined) Rssi.innerHTML = msg.Rssi;
                if (msg.MqttBroker !== undefined) MqttBroker.innerHTML = msg.MqttBroker;
                if (msg.MqttDomain !== undefined) MqttDomain.innerHTML = msg.MqttDomain;
                if (msg.MqttBadConnects !== undefined) MqttBadConnects.innerHTML = msg.MqttBadConnects;
                if (msg.DebugString !== undefined) DebugString.innerHTML = msg.DebugString;
                if (msg.ProcessInterval !== undefined) ProcessInterval.innerHTML = msg.ProcessInterval;

                if (msg.UpTimeMinutes !== undefined) {
                    var minutes = parseInt(msg.UpTimeMinutes);
                    var days = parseInt(minutes / 1440);
                    minutes %= 1440;
                    var hours = parseInt(minutes / 60);
                    minutes %= 60;
                    UpTime.innerHTML = "" + days + " Days, " + hours + " hours, " + minutes + " min";
                }
                if (msg.InfoTable !== undefined) {
                    // Clear out all rows except the header row.
                    while (InfoTable.rows.length > 0) {
                        InfoTable.deleteRow(InfoTable.rows.length - 1);
                    }
                    // comes in with 6 col per row.
                    var arr = msg.InfoTable.split(",");
                    var rows = arr.length / 6;
                    for (var row = 0; row < rows; row++) {
                        var RowElement = document.createElement('TR');
                        InfoTable.appendChild(RowElement);
                        for (var col = 0; col < 6; col++) {
                            var CellElement = document.createElement('TD');
                            RowElement.appendChild(CellElement);
                            // Load the cell after adjusting str below.
                            var str = arr[row * 6 + col];
                            switch(col)
                            {
                                case 0: // Turn into links.
                                    str='<a href="http://' + str + '.local" target="_blank">' + str + '</a>';
                                    break;
                                case 1:
                                    str='<a href="http://' + str  + '" target="_blank">' + str + "</a>";
                                    break;
                                case 4: // up time convert from min to dhm
                                    var minutes = parseInt(str);
                                    var days_s = String("  " + parseInt(minutes / 1440)).slice(-2);
                                    minutes %= 1440;
                                    var hours_s = String("  " + parseInt(minutes / 60)).slice(-2);
                                    minutes_s = String("  " + minutes % 60).slice(-2);
                                    str = days_s + 'd ' + hours_s + 'h ' + minutes_s + 'm';
                                    CellElement.style.textAlign="right";
                                    break;
                                default:
                                    break;
                            }                            
                            CellElement.innerHTML = str;
                        }
                    }
                }
                if (msg.FunctionTable !== undefined) {
                    // Clear out all rows except the header row.
                    while (FunctionTable.rows.length > 0) {
                        FunctionTable.deleteRow(FunctionTable.rows.length - 1);
                    }
                    // comes in with 2 col per row.
                    var arr = msg.FunctionTable.split(",");
                    var rows = arr.length / 2;
                    for (var row = 0; row < rows; row++) {
                        var RowElement = document.createElement('TR');
                        FunctionTable.appendChild(RowElement);
                        for (var col = 0; col < 2; col++) {
                            var CellElement = document.createElement('TD');
                            RowElement.appendChild(CellElement);
                            // Load the cell after adjusting str below.
                            CellElement.innerHTML = arr[row * 2 + col];
                        }
                    }
                }
            }
        }
        function sendKeyValue(checkbox) {
            connection.send(checkbox.name + ',' + checkbox.value);
        }
        function doSend(message, value) {
            connection.send(message + ',' + value);
        }
    </script>
</head>
<body style="background-color:lightgrey;">
    <h1 id="BodyTitle">...</h1>

    <style>
        h1 { margin:0; text-align:center;}
        td { vertical-align: middle; }
        
        .snazzy table {
            font-size: 14px;
            border: thin solid
        }
        
        .snazzy th,
        .snazzy td {
            padding: 5px;
            text-align: left;
        }
        
        .snazzy th {
            color: white;
            background-color: black;
        }

        .snazzy tr:nth-child(even) {
            color: black;
            background-color: #eee;
        }
        
        .snazzy tr:nth-child(odd) {
            color: black;
            background-color: #fff;
        }
    </style>
    <table class="frame">
        <tr>
            <td width="60%">
                ESP info:
                <ul>
                    <li>Debug string:
                        <span id="DebugString"></span>
                    </li>
                    <li>SDK Version:
                        <span id="SDKVersion"></span>, Boot Version:
                        <span id="BootVersion"></span>, Flash size
                        <span id="FlashSize"></span> bytes, CPU Freq
                        <span id="CPUFreq"></span> Mhz
                    </li>
                    <li>Firmware version:
                        <span id="FirmwareVersion"></span>
                    </li>
                    <li>Last reset info:
                        <span id="ResetReason"></span>
                    </li>
                    <li>Free heap:
                        <span id="FreeHeap"></span>
                    </li>
                    <li>Up time:
                        <span id="UpTime"></span>
                    </li>
                </ul>
            </td>
            <td>
                Network info:
                <ul>
                    <li>Current timestamp:
                        <span id="CurrentTimestamp"></span>
                    </li>
                    <li>SSID:
                        <span id="Ssid"></span>
                    </li>
                    <li>RSSI:
                        <span id="Rssi"></span>
                    </li>
                    <li>Broker:
                        <span id="MqttBroker"></span>, Domain name:
                        <span id="MqttDomain"></span>
                    </li>
                    <li>Bad MQTT connects:
                        <span id="MqttBadConnects"></span>
                    </li>
                </ul>
            </td>
        </tr>
        <tr>
            <td>
                <table class="snazzy">
                    <caption>Device Info Table</caption>
                    <thead>
                        <tr>
                            <th>
                                <u>Name</u>
                            </th>
                            <th>
                                <u>IP</u>
                            </th>
                            <th>
                                <u>RSSI</u>
                            </th>
                            <th>
                                <u>FW Version</u>
                            </th>
                            <th>
                                <u>Up time</u>
                            </th>
                            <th>
                                <u>Min ago</u>
                            </th>
                        </tr>
                    </thead>
                    <tbody id="InfoTable"></tbody>
                </table>
            </td>
            <td>
                Function info:
                <ul>
                    <li>Process interval:
                        <span id="ProcessInterval"></span> s
                    </li>
                </ul>
                <table class="snazzy">
                    <caption>Function table</caption>
                    <thead>
                        <tr>
                            <th>
                                <u>Measurement</u>
                            </th>
                            <th>
                                <u>Value</u>
                            </th>
                        </tr>
                    </thead>
                    <tbody id="FunctionTable"></tbody>
                </table>
            </td>
        </tr>
        <tr>
            <td>
                <form method='POST' action='/temp_set' enctype='multipart/form-data'>
                    <input type='number' id='CabinBasementTempSet' name='CabinBasementTempSet'>    <input type='submit' value='Update cabin basement temp'>        <br/>
                </form>
                <form method='POST' action='/temp_set' enctype='multipart/form-data'>
                    <input type='number' name='CabinGarageUpTempSet'>    <input type='submit' value='Update garage upstairs temp'>       <br/>
                </form>
                <form method='POST' action='/temp_set' enctype='multipart/form-data'>
                    <input type='number' name='CabinBasementPropTempSet'><input type='submit' value='Update cabin basement propane temp'><br/>
                </form>
                <form method='POST' action='/temp_set' enctype='multipart/form-data'>
                    <input type='number' name='CabinKitchenTempSet'>     <input type='submit' value='Update cabin kitchen temp'>         <br/>
                </form>
                <form method='POST' action='/temp_set' enctype='multipart/form-data'>
                                                                         <br/><input type='submit' value='Reboot'>
                </form>
                <form method='POST' action='/reboot' enctype='multipart/form-data'>
                    <input type='button'   name='rebootbutton' value='rebootbuttonvalue' onclick='doSend('CabinBasementTempSet', '5')'>reboot</input><br/>
                    <input type='checkbox' name='rebootCB' value='rebootCBValue' onclick='sendCheckbox(this)'>reboot</input><br/>
                </form>
            </td>
        </tr>
    </table>
</body>
)=====";
