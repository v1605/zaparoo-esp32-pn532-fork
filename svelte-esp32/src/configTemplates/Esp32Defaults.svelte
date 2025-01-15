<script lang="ts">
    import { EspUtils } from "../backend/EspUtils";
    import type { ConfigData } from "../types/ConfigData";
    import { commonUtils } from "../backend/common";
    let config: ConfigData = EspUtils.getBlank();
    EspUtils.config().subscribe(value=> config = value);
    const handleSubmit = (event: Event) => {
      event.preventDefault();
      const update: Partial<ConfigData> = {
        defDetectAudioPath: commonUtils.validateAudioPath(config.defDetectAudioPath),
        defRemoveAudioPath: commonUtils.validateAudioPath(config.defRemoveAudioPath),
        defErrAudioPath: commonUtils.validateAudioPath(config.defErrAudioPath),
        defAudioPath: commonUtils.validateAudioPath(config.defAudioPath),
        buzz_on_detect_enabled: config.buzz_on_detect_enabled,
        buzz_on_launch_enabled: config.buzz_on_launch_enabled,
        buzz_on_remove_enabled: config.buzz_on_remove_enabled,
        buzz_on_error_enabled: config.buzz_on_error_enabled,
        sdCard_enabled: config.sdCard_enabled,
        val_AUDIO_GAIN: config.val_AUDIO_GAIN
      };
      EspUtils.updateConfig(update);
    };
</script>
<div class="text-center">
  <h2>ESP32 Defaults</h2>
</div>
<form on:submit={handleSubmit} class="row g-4">
  <div class="col-12">
    <div class="d-flex flex-column flex-md-row align-items-start justify-content-between">
      <div class="input-group-text col-12 col-md-3">
        <div class="form-check form-switch w-100">
          <input class="form-check-input" type="checkbox" role="switch" id="enableBuzzDet" bind:checked={config.buzz_on_detect_enabled}/>
          <label class="form-check-label w-100" for="enableBuzzDet">Detect Rumble</label>
        </div>
      </div>
      <div class="input-group-text col-12 col-md-3">
        <div class="form-check form-switch w-100">
          <input class="form-check-input" type="checkbox" role="switch" id="enableBuzzLaun" bind:checked={config.buzz_on_launch_enabled}/>
          <label class="form-check-label w-100" for="enableBuzzLaun">Launch Rumble</label>
        </div>
      </div>
      <div class="input-group-text col-12 col-md-3">
        <div class="form-check form-switch w-100">
          <input class="form-check-input" type="checkbox" role="switch" id="enableBuzzRem" bind:checked={config.buzz_on_remove_enabled}/>
          <label class="form-check-label w-100" for="enableBuzzRem">Remove Rumble</label>
        </div>
      </div>
      <div class="input-group-text col-12 col-md-3">
        <div class="form-check form-switch w-100">
          <input class="form-check-input" type="checkbox" role="switch" id="enableBuzzErr" bind:checked={config.buzz_on_error_enabled}/>
          <label class="form-check-label w-100" for="enableBuzzErr">Error Rumble</label>
        </div>
      </div>
    </div>
  </div>
  <div class="col-12">
    <div class="d-flex flex-column flex-md-row align-items-center justify-content-between">
      <div class="form-floating col-12 col-md-3">
        <input type="text" class="form-control" id="defDetPath" placeholder="/" bind:value={config.defDetectAudioPath}/>
        <label for="defDetPath">Detect Audio File</label>
      </div>
      <div class="form-floating col-12 col-md-3">
        <input type="text" class="form-control" id="defLaunPath" placeholder="/" bind:value={config.defAudioPath}/>
        <label for="defLaunPath">Launch Audio File</label>
      </div>
      <div class="form-floating col-12 col-md-3">
        <input type="text" class="form-control" id="defRemPath" placeholder="/" bind:value={config.defRemoveAudioPath}/>
        <label for="defRemPath">Remove Audio File</label>
      </div>
      <div class="form-floating col-12 col-md-3">
        <input type="text" class="form-control" id="defErrPath" placeholder="/" bind:value={config.defErrAudioPath}/>
        <label for="defErrPath">Error Audio File</label>
      </div>
    </div>
  </div>
  <div class="col-12">
    <div class="d-flex flex-column flex-md-row justify-content-center">
      <div class="input-group-text col-12 col-md-4">
        <div class="form-floating">
          <input type="number" class="form-control" step=".1" id="defAudioGain" placeholder="0.4" bind:value={config.val_AUDIO_GAIN}/>
          <label for="defAudioGain">Audio Gain (0.4-4)</label>
        </div>
      </div>
    </div>
  </div>

  <button type="submit" class="btn btn-primary mt-3">Save</button>
</form>
