import { writable, type Readable, type Writable } from "svelte/store";
import type { zapSystem, zapSystems, zapSearchResult, zapSearchResults, ConfigData } from "../types/ConfigData";
import {v4 as uuidv4} from 'uuid';
import { EspUtils, } from "./EspUtils";
import { LogUtils } from "./LogUtils";

export class zapUtils{
    private static retSystems: Writable<zapSystems> = writable({} as zapSystems);
    private static bSteamOSConnected = false;
    private static bMisterConnected = false;
    private static currConfig: ConfigData = EspUtils.getBlank();
    private static steamSocket: WebSocket;
    private static misterSocket: WebSocket;
    
    private static buildZapSocketURL(ip: string, path: string): string{
        return `ws://${ip}:7497${path}`;
    }

    static getBlankSystem(): zapSystem {
        return {} as zapSystem;
    }

    static getBlankSystems(): zapSystems {
        return {} as zapSystems;
    }

    static initConnections(){
        EspUtils.config().subscribe(value=> this.currConfig = value);
        if(this.currConfig.steamOS_enabled){this.initSteamConn();}
        if(this.currConfig.mister_enabled){this.initMisterConn();}
        if(!this.currConfig.steamOS_enabled && !this.currConfig.mister_enabled){
            LogUtils.notify("Both MiSTer & SteamOS are disabled in settings");
        }
    }

    private static initSteamConn() {
        this.steamSocket = new WebSocket(this.buildZapSocketURL(this.currConfig.SteamIP, this.currConfig.zap_ws_path));
        this.steamSocket.onopen    = this.steamOnOpen.bind(this);
        this.steamSocket.onclose   = this.steamOnClose.bind(this);
        this.steamSocket.onmessage = this.steamOnMessage.bind(this);
        this.steamSocket.onerror = this.steamOnError.bind(this);
    }

    private static steamOnOpen(){
        LogUtils.notify("Connected to Zaparoo on SteamOS");
        this.bSteamOSConnected = true;
    }

    private static steamOnClose(){
        this.bSteamOSConnected = false;
        setTimeout(()=> this.initSteamConn(), 2000);
    }

    private static steamOnMessage(event: MessageEvent){
        const msgData = JSON.parse(event.data);
        console.log("Steam Msg Data: ", event.data);
    }

    private static steamOnError(){
        LogUtils.notify("Unable to connect to Zaparoo on SteamOS: Check IP/service is running & reload window");
        this.bSteamOSConnected = false;
    }

    private static initMisterConn() {
        this.misterSocket = new WebSocket(this.buildZapSocketURL(this.currConfig.ZapIP, this.currConfig.zap_ws_path));
        this.misterSocket.onopen    = this.misterOnOpen.bind(this);
        this.misterSocket.onclose   = this.misterOnClose.bind(this);
        this.misterSocket.onmessage = this.misterOnMessage.bind(this);
        this.misterSocket.onerror = this.misterOnError.bind(this);
    }

    private static misterOnOpen(){
        LogUtils.notify("Connected to Zaparoo on MiSTer");
        this.bMisterConnected = true;
        this.generateIndexedSystemsList();
    }

    private static misterOnClose(){
        this.bMisterConnected = false;
        setTimeout(()=> this.initMisterConn(), 2000);
    }

    private static misterOnMessage(event: MessageEvent){
        const msgData = JSON.parse(event.data);
        console.log("Mister Msg Data: ", msgData);        
        ///return of systems list
        if(msgData.result.systems){
            let tmpSysList: zapSystems = msgData.result;
            this.processMiSTerSystems(tmpSysList);
        }
    }

    private static misterOnError(){
        LogUtils.notify("Unable to connect to Zaparoo on MiSTer: Check IP/service is running & reload window");
        this.bMisterConnected = false;
    }

    private static processMiSTerSystems(recSysList: zapSystems){
        recSysList.systems.sort(function (a: any, b: any) {
            if (a.name < b.name) {
                return -1;
            }
            if (a.name > b.name) {
                return 1;
            }
            return 0;
        });
        //if steam is connected add it to the top of the list
        let tmpAllObj = {category: "all", id: "*", name:"All MiSTer Systems"};
        recSysList.systems.unshift(tmpAllObj);        
        if(this.bSteamOSConnected){
            let tmpObj = {category: "steamOS", id: "steam", name:"Steam"};
            recSysList.systems.unshift(tmpObj);
        }
        this.retSystems.set(recSysList);
    }

    static indexedSystemsList(): Readable<zapSystems> {
        return this.retSystems;
    }

    static generateIndexedSystemsList(){        
        if(this.bMisterConnected){
            let newUUID = uuidv4();
            let wscmd = {
                jsonrpc: "2.0",
                id: newUUID,
                method: "systems"
            };
            this.misterSocket.send(JSON.stringify(wscmd));
        }else if(this.bSteamOSConnected){
            let tmpZapSystems = this.getBlankSystems();
            tmpZapSystems.systems = [
                {category: "steamOS", id: "steam", name:"Steam"}
            ]
            this.retSystems.set(tmpZapSystems);
        }        
    }    

}