var qrcode = new QRCode(document.getElementById("qrcode"), {
    width : 125,
    height : 125,
    useSVG: false
});

var currConfig = null;
var currUIDExtdRecs = null;
var bSteamOSConnected = false;
var bMisterConnected = false;
var bWriteMode = false;
var bUIDMode = false;

var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', doOnload);
window.addEventListener("beforeunload", doUnload);

function buildZapSocket(ip){
    return new WebSocket(`ws://${ip}:7497${currConfig.zap_ws_path}`);
}

function runSearch() {				
    var sGame = document.getElementById("searchGame").value;
    var sSys = this.getSelectedSystemsValue();
    var elwrDiv = document.getElementById("wrTagDiv");
    elwrDiv.style.display = "none";
    document.getElementById("searchResults").innerHTML = `<h4>Searching....</h4>`;
    if(sSys == "*"){sSys = ""};
    if(sGame){								
        var currIP = "";
        if(sSys == "steam"){
            currIP = document.getElementById("steamIP").value;
            sSys = ""
        }else{
            currIP = document.getElementById("misterIP").value;
        }
        let socket = buildZapSocket(currIP);

        socket.onopen = function(e) {
            console.log("[open] Connection established 1");
            let wscmd = {
                jsonrpc: "2.0",
                id: "47f80537-7a5d-11ef-9c7b-020304050607",
                method: "media.search",
                params: {
                    query: sGame,
                    maxResults: 250
                }	
            };
            if(sSys != ""){wscmd.params.systems = [sSys]};
            socket.send(JSON.stringify(wscmd));
        };
        socket.onmessage = function(event) {
            //console.log(`[message] Data received from server: ${event.data}`);
            var strGames = event.data;;
            var objGames = JSON.parse(strGames);
            //console.log("objGames", objGames.result)

            if(objGames.result.total > 0) {
            
                var resultsDiv = document.getElementById("searchResults");

                document.getElementById("searchResults").innerHTML = "";
                
                var resultsLabel = document.createElement("h3");
                if(objGames.result.total > 250) {
                    resultsLabel.innerText = "Search Results (" + objGames.result.total + ") - Max 250 shown - Please refine your search!";
                }
                else{
                    resultsLabel.innerText = "Search Results (" + objGames.result.total + ")";
                }
                resultsDiv.appendChild(resultsLabel);
                
                var resultsOptionDiv = document.createElement("div");
                resultsOptionDiv.className = "select-group";

                var resultsLabel = document.createElement("label");
                resultsLabel.innerText = "Select a Game to write to the Tag/Card";
                resultsLabel.className = "label";

                resultsOptionDiv.appendChild(resultsLabel);


                //resultsDiv.appendChild(resultsLabel);
                
                var resultsSelect = document.createElement("select");
                resultsSelect.className = "input labelText"
                resultsSelect.id = "resSel";
                resultsSelect.name = "resSel";
                resultsOptionDiv.appendChild

                resultsOptionDiv.appendChild(resultsSelect);
                resultsDiv.appendChild(resultsOptionDiv);

                document.getElementById("resSel").style = "min-width: 60vw"

                let topel = document.createElement("option");
                topel.textContent = "Choose Game";
                topel.value = "null";
                document.getElementById("resSel").appendChild(topel);

                document.getElementById("resSel").addEventListener("change", makeCode, false);
                
                var sysItem = 0;
                for(sysItem in objGames.result.results){
                    let optionTxt = objGames.result.results[sysItem].system.name + ": " + objGames.result.results[sysItem].name;
                    let optionVal = objGames.result.results[sysItem].path;
                    let el = document.createElement("option");
                    el.textContent = optionTxt;
                    el.value = optionVal;
                    document.getElementById("resSel").appendChild(el);
                }
            }
            else{
                var resultsDiv = document.getElementById("searchResults");
                var resultsLabel = document.createElement("h2");
                resultsLabel.innerText = "Search Results (0)";
                resultsDiv.appendChild(resultsLabel);
            }
            socket.close(1000, "Work complete");
            qrcode.clear();
            document.getElementById("qrcode").innerHTML = "";
            qrcode = new QRCode(document.getElementById("qrcode"), {
                width : 125,
                height : 125,
                useSVG: false
            });
            
        };
        socket.onclose = function(event) {
            if (event.wasClean) {
                console.log(`Search [close] Connection closed cleanly, code=${event.code} reason=${event.reason}`);
            } else {
                console.log('Search [close] Connection died');
            }
        };
        socket.onerror = function(error) {
            console.log(`[error]`);
        };
    }
    else{
        alert("Input Search Text");
    }
}

