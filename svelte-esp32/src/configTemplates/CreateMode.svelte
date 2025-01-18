<script lang="ts">
    import { FontAwesomeIcon } from '@fortawesome/svelte-fontawesome';
    import Create from './Create.svelte';
    import FileManager from './FileManager.svelte';
    import type { NavBarLink } from '../types/NavBarLink';
    import { zapUtils } from '../backend/zapUtils'
    import { onDestroy, onMount } from 'svelte';
    let activeLink: string = "nfc"; 
    const links: NavBarLink[] = [
      { name: 'NFC', id: 'nfc', icon: ['fab', 'nfc-symbol'] },
      { name: 'FILE MANAGER', id: 'fileMan', icon: 'folder' }
    ];
    const setActiveLink = (linkId: string): void => {
      activeLink = linkId;
    };
    onMount(() => {
      zapUtils.initConnections();
    });
</script>

<div class="h-100">
  <div class="text-center"> 
    <h2>NFC Create Mode</h2>
  </div>
  <ul class="nav nav-tabs">
    {#each links as { name, id, icon }}
      <li class="nav-item" >
        <a class="nav-link {activeLink === id ? 'active' : ''}" href="#"  on:click={() => setActiveLink(id)}>
          {name} <FontAwesomeIcon icon={icon}></FontAwesomeIcon>
        </a>
      </li>
    {/each}
  </ul>
  <div class="container pt-1 mt-1 h-100">
    <div class="row justify-content-center h-100">
        {#if activeLink === 'nfc'}
          <div class="col-md-12 col-lg-12">
            <Create></Create>
          </div>
        {/if}
        {#if activeLink === 'fileMan'}
          <div class="col-md-12 col-lg-12 h-100">
            <FileManager></FileManager>
          </div>
        {/if}
    </div>
  </div>
</div>
