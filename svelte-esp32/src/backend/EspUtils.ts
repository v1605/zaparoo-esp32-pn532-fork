import { writable, type Writable } from "svelte/store";
import type { ConfigData, ConfigMessage } from "../types/ConfigData";
import { LogUtils } from "./LogUtils";

export class EspUtils{
    private static currentConfig: Writable<ConfigData> = writable({} as ConfigData);
    private static websocket: WebSocket;


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

    static config(): Writable<ConfigData>{
        return this.currentConfig;
    }

    static getBlank(): ConfigData{
        return {} as ConfigData;
    }

    static async updateConfig(update: Partial<ConfigData>){

    }
}