function getIndexedSystems() {				
    bSteamOSConnected = false;
    bMisterConnected = false;
    
    if(document.getElementById("selEnSteamMode").checked)	{
        var currSteamOSIP = document.getElementById("steamIP").value;
        let socket = buildZapSocket(currSteamOSIP);
        socket.onopen = function(e) {
            console.log("Connection established To SteamOS");
            addLogLine("Connection to SteamOS Established!")
            socket.close(1000, "Work complete");
            bSteamOSConnected = true;
            let select = document.getElementById("selSys");
            let el = document.createElement("option");
            el.textContent = "Steam";
            el.value = "steam";
            select.appendChild(el);
        };
        socket.onerror = function(error) {
            console.log(`error connecting to Steam OS`, error);
            addLogLine("Failed to Connect to SteamOS. Check SteamOS IP in settings.")
        };
    }

    if(document.getElementById("selEnMisterMode").checked)	{
        var currMistIP = document.getElementById("misterIP").value;
        let socket = buildZapSocket(currMistIP);
        socket.onopen = function(e) {
            console.log("Connection established to MiSTER");
            addLogLine("Connection to MiSTer Established!")
            let wscmd = {
                jsonrpc: "2.0",
                id: "dbd312f3-7a5f-11ef-8f29-020304050607",
                method: "systems"
            };
            bMisterConnected = true;
            socket.send(JSON.stringify(wscmd));
        };

        socket.onmessage = function(event) {
            //console.log(`[message] Data received from server: ${event.data}`);
            var strSystems = event.data;
            var objSystems = JSON.parse(strSystems);
            //console.log("objSystems", objSystems.result)
            let select = document.getElementById("selSys");
            var sysItem = 0;					
            objSystems.result.systems.sort(function (a, b) {
                if (a.name < b.name) {
                    return -1;
                }
                if (a.name > b.name) {
                    return 1;
                }
                return 0;
            });
            let el = document.createElement("option");
            el.textContent = "All MiSTer Systems";
            el.value = "*";
            select.appendChild(el);
            for(sysItem in objSystems.result.systems){
                let optionTxt = objSystems.result.systems[sysItem].name;
                let optionVal = objSystems.result.systems[sysItem].id;
                let el = document.createElement("option");
                el.textContent = optionTxt;
                el.value = optionVal;
                select.appendChild(el);
            }
            var input = document.getElementById("searchGame");
            input.addEventListener("keyup", function(event) {
                if (event.key === "Enter") {
                    event.preventDefault();
                    document.getElementById("btnSearch").click();
                }
            });
            socket.close(1000, "Work complete");
        };

        socket.onclose = function(event) {
            if (event.wasClean) {
                console.log(`Get Systems [close] Connection closed cleanly, code=${event.code} reason=${event.reason}`);
            } else {
                console.log('Get Systems [close] Connection died');
            }
        };
        socket.onerror = function(error) {
            console.log(`[error]`);
            addLogLine("Failed to Connect to MiSTer. Check MiSTer IP in settings.")
        };
    }
    
}

function mediaIndexSystems() {
    var sSys = this.getSelectedSystemsValue();
    var currIP = "";
    var strIndexingSystem = ""
    if(sSys == "steam"){
        currIP = document.getElementById("steamIP").value;
        strIndexingSystem = "SteamOS"
    }else{
        currIP = document.getElementById("misterIP").value;
        strIndexingSystem = "MiSTer"
    }
    let socket = buildZapSocket(currIP);
    socket.onopen = function(e) {
        console.log("Index [open] Connection established");
        let wscmd = {
            jsonrpc: "2.0",
            id: "6f20e07c-7a5e-11ef-84bb-020304050607",
            method: "media.index"
        };
        socket.send(JSON.stringify(wscmd));
    };
    socket.onmessage = function(event) {
        //console.log(`[message] Data received from server: ${event.data}`);
        var strIndex = event.data;
        var objIndex = JSON.parse(strIndex);
        console.log("objIndex", objIndex)
        if(objIndex.params.indexing) {
            document.getElementById("indexingDiv").innerHTML = `<h4>Indexing ${strIndexingSystem} Games - Total To Index: ${objIndex.params.totalSteps}  -  Currently Processing: ${objIndex.params.currentStep}</h4>`;
        }else{
            document.getElementById("indexingDiv").innerHTML = `<h4>Indexing Complete - Refreshing List....</h4>`;
            socket.close(1000, "Work complete");
            setTimeout(function() {
                document.getElementById("indexingDiv").innerHTML ="";
            }, 2000);
            this.getIndexedSystems();
        }
    };
    socket.onclose = function(event) {
        if (event.wasClean) {
            console.log(`Index [close] Connection closed cleanly, code=${event.code} reason=${event.reason}`);
        } else {
            console.log('Index [close] Connection died');
        }
    };
    socket.onerror = function(error) {
        console.log(`[error]`);
    };

}

