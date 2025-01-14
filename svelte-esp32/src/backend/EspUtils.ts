import { writable, type Readable, type Writable } from "svelte/store";
import type { ConfigData, ConfigMessage } from "../types/ConfigData";
import { LogUtils } from "./LogUtils";
import { UIDUtils } from "./UIDUtils";

export class EspUtils{
    private static currentConfig: Writable<ConfigData> = writable({} as ConfigData);
    private static websocket: WebSocket;
    private static connected: Writable<boolean> = writable();
    private static intialLoad = false;
    private static updating = false;
    private static isUIDModeEnabled= false;
    


    static initWebSocket() {
        console.log('Trying to open a WebSocket connection to ZAP ESP…');
        this.websocket = new WebSocket('/ws');
        this.websocket.onopen    = this.onOpen.bind(this);
        this.websocket.onclose   = this.onClose.bind(this);
        this.websocket.onmessage = this.onMessage.bind(this);
    }

    private static onOpen() {
        LogUtils.addLogLine('Connection to ZAP ESP opened');
        console.log('Connection to ZAP ESP opened');
        this.websocket.send("{'cmd': 'get_Current_Config'}");			
    }
    
    private static onClose() {
        LogUtils.addLogLine('Connection to ZAP ESP closed');
        console.log('Connection to ZAP ESP closed');
        setTimeout(()=> this.initWebSocket(), 2000);
    }
    
    private static onMessage(event: MessageEvent) {
        const msgData = JSON.parse(event.data);		
        switch(msgData.msgType){
            case "notify":
                LogUtils.addLogLine(msgData.data);
                break;
            case "ConfigData":
                this.currentConfig.set((msgData as ConfigMessage).data)
                LogUtils.notify(`Settings ${this.intialLoad ? "Updated" : "Loaded"}`);
                this.intialLoad = true;
                break;
            case "getUIDExtdRec":
                UIDUtils.processUIDExtData(msgData);
                break;
            case "UIDTokenID":
                UIDUtils.processPushedUID(msgData);
                break;
        }
    }

    static toggleUIDMode(){
        this.isUIDModeEnabled = !this.isUIDModeEnabled;
        console.log(`Setting UID Editing Mode : ${this.isUIDModeEnabled}`);
        this.websocket.send(`{'cmd': 'set_UIDMode', 'data': ${this.isUIDModeEnabled}}`);
    }

    static async loadConfig(){}

    static config(): Readable<ConfigData>{
        return this.currentConfig;
    }

    static getBlank(): ConfigData{
        return {} as ConfigData;
    }

    static getConntected(): Readable<boolean>{
        return this.connected;
    }

    static updateConfig(update: Partial<ConfigData>){
        if(this.updating) return true;
        this.updating = true;
        this.currentConfig.subscribe((conf=>{
            const data = Object.assign({...conf}, update);
            const payload = {
                cmd: "set_Current_Config",
                data: data
            };
            this.websocket.send(JSON.stringify(payload));
            LogUtils.notify("Settings updated, ESP will reload");
            this.updating = false;
            this.websocket.onclose = null;
            this.websocket.close();
            setTimeout(()=> this.initWebSocket(), 2000);
        }))();
    }

    static updateWifi(ssid: string, password: string){
        const payload = {
            cmd: "wifi",
            data: {
                ssid: ssid,
                password: password
            }
        };
        this.websocket.send(JSON.stringify(payload));
        LogUtils.notify("Updated Wifi, ESP will now attempt to reconnect");
    }

    
}