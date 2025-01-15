<script lang="ts">
    import { UIDUtils } from "../backend/UIDUtils";
    import type { UIDExtdRecord } from "../types/ConfigData";
    let UIDRecord: UIDExtdRecord = UIDUtils.getBlank();
    UIDUtils.UIDRecord().subscribe(value=> UIDRecord = value);
    const handleSubmit = (event: Event) => {
        event.preventDefault();
        const updRec = UIDUtils.getBlank();
        updRec.UID = UIDRecord.UID;
        updRec.launchAudio = UIDRecord.launchAudio;
        updRec.removeAudio = UIDRecord.removeAudio;
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
                <input type="text" class="form-control" id="tokenUID" placeholder="/" bind:value={UIDRecord.UID}/>
                <label for="tokenUID">Token UID</label>
            </div>
            <div class="form-floating col-12 col-md-3">
                <input type="text" class="form-control" id="LaunPath" placeholder="/" bind:value={UIDRecord.launchAudio}/>
                <label for="LaunPath">Launch Audio File</label>
            </div>
            <div class="form-floating col-12 col-md-3">
                <input type="text" class="form-control" id="RemPath" placeholder="/" bind:value={UIDRecord.removeAudio}/>
                <label for="RemPath">Remove Audio File</label>
            </div>
        </div>
    </div>
    <button type="submit" class="btn btn-primary mt-3">Save</button>
</form>