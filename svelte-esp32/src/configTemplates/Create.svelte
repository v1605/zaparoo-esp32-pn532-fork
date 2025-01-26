<script lang="ts">
  import type { zapSystems, zapSearchResults, htmlFormattedSearchRes, writeResultState, sourceZapSvsList, ConfigData } from '../types/ConfigData';
  import { ZapUtils } from '../backend/ZapUtils';
  import { CommonUtils } from '../backend/CommonUtils';
  import { EspUtils } from "../backend/EspUtils";
    import { LogUtils } from '../backend/LogUtils';
  let zapSysList: zapSystems = ZapUtils.getBlankSystems();
  let zapSvsList: sourceZapSvsList = ZapUtils.getActiveSourceList();
  let zapSrchRes: zapSearchResults = ZapUtils.getBlankSearchResults();
  let htmlSerRes: htmlFormattedSearchRes = ZapUtils.getBlankhmtlSrchRes();
  let selectedSource: string = "";
  let dialogWait: HTMLDialogElement;
  ZapUtils.zapSrcRes().subscribe(value=> zapSrchRes = value);
  ZapUtils.indexedSystemsList().subscribe(value=> zapSysList = value);
  ZapUtils.htmlSrchRes().subscribe(value=> htmlSerRes = value);
  ZapUtils.writeResStat().subscribe(value => WrStIsChanged(value));
  EspUtils.config().subscribe(value=> settingsIsChanged(value));
  let selectedSys: string = "*";
  let searchQry: string | null;
  let selectedGame: string = "";
  let audLaunchP: string | null ;
  let audRemoveP: string | null ;
  let dialogWrite: HTMLDialogElement;
  let dialogSuccess: HTMLDialogElement;
  let dialogFailure: HTMLDialogElement;
  let dialogNoCard: HTMLDialogElement;
    
  function WrStIsChanged(currVal: writeResultState) {
      switch(currVal.state){
        case 1:
          diagWaitClose();
          diagSucessOpen();
          break;
        case 2:
          diagWaitClose();
          diagFailOpen();
          break;
        case 3:
          diagWaitClose();
          diagNoCardOpen();
          break;
      }
  }

  function settingsIsChanged(currSet: ConfigData){
    //monitors for changes to settings and reloads SourceList
    if(currSet.SteamIP != "" || currSet.ZapIP != ""){
      zapSvsList = ZapUtils.getActiveSourceList();
    }
    if(currSet.steamOS_enabled || currSet.mister_enabled){
      zapSvsList = ZapUtils.getActiveSourceList();
    }
    if(!currSet.steamOS_enabled || !currSet.mister_enabled){
      zapSvsList = ZapUtils.getActiveSourceList();
    }    
  }

  function getSystems(){
    ZapUtils.initConnection(selectedSource);
    searchQry = "";
  }
  
  const handleSubmit = (event: Event) => {
    event.preventDefault();
    ZapUtils.doSearch(selectedSys, searchQry);
  };

  function doWrite() {
    if(audLaunchP){audLaunchP = CommonUtils.validateAudioPath(audLaunchP)};
    if(audRemoveP){audRemoveP = CommonUtils.validateAudioPath(audRemoveP)};
    ZapUtils.doWriteCard(selectedGame, audLaunchP, audRemoveP);
    diagWriteClose();
    diagWaitOpen();
  };

  function diagWriteClose() {
    dialogWrite.close("true");
  }

  function diagWriteCancel() {
    dialogWrite.close("true");
    ZapUtils.toggleCreateMode(false);
  }

  function diagWriteOpen() {
    dialogWrite.showModal();
    ZapUtils.toggleCreateMode(true);
  }

  function diagWaitOpen(){
    dialogWait.showModal();
  }

  function diagWaitClose(){
    dialogWait.close("true");
  }

  function diagNoCardOpen(){
    dialogNoCard.showModal();
  }

  function diagNoCardClose(){
    dialogNoCard.close("true");
    ZapUtils.toggleCreateMode(false);
  }

  function diagFailOpen(){
    dialogFailure.showModal();
  }

  function diagFailClose(){
    dialogFailure.close("true");
    ZapUtils.toggleCreateMode(false);
  }

  function diagSuccessClose() {
    dialogSuccess.close("true");
    ZapUtils.toggleCreateMode(false);
  }

  function diagSucessOpen() {
    dialogSuccess.showModal();
  }

  function doTestLaunch() {
    ZapUtils.doTestLaunch(selectedGame);
  };

  function doDBIndex() {
    ZapUtils.updateGamesDB();
  }
    
</script>
<div class="text-center mb-3">
  <h2>Search for a title</h2>
