<script lang="ts">
    import { FontAwesomeIcon } from '@fortawesome/svelte-fontawesome';
    import Create from './Create.svelte';
    import FileManager from './FileManager.svelte';
    import type { NavBarLink } from '../types/NavBarLink';
    
    let isOpen: boolean = false;
    let activeLink: string = "nfc"; 
    const links: NavBarLink[] = [
      { name: 'NFC', id: 'nfc', icon: ['fab', 'nfc-symbol'] },
      { name: 'FILE MANAGER', id: 'fileMan', icon: 'folder' }
    ];
    // const toggleNavbar = (): void => {
    //   isOpen = !isOpen;
    // };
    const setActiveLink = (linkId: string): void => {
      activeLink = linkId;
    };
</script>

<div>
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
  <div class="container pt-5 mt-5">
    <div class="row justify-content-center">
        {#if activeLink === 'nfc'}
          <div class="col-md-8 col-lg-6">
            <Create></Create>
          </div>
        {/if}
        {#if activeLink === 'fileMan'}
          <div class="col-md-8 col-lg-6">
            <FileManager></FileManager>
          </div>
        {/if}
    </div>
  </div>
</div>
