export class commonUtils{

    static validateAudioPath(strPath: string):string{
        if(strPath.length !==0){
            var firstChar = strPath.charAt(0);
            if(firstChar != "/"){
                strPath = "/" + strPath
            }
        }
        return strPath;
    }

}

