
<script lang="ts">

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
<div class="text-center">
  <h2>Search</h2>
</div>
<form on:submit={handleSubmit} class="row g-2">
  <div class="col-12"> 
    <div class="input-group">
      <div class="form-floating">
        <input type="text" class="form-control col-6" id="misterIp" placeholder="mister.local" bind:value={misterAddress} disabled={!isMisterEnabled} />
        <label for="misterIp">Mister Address</label>
      </div>
    </div>
  </div>
  <button type="submit" class="btn btn-primary mt-3">Search</button>
</form>
<div class="text-center mt-2">
    <h2>Write</h2>
</div>
<form on:submit={handleSubmit} class="row g-2">
    <button type="submit" class="btn btn-primary mt-3">Write</button>
</form>