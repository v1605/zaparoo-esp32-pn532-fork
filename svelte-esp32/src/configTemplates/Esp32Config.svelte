<script lang="ts">
    import { EspUtils } from "../backend/EspUtils";
    import type { ConfigData } from "../types/ConfigData";
    let config: ConfigData = EspUtils.getBlank();
    EspUtils.config().subscribe(value=> config = value);
    const handleSubmit = (event: Event) => {
      event.preventDefault();
      const update: Partial<ConfigData> = {
          wifi_led_enabled: config.wifi_led_enabled,
          motor_enabled: config.motor_enabled,
          launch_led_enabled: config.launch_led_enabled,
          audio_enabled: config.audio_enabled,
          pwr_led_enabled: config.pwr_led_enabled,
          pin_I2S_DOUT: config.pin_I2S_DOUT,
          pin_I2S_BCLK: config.pin_I2S_BCLK,
          pin_I2S_LRC: config.pin_I2S_LRC,
          pin_MOTOR_PIN: config.pin_MOTOR_PIN,
          pin_WIFI_LED_PIN: config.pin_WIFI_LED_PIN,
          pin_LAUNCH_LED_PIN: config.pin_LAUNCH_LED_PIN,
          pin_EXTERNAL_POWER_LED: config.pin_EXTERNAL_POWER_LED,
          sdCard_enabled: config.sdCard_enabled
      };
      EspUtils.updateConfig(update);
    };
</script>
<div class="text-center"> 
  <h2>ESP32 Settings</h2>
</div>
<form on:submit={handleSubmit} class="row g-4">
  <div class="col-6"> 
    <div class="input-group">
        <div class="input-group-text col-8">
          <div class="form-check form-switch">
            <input class="form-check-input" type="checkbox" role="switch" id="enableWiFiLED" bind:checked={config.wifi_led_enabled}/>
            <label class="form-check-label" for="enableWiFiLED">WiFi LED</label>
          </div>
        </div>
        <div class="form-floating">
          <input type="number" class="form-control col-4" id="pinWifiLED" placeholder="2" bind:value={config.pin_WIFI_LED_PIN} disabled={!config.wifi_led_enabled} />
          <label for="pinWifiLED">GPIO PIN</label>
        </div>
    </div>
  </div>
  <div class="col-6"> 
      <div class="input-group">
        <div class="input-group-text col-8">
          <div class="form-check form-switch">
            <input class="form-check-input" type="checkbox" role="switch" id="enableLaunchLED" bind:checked={config.launch_led_enabled}/>
            <label class="form-check-label" for="enableLaunchLED">Launch LED</label>
          </div>
        </div>
        <div class="form-floating">
          <input type="number" class="form-control col-4" id="pinLaunchLED" placeholder="33" bind:value={config.pin_LAUNCH_LED_PIN} disabled={!config.launch_led_enabled} />
          <label for="pinLaunchLED">GPIO PIN</label>
        </div>
      </div>
  </div>
  <div class="col-6"> 
    <div class="input-group">
        <div class="input-group-text col-8">
          <div class="form-check form-switch">
            <input class="form-check-input" type="checkbox" role="switch" id="enablePwrLED" bind:checked={config.pwr_led_enabled}/>
            <label class="form-check-label" for="enablePwrLED">Power LED</label>
          </div>
        </div>
        <div class="form-floating">
          <input type="number" class="form-control col-4" id="pinPwrLED" placeholder="15" bind:value={config.pin_EXTERNAL_POWER_LED} disabled={!config.pwr_led_enabled} />
          <label for="pinPwrLED">GPIO PIN</label>
        </div>
    </div>
  </div>
  <div class="col-6"> 
      <div class="input-group">
        <div class="input-group-text col-8">
          <div class="form-check form-switch">
            <input class="form-check-input" type="checkbox" role="switch" id="enableRumbleLED" bind:checked={config.motor_enabled}/>
            <label class="form-check-label" for="enableRumbleLED">Rumble Motor</label>
          </div>
        </div>
        <div class="form-floating">
          <input type="number" class="form-control col-md-4" id="pinMotor" placeholder="32" bind:value={config.pin_MOTOR_PIN} disabled={!config.motor_enabled} />
          <label for="pinMotor">GPIO PIN</label>
        </div>
      </div>
  </div>
  <div class="col-12"> 
    <div class="input-group">
      <div class="input-group-text col-4">
        <div class="form-check form-switch">
          <input class="form-check-input" type="checkbox" role="switch" id="enableAudio" bind:checked={config.audio_enabled}/>
          <label class="form-check-label" for="enableAudio">Audio Module</label>
        </div>
      </div>
      <div class="form-floating">
        <input type="number" class="form-control col-2" id="pinI2SDOUT" placeholder="25" bind:value={config.pin_I2S_DOUT} disabled={!config.audio_enabled} />
        <label for="pinI2SDOUT">GPIO I2SDOUT</label>
      </div>
      <div class="form-floating">
        <input type="number" class="form-control col-2" id="pinI2SBCLK" placeholder="27" bind:value={config.pin_I2S_BCLK} disabled={!config.audio_enabled} />
        <label for="pinI2SBCLK">GPIO I2SBCLK</label>
      </div>
      <div class="form-floating">
        <input type="number" class="form-control col--2" id="pinI2SLRC" placeholder="26" bind:value={config.pin_I2S_LRC} disabled={!config.audio_enabled} />
        <label for="pinI2SLRC">GPIO I2SLRC</label>
      </div>
    </div>
  </div>
  <div class="col-12"> 
      <div class="input-group justify-content-md-center">
        <div class="input-group-text">
          <div class="form-check form-switch">
            <input class="form-check-input" type="checkbox" role="switch" id="enableSDCard" bind:checked={config.sdCard_enabled}/>
            <label class="form-check-label" for="enableSDCard">SD Card Module</label>
          </div>
        </div>
      </div>
  </div>
  <button type="submit" class="btn btn-primary mt-3">Save</button>
</form>