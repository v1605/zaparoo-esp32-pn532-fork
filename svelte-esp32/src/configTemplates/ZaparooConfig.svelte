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
<h2>Zaparoo Settings</h2>
<form on:submit={handleSubmit}>
    <div class="input-group">
      <div class="form-floating">
        <input type="text" class="form-control" id="misterIp" placeholder="mister.local" bind:value={config.ZapIP} disabled={!config.mister_enabled} />
        <label for="misterIp">Mister Address</label>
      </div>
      <div class="input-group-text">
        <div class="form-check form-switch">
          <input class="form-check-input" type="checkbox" role="switch" id="enableMister" bind:checked={config.mister_enabled}/>
          <label class="form-check-label visually-hidden" for="enableMister">Enable Mister</label>
        </div>
      </div>
    </div>
    <button type="submit" class="btn btn-primary mt-3">Save</button>
  </form>