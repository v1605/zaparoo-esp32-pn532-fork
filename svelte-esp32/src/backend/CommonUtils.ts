export class CommonUtils{

    static validateAudioPath(strPath: string):string {
        if(strPath){
            strPath = strPath.trim();
            if(strPath.charAt(0) != "/"){
                strPath = "/" + strPath
            }
        }
        return strPath;
    }

}

