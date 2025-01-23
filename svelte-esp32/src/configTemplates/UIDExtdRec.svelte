<script lang="ts">
    import { UIDUtils } from "../backend/UIDUtils";
    import type { UIDExtdRecord } from "../types/ConfigData";
    import { CommonUtils } from "../backend/CommonUtils";
    let uidRecord: UIDExtdRecord = UIDUtils.getBlank();
    UIDUtils.UIDRecord().subscribe(value=> uidRecord = value);
    const handleSubmit = (event: Event) => {
        event.preventDefault();
        const updRec = UIDUtils.getBlank();
        updRec.UID = uidRecord.UID;
        updRec.launchAudio = CommonUtils.validateAudioPath(uidRecord.launchAudio);
        updRec.removeAudio = CommonUtils.validateAudioPath(uidRecord.removeAudio);
        UIDUtils.updateUIDRecord(updRec);
    }
    
</script>
<div class="text-center mt-2">
    <h4>Scan an item to update audio controls</h4>
</div>
<form on:submit={handleSubmit} class="row g-1 mt-5">
    <div class="col-12">
        <div class="d-flex flex-column flex-md-row align-items-center justify-content-between">
            <div class="form-floating col-12 col-md-3">
                <input type="text" class="form-control" id="tokenUID" placeholder="/" bind:value={uidRecord.UID}/>
                <label for="tokenUID">Token UID</label>
            </div>
            <div class="form-floating col-12 col-md-3">
                <input type="text" class="form-control" id="LaunPath" placeholder="/" bind:value={uidRecord.launchAudio}/>
                <label for="LaunPath">Launch Audio File</label>
            </div>
            <div class="form-floating col-12 col-md-3">
                <input type="text" class="form-control" id="RemPath" placeholder="/" bind:value={uidRecord.removeAudio}/>
                <label for="RemPath">Remove Audio File</label>
            </div>
        </div>
    </div>
    <button type="submit" class="btn btn-primary mt-3">Save</button>
</form>