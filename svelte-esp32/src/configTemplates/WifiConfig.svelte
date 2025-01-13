<script lang="ts">
    import { EspUtils } from "../backend/EspUtils";
    let wifi = "";
    let password = "";
    const handleSubmit = (event: Event) => {
      event.preventDefault();
      EspUtils.updateWifi(wifi, password);
    }
</script>
<div class="text-center">
  <h2>Wifi</h2>
</div>
  <form on:submit={handleSubmit} class="row g-2">
    <div class="mb-3">
      <label for="ssid" class="form-label">Network</label>
      <input bind:value={wifi} type="text" class="form-control" id="ssid" placeholder="Wifi Network">
    </div>
    <div class="mb-3">
      <label for="password" class="form-label">Password</label>
      <input bind:value={password} type="password" class="form-control" id="password">
    </div>
    <button type="submit" class="btn btn-primary mt-3">Save</button>
</form>