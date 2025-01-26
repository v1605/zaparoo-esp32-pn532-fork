
export interface ConfigMessage{
  msgType: "ConfigData";
  data: ConfigData
}

export interface UIDExtdRecsMessage{
  msgType: "getUIDExtdRec";
  data: UIDExtdRecords;
}

export interface PushedUIDTokenMessage{
  msgType: "UIDTokenID";
  data: string;
}

export interface EspMessage{
  cmd: string;
  data: any;
}

export interface ConfigData {
  wifiLedEnabled: boolean;
  motorEnabled: boolean;
  launchLedEnabled: boolean;
  audioEnabled: boolean;
  pwrLedEnabled: boolean;
  resetOnRemove: boolean;
  ZapIP: string;
  i2sDoutPin: number;
  i2sBclkPin: number;
  i2sLrcPin: number;
  audioGain: number;
  motorPin: number;
  wifiLedPin: number;
  launchLedPin: number;
  pwrLedPin: number;
  mister_enabled: boolean;
  steamOS_enabled: boolean;
  sdCardEnabled: boolean;
  SteamIP: string;
  defaultLaunchAudio: string;
  defaultInsertAudio: string;
  defaultRemoveAudio: string;
  defaultErrorAudio: string;
  systemAudioEnabled: boolean;
  gameAudioEnabled: boolean;
  buzzOnDetect: boolean;
  buzzOnLaunch: boolean;
  buzzOnRemove: boolean;
  buzzOnError: boolean;
  PN532_module: boolean;
  zap_ws_path: string;
};

export interface UIDExtdRecord {
  UID: string;
  launchAudio: string;
  removeAudio: string;
};

export interface UIDExtdRecords {
  UID_ExtdRecs: Array<UIDExtdRecord>;
}

export interface zapSystem {
  id: string;
  name: string;
  category: string;
};

export interface zapSystems {
  systems: Array<zapSystem>
}

export interface zapSearchResult {
  system: zapSystem;
  name: string;
  path: string;
}

export interface zapSearchResults {
  results: Array<zapSearchResult>;
  total: number;
}

export interface htmlFormattedSearchRec {
  path: string;
  name: string;
}

export interface htmlFormattedSearchRes {
  results: Array<htmlFormattedSearchRec>
}

export interface writeResultState {
  state: number
}

export interface indexingMessage {
  params: indexingParams
}

export interface indexingParams {
  totalSteps: number,
  currentStep: number
}

export interface sourceZapSvsList {
  sources: Array<sourceZapSvs>
}

export interface sourceZapSvs {
  name: string,
  value: string
}