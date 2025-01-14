import { writable, type Readable, type Writable } from "svelte/store";
import type { UIDExtdRecords, UIDExtdRecord, UIDExtdRecsMessage, pushedUIDTokenMessage } from "../types/ConfigData";

export class UIDUtils{
    private static currentUIDData: UIDExtdRecords;
    private static currentUIDRecord: Writable<UIDExtdRecord> = writable({} as UIDExtdRecord);

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
        }
    }

}