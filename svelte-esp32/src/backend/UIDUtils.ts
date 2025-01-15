import { writable, type Readable, type Writable } from "svelte/store";
import type { UIDExtdRecords, UIDExtdRecord, UIDExtdRecsMessage, pushedUIDTokenMessage, sendToESPMessage } from "../types/ConfigData";
import { EspUtils } from "./EspUtils";
import { LogUtils } from "./LogUtils";

export class UIDUtils{
    private static currentUIDData: UIDExtdRecords;
    private static currentUIDRecord: Writable<UIDExtdRecord> = writable({} as UIDExtdRecord);

    private static getBlankESPMsg(): sendToESPMessage{
        return {} as sendToESPMessage;
    }
    
    static getBlank(): UIDExtdRecord{
        return {} as UIDExtdRecord;
    }
    
    static UIDRecord(): Readable<UIDExtdRecord> {
        return this.currentUIDRecord;
    }

    static processUIDExtData(UIDData: UIDExtdRecsMessage){
        console.log("UIDData:", UIDData)
        let currData: UIDExtdRecords = UIDData.data;
        this.currentUIDData = currData;
        console.log("currentUIDData:", this.currentUIDData);
    }

    static processPushedUID(PushedUIDRecord: pushedUIDTokenMessage){
        console.log("PushedUIDRecord:", PushedUIDRecord)
        let curRec = this.currentUIDData.UID_ExtdRecs.filter((item: {UID: String}) => (item.UID == PushedUIDRecord.data));
        if(curRec.length !== 0){
            this.currentUIDRecord.set(curRec[0]);
        }else{
            let tmpRec = this.getBlank();
            tmpRec.UID = PushedUIDRecord.data;
            tmpRec.launchAudio = "";
            tmpRec.removeAudio = "";
            this.currentUIDRecord.set(tmpRec);
        }
    }

    static updateUIDRecord(update: UIDExtdRecord){
        let bDidUpdate = false;
        let newCMD = this.getBlankESPMsg();
        for (var i = 0; i < this.currentUIDData.UID_ExtdRecs.length; i++) {
            if(this.currentUIDData.UID_ExtdRecs[i].UID == update.UID){
                this.currentUIDData.UID_ExtdRecs[i].launchAudio = update.launchAudio;
                this.currentUIDData.UID_ExtdRecs[i].removeAudio = update.removeAudio;
                bDidUpdate = true;
            }
        }
        if(!bDidUpdate){
            //new record
            var tmpRecord = this.getBlank();
            tmpRecord.UID = update.UID;
            tmpRecord.launchAudio = update.launchAudio;
            tmpRecord.removeAudio = update.removeAudio;
            this.currentUIDData.UID_ExtdRecs.push(tmpRecord)
        }
        newCMD.cmd = "saveUIDExtdRec";
        newCMD.data = this.currentUIDData;
        EspUtils.sendMessage(newCMD);
        LogUtils.notify("UID Control Record Saved");
    }

}