</div>
<form on:submit={handleSubmit} class="row g-3">
  <div class="col-12">
    <div class="input-group mb-3">
      <div class="col-4">
        <label for="selSystem">Select Source</label><select class="form-select" id="selSystem" bind:value={selectedSource} on:change={getSystems}>
          {#each zapSvsList.sources as { value, name}}
            <option value={value}>{name}</option>
          {/each}
        </select>
      </div>
      {#if selectedSource != ""}
      <div class="col-2">
        <button type="button" class="btn btn-primary mt-4" on:click={doDBIndex}>Update Zap DB</button>
      </div>
      {/if}
    </div>
    {#if zapSysList.systems && selectedSource != ""}
    <div class="input-group ">
      <div class="col-5">
        <label for="selSystem">Select System</label><select class="form-select" id="selSystem" bind:value={selectedSys}>
          {#each zapSysList.systems as { id, name}}
            <option value={id}>{name}</option>
          {/each}
        </select>
      </div>
      <div class="col-6">
        <label for="searchQry">Search Query</label>
        <input type="text" class="form-control" id="searchQry" bind:value={searchQry}/>
      </div>
      <div class="col-1">
        <button type="submit" class="btn btn-primary mt-4">Search</button>
      </div>
    </div>
    {/if}
    {#if !zapSysList.systems && selectedSource != ""}
    <div class="text-center mt-5">
      <h3>Unable to connect to Zaparoo service. Check the service is running and IP address is correct.</h3>
    </div>
    <div class="text-center mt-5">
      <h3>Reload this window to try again</h3>
    </div>
    {/if}
    {#if zapSrchRes.total > 0}
      <div class="input-group mt-3">
        <div class="col-8">
          <label for="selSystem">Search Results ({zapSrchRes.total < 250 ? zapSrchRes.total:'250'}) - Select a Game to write to Card</label><select class="form-select" id="selGame" bind:value={selectedGame}>
            {#each htmlSerRes.results as {name, path}}
              <option value={path}>{name}</option>
            {/each}
          </select>
        </div>
      </div>
      {#if selectedGame.length > 0}
        <div class="input-group mt-3">
          <div class="col-6">
            <label for="searchQry">Launch Audio Path</label>
            <input type="text" class="form-control" id="aLauchP" bind:value={audLaunchP}/>
          </div>
          <div class="col-6">
            <label for="searchQry">Remove Audio Path</label>
            <input type="text" class="form-control" id="aRemoveP" bind:value={audRemoveP}/>
          </div>
        </div>
      {/if}
    {/if}
    {#if selectedGame.length > 0}
    <div class="container">
      <div class="row">
        <div class="col text-center">  
          <button type="button" class="btn btn-primary mt-4" on:click="{diagWriteOpen}">Write To Card</button>
        
          <button type="button" class="btn btn-primary mt-4" on:click="{doTestLaunch}">Test Launch</button>
        </div>
      </div>
    </div>
    {/if}
    {#if zapSrchRes.total == 0}
    <div class="text-center mt-3">
      <h2>No Results</h2>
    </div>
    {/if}
  </div>
</form>
<dialog bind:this={dialogWrite}>
  <div class="text-center mb-3">
    <h4>Hold/Insert Card into reader and click OK to write data</h4>
  </div>
  <div class="container">
    <div class="row">
      <div class="col text-center">  
        <button type="button" class="btn btn-primary mt-4" on:click="{doWrite}">OK</button>      
        <button type="button" class="btn btn-primary mt-4" on:click="{diagWriteCancel}">Cancel</button>
      </div>
    </div>
  </div>
</dialog>
<dialog bind:this={dialogSuccess}>
  <div class="text-center mb-3">
    <h4>Remove Card and click OK to continue</h4>
  </div>
  <div class="container">
    <div class="row">
      <div class="col text-center">  
        <button type="button" class="btn btn-primary mt-4" on:click="{diagSuccessClose}">OK</button>      
      </div>
    </div>
  </div>
</dialog>
<dialog bind:this={dialogFailure}>
  <div class="text-center mb-3">
    <h4>Write Failed! Check TAG/Card type. Remove Card -> Click OK & try again</h4>
  </div>
  <div class="container">
    <div class="row">
      <div class="col text-center">  
        <button type="button" class="btn btn-primary mt-4" on:click="{diagFailClose}">OK</button>      
      </div>
    </div>
  </div>
</dialog>
<dialog bind:this={dialogNoCard}>
  <div class="text-center mb-3">
    <h4>Write Failed! Card Not Detected -> Click OK & try again</h4>
  </div>
  <div class="container">
    <div class="row">
      <div class="col text-center">  
        <button type="button" class="btn btn-primary mt-4" on:click="{diagNoCardClose}">OK</button>      
      </div>
    </div>
  </div>
</dialog>
<dialog bind:this={dialogWait}>
  <div class="text-center mb-3">
    <h4>Writing Data To Card - Please Wait</h4>
  </div>
</dialog>