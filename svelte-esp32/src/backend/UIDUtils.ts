import { writable, type Readable, type Writable } from "svelte/store";
import type { UIDExtdRecords, UIDExtdRecord, UIDExtdRecsMessage } from "../types/ConfigData";

export class UIDUtils{
    private static currentUIDData: Writable<UIDExtdRecords> = writable({} as UIDExtdRecords);

    private static notification:Writable<string> = writable("");

    static getBlank(): UIDExtdRecords{
        return {} as UIDExtdRecords;
    }

    static UIDRecords(): Readable<UIDExtdRecords> {
        return this.currentUIDData;
    }

    static processUIDExtData(UIDData: UIDExtdRecsMessage){
        console.log("UIDData:", UIDData)
        this.currentUIDData.set((UIDData as UIDExtdRecsMessage).data);
    }

}