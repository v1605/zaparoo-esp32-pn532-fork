import { writable, type Readable, type Writable } from "svelte/store";

export class LogUtils{

    private static notification:Writable<string> = writable("");


    public static addLogLine(line: String){} 


    public static getNotification():Readable<string>{
        return this.notification;
    }

    public static notify(message: string){
        this.notification.set(message);
    }

}