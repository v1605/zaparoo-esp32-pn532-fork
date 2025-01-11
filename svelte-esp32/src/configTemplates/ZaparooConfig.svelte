<script lang="ts">
    import { ConfigUtils } from "../backend/ConfigUtils";

     // Define variables for the input and toggle switch
  let misterAddress = '';
  let isEnabled = false;

  // Optional: Handle form submission
  const handleSubmit = (event: Event) => {
    event.preventDefault();
    ConfigUtils.updateConfig({mister_enabled: isEnabled, ZapIP: misterAddress});
    console.log('Mister Address:', misterAddress);
    console.log('Is Enabled:', isEnabled);
  };
</script>
<h2>Zaparoo Settings</h2>
<form on:submit={handleSubmit}>
    <div class="input-group">
      <div class="form-floating">
        <input type="text" class="form-control" id="misterIp" placeholder="mister.local" bind:value={misterAddress} disabled={!isEnabled} />
        <label for="misterIp">Mister Address</label>
      </div>
      <div class="input-group-text">
        <div class="form-check form-switch">
          <input class="form-check-input" type="checkbox" role="switch" id="enableMister" bind:checked={isEnabled}/>
          <label class="form-check-label visually-hidden" for="enableMister">Enable Mister</label>
        </div>
      </div>
    </div>
    <button type="submit" class="btn btn-primary mt-3">Save</button>
  </form>