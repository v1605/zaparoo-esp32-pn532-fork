<script lang="ts">
    import { FontAwesomeIcon } from '@fortawesome/svelte-fontawesome';
    import { UIDUtils } from "../backend/UIDUtils";
    import { EspUtils } from '../backend/EspUtils';
    import type { UIDExtdRecords, UIDExtdRecord } from "../types/ConfigData";
    
   
    let UIDRecord: UIDExtdRecord = UIDUtils.getBlank();

    UIDUtils.UIDRecord().subscribe(value=> UIDRecord = value);
    
    
</script>
<div>
    {JSON.stringify(UIDRecord)}
</div>