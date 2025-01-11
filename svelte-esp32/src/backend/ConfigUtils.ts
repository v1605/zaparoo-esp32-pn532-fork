import type { ConfigData } from "../types/ConfigData";

export class ConfigUtils{

    private static currentConfig: ConfigData | undefined;

    static async loadConfig(){}

    static getCurrentConfig(): ConfigData|undefined{
        return this.currentConfig;
    }

    static async updateConfig(update: Partial<ConfigData>){

    }
}