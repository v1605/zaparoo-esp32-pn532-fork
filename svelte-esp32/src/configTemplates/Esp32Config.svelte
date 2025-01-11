<script lang="ts">
    import { ConfigUtils } from "../backend/ConfigUtils";

     // Define variables for the input and toggle switch
  let misterAddress = '';
  let isMisterEnabled = false;
  let steamAddress = '';
  let isSteamEnabled = false;
  let isRRoREnabled = false;

  // Optional: Handle form submission
  const handleSubmit = (event: Event) => {
    event.preventDefault();
    ConfigUtils.updateConfig({
      wifi_led_enabled: boolean;
      motor_enabled: boolean;
      launch_led_enabled: boolean;
      audio_enabled: boolean;
      pwr_led_enabled: boolean;
      pin_I2S_DOUT: number;
      pin_I2S_BCLK: number;
      pin_I2S_LRC: number;
      val_AUDIO_GAIN: number;
      pin_MOTOR_PIN: number;
      pin_WIFI_LED_PIN: number;
      pin_LAUNCH_LED_PIN: number;
      pin_EXTERNAL_POWER_LED: number;
    });
    console.log('Mister Address:', misterAddress);
    console.log('Is Enabled:', isMisterEnabled);
  };
</script>
<h2>Zaparoo Settings</h2>
<form on:submit={handleSubmit} class="row g-2">
    <div class="input-group">
      <div class="input-group-text">
        <div class="form-check form-switch">
          <input class="form-check-input" type="checkbox" role="switch" id="enableMister" bind:checked={isMisterEnabled}/>
          <label class="form-check-label visually-hidden" for="enableMister">Enable Mister</label>
        </div>
      </div>
      <div class="form-floating">
        <input type="text" class="form-control" id="misterIp" placeholder="mister.local" bind:value={misterAddress} disabled={!isMisterEnabled} />
        <label for="misterIp">Mister Address</label>
      </div>
      <div class="input-group-text">
        <div class="form-check form-switch">
          <input class="form-check-input" type="checkbox" role="switch" id="enableRRoR" bind:checked={isRRoREnabled} disabled={!isMisterEnabled}/>
          <label class="form-check-label" for="enableRRoR">Reset On Remove</label>
        </div>
      </div>
    </div>
    <div class="input-group">
      <div class="input-group-text">
        <div class="form-check form-switch">
          <input class="form-check-input" type="checkbox" role="switch" id="enableSteam" bind:checked={isSteamEnabled}/>
          <label class="form-check-label visually-hidden" for="enableSteam">Enable SteamOS</label>
        </div>
      </div>
      <div class="form-floating">
        <input type="text" class="form-control" id="steamIp" placeholder="steam.local" bind:value={steamAddress} disabled={!isSteamEnabled} />
        <label for="misterIp">Steam Address</label>
      </div>
    </div>
    <button type="submit" class="btn btn-primary mt-3">Save</button>
</form>