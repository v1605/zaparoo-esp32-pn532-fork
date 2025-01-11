<script lang="ts">
    import { ConfigUtils } from "../backend/ConfigUtils";

     // Define variables for the input and toggle switch
  let isWifiLedEnabled = false;
  let isMotorEnabled = false;
  let isLaunchLedEnabled = false;
  let isAudioEnabled = false;
  let isPwrLedEnabled = false;
  let intPinI2SDOUT:number|null = 25;
  let intPinI2SBCLK:number|null = 27;
  let intPinI2SLRC:number|null = 26;
  let intPinMotor:number|null = 32;
  let intPinWifiLed:number|null = 2;
  let intPinLaunchLed:number|null = 33;
  let intPinExtPwrLed:number|null = 15;


  // Optional: Handle form submission
  const handleSubmit = (event: Event) => {
    event.preventDefault();
    ConfigUtils.updateConfig({
      wifi_led_enabled: isWifiLedEnabled,
      motor_enabled: isMotorEnabled,
      launch_led_enabled: isLaunchLedEnabled,
      audio_enabled: isAudioEnabled,
      pwr_led_enabled: isPwrLedEnabled,
      pin_I2S_DOUT: intPinI2SDOUT,
      pin_I2S_BCLK: intPinI2SBCLK,
      pin_I2S_LRC: intPinI2SLRC,
      pin_MOTOR_PIN: intPinMotor,
      pin_WIFI_LED_PIN: intPinWifiLed,
      pin_LAUNCH_LED_PIN: intPinLaunchLed,
      pin_EXTERNAL_POWER_LED: intPinExtPwrLed
    });
  };
</script>
<h2>ESP32 Settings</h2>
<form on:submit={handleSubmit} class="row g-4">
  <div class="col-md-6"> 
    <div class="input-group">
        <div class="input-group-text col-md-8">
          <div class="form-check form-switch">
            <input class="form-check-input" type="checkbox" role="switch" id="enableWiFiLED" bind:checked={isWifiLedEnabled}/>
            <label class="form-check-label" for="enableWiFiLED">WiFi LED</label>
          </div>
        </div>
        <div class="form-floating">
          <input type="number" class="form-control col-md-4" id="pinWifiLED" placeholder="2" bind:value={intPinWifiLed} disabled={!isWifiLedEnabled} />
          <label for="pinWifiLED">GPIO PIN</label>
        </div>
    </div>
  </div>
  <div class="col-md-6"> 
      <div class="input-group">
        <div class="input-group-text col-md-8">
          <div class="form-check form-switch">
            <input class="form-check-input" type="checkbox" role="switch" id="enableLaunchLED" bind:checked={isLaunchLedEnabled}/>
            <label class="form-check-label" for="enableLaunchLED">Launch LED</label>
          </div>
        </div>
        <div class="form-floating">
          <input type="number" class="form-control col-md-4" id="pinLaunchLED" placeholder="33" bind:value={intPinLaunchLed} disabled={!isLaunchLedEnabled} />
          <label for="pinLaunchLED">GPIO PIN</label>
        </div>
      </div>
  </div>
  <div class="col-md-6"> 
    <div class="input-group">
        <div class="input-group-text col-md-8">
          <div class="form-check form-switch">
            <input class="form-check-input" type="checkbox" role="switch" id="enablePwrLED" bind:checked={isPwrLedEnabled}/>
            <label class="form-check-label" for="enablePwrLED">Power LED</label>
          </div>
        </div>
        <div class="form-floating">
          <input type="number" class="form-control col-md-4" id="pinPwrLED" placeholder="15" bind:value={intPinExtPwrLed} disabled={!isPwrLedEnabled} />
          <label for="pinPwrLED">GPIO PIN</label>
        </div>
    </div>
  </div>
  <div class="col-md-6"> 
      <div class="input-group">
        <div class="input-group-text col-md-8">
          <div class="form-check form-switch">
            <input class="form-check-input" type="checkbox" role="switch" id="enableRumbleLED" bind:checked={isMotorEnabled}/>
            <label class="form-check-label" for="enableRumbleLED">Rumble Motor</label>
          </div>
        </div>
        <div class="form-floating">
          <input type="number" class="form-control col-md-4" id="pinMotor" placeholder="32" bind:value={intPinMotor} disabled={!isMotorEnabled} />
          <label for="pinMotor">GPIO PIN</label>
        </div>
      </div>
  </div>
  <div class="col-md-12"> 
    <div class="input-group">
      <div class="input-group-text col-md-4">
        <div class="form-check form-switch">
          <input class="form-check-input" type="checkbox" role="switch" id="enableAudio" bind:checked={isAudioEnabled}/>
          <label class="form-check-label" for="enableAudio">Audio Module</label>
        </div>
      </div>
      <div class="form-floating">
        <input type="number" class="form-control col-md-2" id="pinI2SDOUT" placeholder="25" bind:value={intPinI2SDOUT} disabled={!isAudioEnabled} />
        <label for="pinI2SDOUT">GPIO I2SDOUT</label>
      </div>
      <div class="form-floating">
        <input type="number" class="form-control col-md-2" id="pinI2SBCLK" placeholder="27" bind:value={intPinI2SBCLK} disabled={!isAudioEnabled} />
        <label for="pinI2SBCLK">GPIO I2SBCLK</label>
      </div>
      <div class="form-floating">
        <input type="number" class="form-control col-md-2" id="pinI2SLRC" placeholder="26" bind:value={intPinI2SLRC} disabled={!isAudioEnabled} />
        <label for="pinI2SLRC">GPIO I2SLRC</label>
      </div>
    </div>
</div>
  <button type="submit" class="btn btn-primary mt-3">Save</button>
</form>