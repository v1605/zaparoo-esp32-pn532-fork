<script lang="ts">
     // Define variables for the input and toggle switch
  let misterAddress = '192';
  let isEnabled = false;

  // Optional: Handle form submission
  const handleSubmit = (event) => {
    event.preventDefault();
    console.log('Mister Address:', misterAddress);
    console.log('Is Enabled:', isEnabled);
  };
</script>

<form on:submit={handleSubmit}>
    <div class="input-group">
      <div class="form-floating">
        <input type="text" class="form-control" id="misterIp" placeholder="Enter Mister Address" bind:value={misterAddress}/>
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