//Get Index Systems and populate selection box
function getSysForSelect() {				
    var strSystems = this.getIndexedSystems();
    console.log("strSystems", strSystems)
    var objSystems = JSON.parse(strSystems);
    let select = document.getElementById("selSys");
    var sysItem = 0;
    for(sysItem in objSystems.systems){
        let optionTxt = objSystems.systems[sysItem].name;
        let optionVal = objSystems.systems[sysItem].id;
        let el = document.createElement("option");
        el.textContent = optionTxt;
        el.value = optionVal;
        select.appendChild(el);
    }
}

function getSelectedSystemsValue() {
    var select = document.getElementById("selSys");
    var selectedValue = select.value;
    return selectedValue;
}

function doOnload(event) {
    initWebSocket();
}

function doUnload(event){
    var tmpLCMD = `{"cmd": "closeWS", "data": "closeWS"}`;
    websocket.send(tmpLCMD);
    setTimeout(function() {
        websocket.close();
    }, 1000);
    setTimeout(function() {
        addLogLine("Zap ESP will now close WS");
    }, 1000);
}

function doUIDExtGetData(){
    websocket.send("{'cmd': 'getUIDExtdRec'}");
}

function saveUIDExtdRecData(){
    var tmpUID = document.getElementById('RFIDUID').value;
    var bDidUpdate = false;
    if(tmpUID.length > 0){
        for (var i = 0; i < currUIDExtdRecs.UID_ExtdRecs.length; i++) {
            if(currUIDExtdRecs.UID_ExtdRecs[i].UID == tmpUID){
                //record exists so update
                currUIDExtdRecs.UID_ExtdRecs[i].launchAudio = validateAudioPath(document.getElementById('UIDaudioTAGGameLaunchFilePath').value);
                currUIDExtdRecs.UID_ExtdRecs[i].removeAudio = validateAudioPath(document.getElementById('UIDaudioTAGRemovedFilePath').value);
                bDidUpdate = true;
            }
        }
        if(!bDidUpdate){
            //new record
            var tmpRecord = {};
            tmpRecord.UID = tmpUID;
            tmpRecord.launchAudio = validateAudioPath(document.getElementById('UIDaudioTAGGameLaunchFilePath').value);
            tmpRecord.removeAudio = validateAudioPath(document.getElementById('UIDaudioTAGRemovedFilePath').value);
            currUIDExtdRecs.UID_ExtdRecs.push(tmpRecord)
        }
        //console.log("Updated currUIDExtdRecs:", currUIDExtdRecs);
        var tmpLCMD = `{"cmd": "saveUIDExtdRec", "data": ${JSON.stringify(currUIDExtdRecs)}}`;
        websocket.send(tmpLCMD);
    }

}

function processUIDExtData(dataReceived){
    currUIDExtdRecs = dataReceived;
    addLogLine("UID Database retrieved");
    addLogLine("Scan an RFID tag/card/amibo to continue");
}

function doReceiveUID(UIDStr){
    document.getElementById('RFIDUID').value = UIDStr;
    var UID_Record = currUIDExtdRecs.UID_ExtdRecs.filter(item => item.UID == UIDStr);
    if(UID_Record.length > 0){
        document.getElementById('UIDaudioTAGGameLaunchFilePath').value = UID_Record[0].launchAudio;
        document.getElementById('UIDaudioTAGRemovedFilePath').value = UID_Record[0].removeAudio;
    }else {
        document.getElementById('UIDaudioTAGGameLaunchFilePath').value = "";
        document.getElementById('UIDaudioTAGRemovedFilePath').value = "";
    }
}

