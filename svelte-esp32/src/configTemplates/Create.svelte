<script lang="ts">
  import type { zapSystems } from '../types/ConfigData';
  import { zapUtils } from '../backend/zapUtils'
  let zapSysList: zapSystems = zapUtils.getBlankSystems();
  zapUtils.indexedSystemsList().subscribe(value=> zapSysList = value);
  let selectedSys: string = "*";
  let searchQry: string | null;

    //import QRCode from 'qrcode';

     // Define variables for the input and toggle switch
    let misterAddress = '';
    let isMisterEnabled = false;
    let steamAddress = '';
    let isSteamEnabled = false;
  
  // Optional: Handle form submission
    const handleSubmit = (event: Event) => {
        event.preventDefault();
        
    };
    // import { onMount } from 'svelte';
    // let qrJSReady = false;
    // let mounted = false;

    // //QRCode.toCanvas();

    // onMount(() => {
    //     // component is ready.
    //     mounted = true;
    //     if (qrJSReady) {
    //         doClientSideCode();
    //     }
    // });

    // function qrJSLoaded() {
    //     // The QR javascript is ready.
    //     qrJSReady = true;
    //     if (mounted) {
    //         doClientSideCode();
    //     }
    // }

    // function doClientSideCode() {
    //     //do whatever here
    // } 
</script>
<div class="text-center mb-3">
  <h2>Search</h2>
</div>
<form on:submit={handleSubmit} class="row g-3">
  <div class="col-12">
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
  </div>
  <!-- <button type="submit" class="btn btn-primary mt-3">Search</button> -->
</form>