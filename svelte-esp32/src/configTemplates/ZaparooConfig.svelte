<script lang="ts">
    import { EspUtils } from "../backend/EspUtils";
    import type { ConfigData } from "../types/ConfigData";
    let config: ConfigData = EspUtils.getBlank();
    EspUtils.config().subscribe(value=> config = value);
    const handleSubmit = (event: Event) => {
      event.preventDefault();
      const update: Partial<ConfigData> = {ZapIP: config.ZapIP, mister_enabled: config.mister_enabled};
        EspUtils.updateConfig(update);
    };
</script>
<div class="text-center">
  <h2>Zaparoo Settings</h2>
</div>
  <form on:submit={handleSubmit} class="row g-2">
  <div class="col-12"> 
    <div class="input-group">
      <div class="input-group-text">
        <div class="form-check form-switch col-2">
          <input class="form-check-input" type="checkbox" role="switch" id="enableMister" bind:checked={isMisterEnabled}/>
          <label class="form-check-label visually-hidden" for="enableMister">Enable Mister</label>
        </div>
      </div>
      <div class="form-floating">
        <input type="text" class="form-control col-6" id="misterIp" placeholder="mister.local" bind:value={misterAddress} disabled={!isMisterEnabled} />
        <label for="misterIp">Mister Address</label>
      </div>
      <div class="input-group-text">
        <div class="form-check form-switch col-4">
          <input class="form-check-input" type="checkbox" role="switch" id="enableRRoR" bind:checked={isRRoREnabled} disabled={!isMisterEnabled}/>
          <label class="form-check-label" for="enableRRoR">Reset On Remove</label>
        </div>
      </div>
    </div>
  </div>
  <div>
    <div class="input-group">
      <div class="input-group-text">
        <div class="form-check form-switch col-2">
          <input class="form-check-input" type="checkbox" role="switch" id="enableSteam" bind:checked={isSteamEnabled}/>
          <label class="form-check-label visually-hidden" for="enableSteam">Enable SteamOS</label>
        </div>
      </div>
      <div class="form-floating">
        <input type="text" class="form-control" id="steamIp" placeholder="steam.local" bind:value={steamAddress} disabled={!isSteamEnabled} />
        <label for="steamIp">Steam Address</label>
      </div>
    </div>
  </div>
  <button type="submit" class="btn btn-primary mt-3">Save</button>
</form>