function addLogLine(message){
    var logger = document.getElementById('logDiv');
    if (typeof message == 'object') {
        logger.innerHTML += (JSON && JSON.stringify ? JSON.stringify(message) : message) + '<br />';
    } else {
        logger.innerHTML += message + '<br />';
    }
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection to ZAP ESP…');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    addLogLine('Connection to ZAP ESP opened');
    console.log('Connection to ZAP ESP opened');
    setTimeout(function() {
        websocket.send("{'cmd': 'get_Current_Config'}");
    }, 500);				
}

function onClose(event) {
    addLogLine('Connection to ZAP ESP closed');
    console.log('Connection to ZAP ESP closed');
    setTimeout(initWebSocket, 5000);
}

function onMessage(event) {
    console.log("received msg");
    var msgData = JSON.parse(event.data);
    console.log("event data: ", msgData);				
    switch(msgData.msgType){
        case "notify":
            addLogLine(msgData.data);
            break;
        case "ConfigData":
            doSettingsReceive(msgData.data);
            break;
        case "getUIDExtdRec":
            //console.log('msgData.data',msgData.data);
            processUIDExtData(msgData.data);
            break;
        case "UIDTokenID":
            doReceiveUID(msgData.data);
            break;
    }
}

function setMenuButtons(){
    if(currConfig){
        //console.log("currConfig.audio_enabled:", currConfig.audio_enabled)
        if(currConfig.audio_enabled){
            document.getElementById("FMButton").classList.remove('disabledli');
            document.getElementById("DefaultsButton").classList.remove('disabledli');
            document.getElementById("UIDButton").classList.remove('disabledli');
            setDefaultsDivs();						
        }else if(!currConfig.audio_enabled && currConfig.motor_enabled){
            document.getElementById("DefaultsButton").classList.remove('disabledli');
            setDefaultsDivs();					
        }

    }
}

function setDefaultsDivs(){
    if(!currConfig.audio_enabled){
        document.getElementById("audioDefFilesDiv").style.display = "none";
        document.getElementById("audioDefGainDiv").style.display = "none";
    }
    if(!currConfig.motor_enabled){
        document.getElementById("rumbleDefDiv").style.display = "none";
    }
}

function toggleDefSettingsMode(){
    var elSetDiv = document.getElementById("settingsDiv");
    elSetDiv.style.display = "none";
    var elFMDiv = document.getElementById("FileManDiv");
    elFMDiv.style.display = "none";
    var elZapDiv = document.getElementById("ZapSettingsDiv");
    elZapDiv.style.display = "none";
    var elTagDiv = document.getElementById("tagGenDiv");
    elTagDiv.style.display = "none";
    var elUIDDiv = document.getElementById("UIDExtDiv");
    elUIDDiv.style.display = "none";
    if(bWriteMode){websocket.send(
        "{'cmd': 'set_WriteMode', 'data': false}")
        bWriteMode = false;
    }
    if(bUIDMode){websocket.send(
        "{'cmd': 'set_UIDMode', 'data': false}")
        bWriteMode = false;
    }
    var elAudioDiv = document.getElementById("DefSettingsDiv");
    if(elAudioDiv.style.display == "block"){
        elAudioDiv.style.display = "none";
    }else{
        elAudioDiv.style.display = "block";
    }
}

function toggleUIDMode(){
    var elSetDiv = document.getElementById("settingsDiv");
    elSetDiv.style.display = "none";
    var elFMDiv = document.getElementById("FileManDiv");
    elFMDiv.style.display = "none";
    var elZapDiv = document.getElementById("ZapSettingsDiv");
    elZapDiv.style.display = "none";
    var elTagDiv = document.getElementById("tagGenDiv");
    elTagDiv.style.display = "none";
    var elAudioDiv = document.getElementById("DefSettingsDiv");
    elAudioDiv.style.display = "none";
    if(bWriteMode){websocket.send(
        "{'cmd': 'set_WriteMode', 'data': false}")
        bWriteMode = false;
    }
    var elUIDDiv = document.getElementById("UIDExtDiv");
    if(elUIDDiv.style.display == "block"){
        elUIDDiv.style.display = "none";
        websocket.send("{'cmd': 'set_UIDMode', 'data': false}");
        bUIDMode = false;
        document.activeElement.blur();
        document.getElementById("ZAPButton").classList.remove('disabledli');
        document.getElementById("ESP32Button").classList.remove('disabledli');
        document.getElementById("DefaultsButton").classList.remove('disabledli');
        document.getElementById("NFCButton").classList.remove('disabledli');
    }else{
        elUIDDiv.style.display = "block";
        if(!bUIDMode){
            websocket.send("{'cmd': 'set_UIDMode', 'data': true}");
            bUIDMode = true;
            doUIDExtGetData();
        }
        document.getElementById("ZAPButton").classList.add('disabledli');
        document.getElementById("ESP32Button").classList.add('disabledli');
        document.getElementById("DefaultsButton").classList.add('disabledli');
        document.getElementById("NFCButton").classList.add('disabledli');
    }
}

