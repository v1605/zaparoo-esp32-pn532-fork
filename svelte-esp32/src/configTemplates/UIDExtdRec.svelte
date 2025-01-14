<script lang="ts">
    import { FontAwesomeIcon } from '@fortawesome/svelte-fontawesome';
    import { UIDUtils } from "../backend/UIDUtils";
    import { EspUtils } from '../backend/EspUtils';
    import type { UIDExtdRecords, UIDExtdRecord } from "../types/ConfigData";
    import { onDestroy } from 'svelte';
    EspUtils.toggleUIDMode();
    let UIDRecords: UIDExtdRecords = UIDUtils.getBlank();

    UIDUtils.UIDRecords().subscribe(value=> UIDRecords = value);
    
    onDestroy(() => {
        EspUtils.toggleUIDMode();
    });
</script>
<div>
    
</div>