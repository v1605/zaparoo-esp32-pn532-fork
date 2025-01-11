<script lang="ts">
    import { ConfigUtils } from "../backend/ConfigUtils";

     // Define variables for the input and toggle switch
  let strDefLaunchPath = "";
  let strDefRemovePath = "";
  let strDefErrorPath = "";
  let strDefDetectPath = "";
  let isSDCardEnabled = false;
  let isBuzzRemoveEnabled = false;
  let isBuzzLaunchEnabled = false;
  let isBuzzDetectEnabled = false;
  let isBuzzErrorEnabled = false;
  let intAudioGain: number|null = null;

  // Optional: Handle form submission
  const handleSubmit = (event: Event) => {
    event.preventDefault();
    ConfigUtils.updateConfig({
      defDetectAudioPath: strDefDetectPath,
      defRemoveAudioPath: strDefRemovePath,
      defErrAudioPath: strDefErrorPath,
      defAudioPath: strDefLaunchPath,
      buzz_on_detect_enabled: isBuzzDetectEnabled,
      buzz_on_launch_enabled: isBuzzLaunchEnabled,
      buzz_on_remove_enabled: isBuzzRemoveEnabled,
      buzz_on_error_enabled: isBuzzErrorEnabled,
      sdCard_enabled: isSDCardEnabled,
      val_AUDIO_GAIN: intAudioGain
    });
  };
</script>
<h2>ESP32 Defaults</h2>
<form on:submit={handleSubmit} class="row g-4">
  <div class="col-12"> 
    <div class="input-group">
      <div class="input-group-text col-3">
        <div class="form-check form-switch">
          <input class="form-check-input" type="checkbox" role="switch" id="enableBuzzDet" bind:checked={isBuzzDetectEnabled}/>
          <label class="form-check-label" for="enableBuzzDet">Detect Rumble</label>
        </div>
      </div>
      <div class="input-group-text col-3">
        <div class="form-check form-switch">
          <input class="form-check-input" type="checkbox" role="switch" id="enableBuzzLaun" bind:checked={isBuzzLaunchEnabled}/>
          <label class="form-check-label" for="enableBuzzLaun">Launch Rumble</label>
        </div>
      </div>
      <div class="input-group-text col-3">
        <div class="form-check form-switch">
          <input class="form-check-input" type="checkbox" role="switch" id="enableBuzzRem" bind:checked={isBuzzRemoveEnabled}/>
          <label class="form-check-label" for="enableBuzzRem">Remove Rumble</label>
        </div>
      </div>
      <div class="input-group-text col-3">
        <div class="form-check form-switch">
          <input class="form-check-input" type="checkbox" role="switch" id="enableBuzzErr" bind:checked={isBuzzErrorEnabled}/>
          <label class="form-check-label" for="enableBuzzErr">Error Rumble</label>
        </div>
      </div>
    </div>
  </div>
  <div class="col-12">
    <div class="input-group">
      <div class="form-floating">
        <input type="text" class="form-control col-3" id="defDetPath" placeholder="/" bind:value={strDefDetectPath}/>
        <label for="defDetPath">Detect Audio File</label>
      </div>
      <div class="form-floating">
        <input type="text" class="form-control col-3" id="defLaunPath" placeholder="/" bind:value={strDefLaunchPath}/>
        <label for="defLaunPath">Launch Audio File</label>
      </div>
      <div class="form-floating">
        <input type="text" class="form-control col-3" id="defRemPath" placeholder="/" bind:value={strDefRemovePath}/>
        <label for="defRemPath">Remove Audio File</label>
      </div>
      <div class="form-floating">
        <input type="text" class="form-control col-3" id="defErrPath" placeholder="/" bind:value={strDefErrorPath}/>
        <label for="defErrPath">Error Audio File</label>
      </div>
    </div>
  </div>
  <div class="col-12"> 
    <div class="input-group justify-content-md-center">
      <div class="input-group-text">
        <div class="form-floating">
          <input type="number" class="form-control col-4" id="defAudioGain" placeholder="0.4" bind:value={intAudioGain}/>
          <label for="defAudioGain">Audio Gain (0.4-4)</label>
        </div>
      </div>
    </div>
</div>
  <button type="submit" class="btn btn-primary mt-3">Save</button>
</form>