function toggleZapSettingsMode(){
    var elSetDiv = document.getElementById("settingsDiv");
    elSetDiv.style.display = "none";
    var elFMDiv = document.getElementById("FileManDiv");
    elFMDiv.style.display = "none";
    var elAudioDiv = document.getElementById("DefSettingsDiv");
    elAudioDiv.style.display = "none";
    var elTagDiv = document.getElementById("tagGenDiv");
    elTagDiv.style.display = "none";
    var elUIDDiv = document.getElementById("UIDExtDiv");
    elUIDDiv.style.display = "none";
    if(bWriteMode){websocket.send(
        "{'cmd': 'set_WriteMode', 'data': false}")
        bWriteMode = false;
    }
    if(bUIDMode){websocket.send(
        "{'cmd': 'set_UIDMode', 'data': false}")
        bWriteMode = false;
    }
    var elZapDiv = document.getElementById("ZapSettingsDiv");
    if(elZapDiv.style.display == "block"){
        elZapDiv.style.display = "none";
        document.activeElement.blur();
    }else{
        elZapDiv.style.display = "block";
    }
    
}

function toggleSettingsMode() {				
    var elFMDiv = document.getElementById("FileManDiv");
    elFMDiv.style.display = "none";
    var elAudioDiv = document.getElementById("DefSettingsDiv");
    elAudioDiv.style.display = "none";
    var elZapDiv = document.getElementById("ZapSettingsDiv");
    elZapDiv.style.display = "none";
    var elTagDiv = document.getElementById("tagGenDiv");
    elTagDiv.style.display = "none";
    var elUIDDiv = document.getElementById("UIDExtDiv");
    elUIDDiv.style.display = "none";
    if(bWriteMode){websocket.send(
        "{'cmd': 'set_WriteMode', 'data': false}")
        bWriteMode = false;
    }
    if(bUIDMode){websocket.send(
        "{'cmd': 'set_UIDMode', 'data': false}")
        bWriteMode = false;
    }
    var elSetDiv = document.getElementById("settingsDiv");
    if(elSetDiv.style.display == "block"){
        elSetDiv.style.display = "none";
        document.activeElement.blur();
    }else{
        elSetDiv.style.display = "block";
    }
}

function toggleFileManMode(){
    var elSetDiv = document.getElementById("settingsDiv");
    elSetDiv.style.display = "none";
    var elAudioDiv = document.getElementById("DefSettingsDiv");
    elAudioDiv.style.display = "none";
    var elZapDiv = document.getElementById("ZapSettingsDiv");
    elZapDiv.style.display = "none";
    var elTagDiv = document.getElementById("tagGenDiv");
    elTagDiv.style.display = "none";
    var elUIDDiv = document.getElementById("UIDExtDiv");
    elUIDDiv.style.display = "none";
    var elFMDiv = document.getElementById("FileManDiv");
    if(elFMDiv.style.display == "block"){
        elFMDiv.style.display = "none";
        document.activeElement.blur();
    }else{
        elFMDiv.style.display = "block";
    }
}

