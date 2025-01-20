import { writable, type Readable, type Writable } from "svelte/store";
import type { zapSystem, zapSystems, htmlFormattedSearchRes, zapSearchResults, ConfigData, sendToESPMessage, writeResultState } from "../types/ConfigData";
import {v4 as uuidv4} from 'uuid';
import { EspUtils, } from "./EspUtils";
import { LogUtils } from "./LogUtils";
export class zapUtils{
    private static retSystems: Writable<zapSystems> = writable({} as zapSystems);
    private static srchResults: Writable<zapSearchResults> = writable({} as zapSearchResults);
    private static htmlResults: Writable<htmlFormattedSearchRes> = writable({} as htmlFormattedSearchRes);
    private static bSteamOSConnected = false;
    private static bMisterConnected = false;
    private static currConfig: ConfigData;
    private static steamSocket: WebSocket;
    private static misterSocket: WebSocket;
    private static isCreateModeEnabled = false;
    private static writeResultState: Writable<writeResultState> = writable({} as writeResultState);
    
    private static buildZapSocketURL(ip: string, path: string): string{
        return `ws://${ip}:7497${path}`;
    }

    static getBlankSystem(): zapSystem {
        return {} as zapSystem;
    }

    static getBlankSystems(): zapSystems {
        return {} as zapSystems;
    }

    static getBlankSearchResults(): zapSearchResults {
        return {} as zapSearchResults;
    }

    private static getBlankESPMsg(): sendToESPMessage{
        return {} as sendToESPMessage;
    }

    static getBlankhmtlSrchRes(): htmlFormattedSearchRes{
        return {} as htmlFormattedSearchRes;
    }

    static getBlankWriteState(): writeResultState{
        return {} as writeResultState;
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
        //console.log("Mister Msg Data: ", msgData);        
        ///return of systems list
        if(msgData.result.systems){
            let tmpSysList: zapSystems = msgData.result;
            this.processMiSTerSystems(tmpSysList);
        }
        //return of search results
        if(msgData.result.results){
            let tmpSearchRes: zapSearchResults = msgData.result;
            this.processSearchResults(tmpSearchRes);
        }
    }

    private static misterOnError(){
        LogUtils.notify("Unable to connect to Zaparoo on MiSTer: Check IP/service is running & reload window");
        this.bMisterConnected = false;
    }

    private static processSearchResults(recSearchRes: zapSearchResults){
        if(recSearchRes.total > 250){
            LogUtils.notify(`Max 250 results Shown (${recSearchRes.total}) - Refine your search`);
        }
        let tmpHtmlSR = this.getBlankhmtlSrchRes();
        tmpHtmlSR.results = [];
        for (var i = 0; i < recSearchRes.results.length; i++) {
            tmpHtmlSR.results.push({
                path: recSearchRes.results[i].path,
                name: `${recSearchRes.results[i].system.name} - ${recSearchRes.results[i].name}`
            });
        }
        this.srchResults.set(recSearchRes);
        this.htmlResults.set(tmpHtmlSR);
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

    static htmlSrchRes(): Readable<htmlFormattedSearchRes>{
        return this.htmlResults;
    }

    static zapSrcRes(): Readable<zapSearchResults>{
        return this.srchResults;
    }

    static writeResStat(): Readable<writeResultState>{
        return this.writeResultState;
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
    
    static doSearch(sysName: string, srchQuery: string | null) {
        let newUUID = uuidv4();
        let tmpParams;
        if(sysName != "*"){
            tmpParams = {query: srchQuery, maxResults: 250, systems:[sysName]}
        }else{
            tmpParams = {query: srchQuery, maxResults: 250}
        }
        let wscmd = {
            jsonrpc: "2.0",
            id: newUUID,
            method: "media.search",
            params: tmpParams
        };
        this.misterSocket.send(JSON.stringify(wscmd));
        if(sysName == "steam" && this.bSteamOSConnected){
            //do steamOS search
            this.steamSocket.send(JSON.stringify(wscmd));
        }else if(sysName != "steam" && this.bMisterConnected){
            //do MiSTer search
            this.misterSocket.send(JSON.stringify(wscmd));
        }
    }

    static doTestLaunch(launchPath: string){
        let newCMD = this.getBlankESPMsg();
        newCMD.cmd = "Test_Tag_Launch_Game";
        newCMD.data = launchPath;
        EspUtils.sendMessage(newCMD);
        LogUtils.notify(`Sent Launch Cmd for: ${launchPath}`);
    }

    static doWriteCard(launchPath: string, aLaunchP: string | null, aRemoveP: string | null){
        let tmpWRS = this.getBlankWriteState();
        tmpWRS.state = 0;        
        this.writeResultState.set(tmpWRS);
        let newCMD = this.getBlankESPMsg();
        newCMD.cmd = "write_Tag_Launch_Game";
        newCMD.data = {
            launchData: launchPath,
            audioLaunchPath: aLaunchP,
            audioRemovePath: aRemoveP
        }
        console.log("write cmd: ", newCMD);
        EspUtils.sendMessage(newCMD);
    }

    static toggleCreateMode(isEnabled: boolean){
        this.isCreateModeEnabled = isEnabled;
        let newCMD = this.getBlankESPMsg();
        console.log(`Setting Create Mode : ${this.isCreateModeEnabled}`);
        newCMD.cmd = "set_WriteMode";
        newCMD.data = this.isCreateModeEnabled;
        EspUtils.sendMessage(newCMD);
    }

    

    static handleWriteResults(isSuccess: boolean, isCardDetected: boolean){
        let tmpWRS = this.getBlankWriteState();
        if(isSuccess){
            tmpWRS.state = 1;
        }else if(!isSuccess && isCardDetected){
            //write failed
            tmpWRS.state = 2;
        }else if(!isSuccess && !isCardDetected){
            //write failed no card detetcted
            tmpWRS.state = 3;
        }
        this.writeResultState.set(tmpWRS);        
    }



}