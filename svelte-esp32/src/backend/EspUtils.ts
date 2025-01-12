import { writable, type Readable, type Writable } from "svelte/store";
import type { ConfigData, ConfigMessage } from "../types/ConfigData";
import { LogUtils } from "./LogUtils";

export class EspUtils{
    private static currentConfig: Writable<ConfigData> = writable({} as ConfigData);
    private static websocket: WebSocket;
    private static connected: Writable<boolean> = writable();


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
        setTimeout(() =>{
            this.websocket.send("{'cmd': 'get_Current_Config'}");
        }, 500);				
    }
    
    private static onClose() {
        LogUtils.addLogLine('Connection to ZAP ESP closed');
        console.log('Connection to ZAP ESP closed');
        setTimeout(()=> this.initWebSocket(), 5000);
    }
    
    private static onMessage(event: MessageEvent) {
        const msgData = JSON.parse(event.data);		
        switch(msgData.msgType){
            case "notify":
                LogUtils.addLogLine(msgData.data);
                break;
            case "ConfigData":
                //Object.assign(this.currentConfig, (msgData as ConfigMessage).data);
                this.currentConfig.set((msgData as ConfigMessage).data)
                break;
            case "getUIDExtdRec":
                //console.log('msgData.data',msgData.data);
                //processUIDExtData(msgData.data);
                break;
            case "UIDTokenID":
                //doReceiveUID(msgData.data);
                break;
        }
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
        this.currentConfig.subscribe((conf=>{
            const data = Object.assign({...conf}, update);
            const payload = {
                cmd: "set_Current_Config",
                data: data
            };
            this.websocket.send(JSON.stringify(payload));
        }))();
    }
}