function toggleNFCMode(){
    var elSetDiv = document.getElementById("settingsDiv");
    elSetDiv.style.display = "none";
    var elFMDiv = document.getElementById("FileManDiv");
    elFMDiv.style.display = "none";
    var elAudioDiv = document.getElementById("DefSettingsDiv");
    elAudioDiv.style.display = "none";
    var elZapDiv = document.getElementById("ZapSettingsDiv");
    elZapDiv.style.display = "none";
    var elUIDDiv = document.getElementById("UIDExtDiv");
    elUIDDiv.style.display = "none";
    var elTagDiv = document.getElementById("tagGenDiv");
    if(bUIDMode){websocket.send(
        "{'cmd': 'set_UIDMode', 'data': false}")
        bWriteMode = false;
    }
    if(elTagDiv.style.display == "block"){
        elTagDiv.style.display = "none";
        websocket.send("{'cmd': 'set_WriteMode', 'data': false}");
        bWriteMode = false;
        document.activeElement.blur();
        document.getElementById("ZAPButton").classList.remove('disabledli');
        document.getElementById("ESP32Button").classList.remove('disabledli');
        document.getElementById("DefaultsButton").classList.remove('disabledli');
        document.getElementById("UIDButton").classList.remove('disabledli');
    }else{
        elTagDiv.style.display = "block";
        getIndexedSystems();
        if(!bWriteMode){websocket.send("{'cmd': 'set_WriteMode', 'data': true}");}
        bWriteMode = true;
        document.getElementById("ZAPButton").classList.add('disabledli');
        document.getElementById("ESP32Button").classList.add('disabledli');
        document.getElementById("DefaultsButton").classList.add('disabledli');
        document.getElementById("UIDButton").classList.add('disabledli');
    }

}

function closeAllDivs(){
    var elSetDiv = document.getElementById("settingsDiv");
    elSetDiv.style.display = "none";
    var elFMDiv = document.getElementById("FileManDiv");
    elFMDiv.style.display = "none";
    var elAudioDiv = document.getElementById("DefSettingsDiv");
    elAudioDiv.style.display = "none";
    var elZapDiv = document.getElementById("ZapSettingsDiv");
    elZapDiv.style.display = "none";
    var elTagDiv = document.getElementById("tagGenDiv");
    elTagDiv.style.display = "none";
}

function validateAudioPath(audioFilePath){
    if(audioFilePath.length > 0){
        var firstChar = audioFilePath.charAt(0);
        if(firstChar != "/"){
            audioFilePath = "/" + audioFilePath
        }
    }
    return audioFilePath;
}

