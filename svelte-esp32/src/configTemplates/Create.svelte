<script lang="ts">
  import type { zapSystems, zapSearchResults, htmlFormattedSearchRes, writeResultState } from '../types/ConfigData';
  import { zapUtils } from '../backend/zapUtils';
  import { commonUtils } from '../backend/common';
  let zapSysList: zapSystems = zapUtils.getBlankSystems();
  let zapSrchRes: zapSearchResults = zapUtils.getBlankSearchResults();
  let htmlSerRes: htmlFormattedSearchRes = zapUtils.getBlankhmtlSrchRes();
  let WriteResState: writeResultState = zapUtils.getBlankWriteState();
  zapUtils.zapSrcRes().subscribe(value=> zapSrchRes = value);
  zapUtils.indexedSystemsList().subscribe(value=> zapSysList = value);
  zapUtils.htmlSrchRes().subscribe(value=> htmlSerRes = value);
  zapUtils.writeResStat().subscribe(value => WrStIsChanged(value));
  let selectedSys: string = "*";
  let searchQry: string | null;
  let selectedGame: string = "";
  let audLaunchP: string | null ;
  let audRemoveP: string | null ;
  let dialogWrite: HTMLDialogElement;
  let dialogSuccess: HTMLDialogElement;
  let dialogFailure: HTMLDialogElement;
  let dialogNoCard: HTMLDialogElement;
  let dialogWait: HTMLDialogElement;

  
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

  // Optional: Handle form submission
  const handleSubmit = (event: Event) => {
    event.preventDefault();
    zapUtils.doSearch(selectedSys, searchQry);
  };

  function doWrite() {
    if(audLaunchP){audLaunchP = commonUtils.validateAudioPath(audLaunchP)};
    if(audRemoveP){audRemoveP = commonUtils.validateAudioPath(audRemoveP)};
    zapUtils.doWriteCard(selectedGame, audLaunchP, audRemoveP);
    diagWriteClose();
    diagWaitOpen();
  };

  function diagWriteClose() {
    dialogWrite.close("true");
  }

  function diagWriteOpen() {
    dialogWrite.showModal();
    zapUtils.toggleCreateMode(true);
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
    zapUtils.toggleCreateMode(false);
  }

  function diagFailOpen(){
    dialogFailure.showModal();
  }

  function diagFailClose(){
    dialogFailure.close("true");
    zapUtils.toggleCreateMode(false);
  }

  function diagSuccessClose() {
    dialogSuccess.close("true");
    zapUtils.toggleCreateMode(false);
  }

  function diagSucessOpen() {
    dialogSuccess.showModal();
  }

  function doTestLaunch() {
    zapUtils.doTestLaunch(selectedGame);
  };
    
</script>
{#if zapSysList.systems}
<div class="text-center mb-3">
  <h2>Search for a title</h2>
</div>
{/if}
<form on:submit={handleSubmit} class="row g-3">
  <div class="col-12">
    {#if zapSysList.systems}
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
    {#if !zapSysList.systems}
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
        <button type="button" class="btn btn-primary mt-4" on:click="{diagWriteClose}">Cancel</button>
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
    <h4>Write Failed! ESP resetting. Remove Card -> Click OK & try again</h4>
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