function doSettingsSave() {
    // Force all elements to lose focus
    document.activeElement.blur();
    console.log("Settings Save");
    var currMisterVal = document.getElementById("misterIP").value;
    if(!currMisterVal){currMisterVal = "mister.local"};
    var currRetRem = true;				
    if(!document.getElementById("selResetOnRem").checked){currRetRem = false;};
    var currSelEnWiFiLED = false;
    if(document.getElementById("selEnWiFiLED").checked){currSelEnWiFiLED = true;}
    var currSelEnLaunchLED = false;
    if(document.getElementById("selEnLaunchLED").checked){currSelEnLaunchLED = true;}
    var currSelEnPowerLED = false;
    if(document.getElementById("selEnPowerLED").checked){currSelEnPowerLED = true;}
    var currSelEnMotor = false;
    if(document.getElementById("selEnMotor").checked){currSelEnMotor = true;}
    var currSelEnAudio = false;
    if(document.getElementById("selEnAudio").checked){currSelEnAudio = true;}
    var currwiFiLEDPin = document.getElementById("wiFiLEDPin").value;
    if(!currwiFiLEDPin){currwiFiLEDPin = 4;}
    var currlaunchLEDPin = document.getElementById("launchLEDPin").value;
    if(!currlaunchLEDPin){currlaunchLEDPin = 33;}
    var currpowerLEDPin = document.getElementById("powerLEDPin").value;
    if(!currpowerLEDPin){currpowerLEDPin = 22;}
    var currmotorPin = document.getElementById("motorPin").value;
    if(!currmotorPin){currmotorPin = 25;}
    var currI2SDOUTPin = document.getElementById("I2SDOUTPin").value;
    if(!currI2SDOUTPin){currI2SDOUTPin = 26;}
    var currI2SBCLKPin = document.getElementById("I2SBCLKPin").value;
    if(!currI2SBCLKPin){currI2SBCLKPin = 17;}
    var currI2SLRCPin = document.getElementById("I2SLRCPin").value;
    if(!currI2SLRCPin){currI2SLRCPin = 21;}
    var curraudioGainInp = document.getElementById("audioGainInp").value;
    if(!curraudioGainInp){curraudioGainInp = 1;}
    var curraudioFilePath = document.getElementById("audioFilePath").value;
    if(!curraudioFilePath){curraudioFilePath = "";}else{curraudioFilePath = validateAudioPath(curraudioFilePath)}
    var curraudioDetFilePath = document.getElementById("audioDetectedFilePath").value;
    if(!curraudioDetFilePath){curraudioDetFilePath = "";}else{curraudioDetFilePath = validateAudioPath(curraudioDetFilePath)}
    var curraudioRemFilePath = document.getElementById("audioRemovedFilePath").value;
    if(!curraudioRemFilePath){curraudioRemFilePath = "";}else{curraudioRemFilePath = validateAudioPath(curraudioRemFilePath)}
    var curraudioErrFilePath = document.getElementById("audioErrFilePath").value;
    if(!curraudioErrFilePath){curraudioErrFilePath = "";}else{curraudioErrFilePath = validateAudioPath(curraudioErrFilePath)}
    var currSteamVal = document.getElementById("steamIP").value;
    if(!currSteamVal){currSteamVal = "steamOS.local"};
    var currSelSteamOS = true;				
    if(!document.getElementById("selEnSteamMode").checked){currSelSteamOS = false;};
    var currSelMister = true;				
    if(!document.getElementById("selEnMisterMode").checked){currSelMister = false;};
    var currSelSDCard = true;				
    if(!document.getElementById("selEnSDCard").checked){currSelSDCard = false;};
    var currRumbleOnDet = true;
    if(!document.getElementById("selEnRumDet").checked){currRumbleOnDet = false;}
    var currRumbleOnLaunch = true;
    if(!document.getElementById("selEnRumLaunch").checked){currRumbleOnLaunch = false;}
    var currRumbleOnRem = true;
    if(!document.getElementById("selEnRumRem").checked){currRumbleOnRem = false;}
    var currRumbleOnErr = true;
    if(!document.getElementById("selEnRumErr").checked){currRumbleOnErr = false;}		
    var currSettings = {
        cmd: "set_Current_Config",
        data: {
            wifi_led_enabled: currSelEnWiFiLED,
            motor_enabled: currSelEnMotor,
            launch_led_enabled: currSelEnLaunchLED,
            audio_enabled: currSelEnAudio,
            pwr_led_enabled: currSelEnPowerLED,
            reset_on_remove_enabled: currRetRem,
            defAudioPath: curraudioFilePath,
            ZapIP: currMisterVal,
            pin_I2S_DOUT: currI2SDOUTPin,
            pin_I2S_BCLK: currI2SBCLKPin,
            pin_I2S_LRC: currI2SLRCPin,
            val_AUDIO_GAIN: curraudioGainInp,
            pin_MOTOR_PIN: currmotorPin,
            pin_WIFI_LED_PIN: currwiFiLEDPin,
            pin_LAUNCH_LED_PIN: currlaunchLEDPin,
            pin_EXTERNAL_POWER_LED: currpowerLEDPin,
            defDetectAudioPath: curraudioDetFilePath,
            defRemoveAudioPath: curraudioRemFilePath,
            defErrAudioPath: curraudioErrFilePath,
            systemAudio_enabled: false,
            gameAudio_enabled: false,
            sdCard_enabled: currSelSDCard,
            steamOS_enabled: currSelSteamOS,
            SteamIP: currSteamVal,
            mister_enabled: currSelMister,
            buzz_on_detect_enabled: currRumbleOnDet,
            buzz_on_launch_enabled: currRumbleOnLaunch,
            buzz_on_remove_enabled: currRumbleOnRem,
            buzz_on_error_enabled: currRumbleOnErr,

        }
    }
    console.log("Data: ", currSettings);
    websocket.send(JSON.stringify(currSettings));
    var elSetDiv = document.getElementById("settingsDiv");
    elSetDiv.style.display = "none";
    setTimeout(function() {
        addLogLine("Zap ESP will now reboot. This page will auto refresh in 5 seconds");
    }, 1000);
    setTimeout(function() {
        window.location.reload();
    }, 5000);
}

function doSettingsReceive(cfgDataJson){
    addLogLine("Settings Received from Zap ESP");
    document.getElementById("misterIP").value = cfgDataJson.ZapIP;
    document.getElementById("selResetOnRem").checked = cfgDataJson.reset_on_remove_enabled;
    document.getElementById("selEnWiFiLED").checked = cfgDataJson.wifi_led_enabled;
    document.getElementById("selEnLaunchLED").checked = cfgDataJson.launch_led_enabled;
    document.getElementById("selEnPowerLED").checked = cfgDataJson.pwr_led_enabled;
    document.getElementById("selEnMotor").checked = cfgDataJson.motor_enabled;
    document.getElementById("selEnAudio").checked = cfgDataJson.audio_enabled;
    document.getElementById("wiFiLEDPin").value = cfgDataJson.pin_WIFI_LED_PIN;
    document.getElementById("launchLEDPin").value = cfgDataJson.pin_LAUNCH_LED_PIN;
    document.getElementById("powerLEDPin").value = cfgDataJson.pin_EXTERNAL_POWER_LED;
    document.getElementById("motorPin").value = cfgDataJson.pin_MOTOR_PIN;
    document.getElementById("I2SDOUTPin").value = cfgDataJson.pin_I2S_DOUT;
    document.getElementById("I2SBCLKPin").value = cfgDataJson.pin_I2S_BCLK;
    document.getElementById("I2SLRCPin").value = cfgDataJson.pin_I2S_LRC;
    document.getElementById("audioGainInp").value = cfgDataJson.val_AUDIO_GAIN;
    document.getElementById("audioFilePath").value = cfgDataJson.defAudioPath;
    document.getElementById("audioDetectedFilePath").value = cfgDataJson.defDetectAudioPath;
    document.getElementById("audioRemovedFilePath").value = cfgDataJson.defRemoveAudioPath;
    document.getElementById("audioErrFilePath").value = cfgDataJson.defErrAudioPath;
    document.getElementById("steamIP").value = cfgDataJson.SteamIP;
    document.getElementById("selEnSteamMode").checked = cfgDataJson.steamOS_enabled;
    document.getElementById("selEnMisterMode").checked = cfgDataJson.mister_enabled;
    document.getElementById("selEnSDCard").checked = cfgDataJson.sdCard_enabled;
    document.getElementById("selEnRumErr").checked = cfgDataJson.buzz_on_error_enabled;
    document.getElementById("selEnRumLaunch").checked = cfgDataJson.buzz_on_launch_enabled;
    document.getElementById("selEnRumDet").checked = cfgDataJson.buzz_on_detect_enabled;
    document.getElementById("selEnRumRem").checked = cfgDataJson.buzz_on_remove_enabled;
    if(!cfgDataJson.PN532_module){
        document.getElementById("selEnSDCard").checked = false;
        document.getElementById("selEnSDCard").disabled = true;
    };
    //set the global currConfig
    currConfig = cfgDataJson;
    setMenuButtons();
}

function makeCode () {		
    var elText = document.getElementById("resSel").value;
    var mip = document.getElementById("misterIP").value;
    var tmpText = "http://" + mip + ":7497/l/" + elText;
    console.log("QRCode Data", encodeURI(tmpText))
    if (elText == "null") {
        alert("Select a Game");
        elText.focus();
        return;
    }
    qrcode.makeCode(encodeURI(tmpText));
    var elwrDiv = document.getElementById("wrTagDiv");
    elwrDiv.style.display = "block";
    var elwrAudioDiv = document.getElementById("wrTagAudioDiv");
    elwrAudioDiv.style.display = "block";
    
}

function writeTag(){
    var elText = document.getElementById("resSel").value;
    var curraudioTAGRemFilePath = document.getElementById("audioTAGRemovedFilePath").value;
    if(!curraudioTAGRemFilePath){curraudioTAGRemFilePath = "";}else{curraudioTAGRemFilePath = validateAudioPath(curraudioTAGRemFilePath)}
    var curraudioTAGLaunchFilePath = document.getElementById("audioTAGGameLaunchFilePath").value;
    if(!curraudioTAGLaunchFilePath){curraudioTAGLaunchFilePath = "";}else{curraudioTAGLaunchFilePath = validateAudioPath(curraudioTAGLaunchFilePath)}		
    var tmpLCMD = `{"cmd": "write_Tag_Launch_Game", "launchData": "${elText}", "audioLaunchPath": "${curraudioTAGLaunchFilePath}", "audioRemovePath": "${curraudioTAGRemFilePath}"}`;
    //console.log("Tag Data: ", tmpLCMD);
    websocket.send(tmpLCMD);
}

function testLaunch(){
    var elText = document.getElementById("resSel").value;
    var tmpLCMD = `{"cmd": "Test_Tag_Launch_Game", "data": "${elText}"}`;
    //console.log("Tag Data: ", tmpLCMD);
    websocket.send(tmpLCMD);
}