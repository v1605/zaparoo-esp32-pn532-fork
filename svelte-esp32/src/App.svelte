<script lang="ts">
    import { FontAwesomeIcon } from '@fortawesome/svelte-fontawesome';
    import ZaparooConfig from './configTemplates/ZaparooConfig.svelte';
    import Esp32Config from './configTemplates/Esp32Config.svelte';
    import Esp32Defaults from './configTemplates/Esp32Defaults.svelte';
    import CreateMode from './configTemplates/CreateMode.svelte';
    import UidMode from './configTemplates/UIDMode.svelte';
    import type { NavBarLink } from './types/NavBarLink';
    let isOpen: boolean = false;
    let activeLink: string = "zaparoo"; 
    const links: NavBarLink[] = [
      { name: 'CREATE', id: 'create', icon: ['fab', 'nfc-symbol'] },
      { name: 'UID CONTROL', id: 'uid-control', icon: 'music' },
      { name: 'ZAPAROO', id: 'zaparoo', icon: 'gear' },
      { name: 'ESP32', id: 'esp32', icon: 'wrench' },
      { name: 'DEFAULTS', id: 'defaults', icon: 'sliders' }
    ];
    const toggleNavbar = (): void => {
      isOpen = !isOpen;
    };
    const setActiveLink = (linkId: string): void => {
      activeLink = linkId;
    };
</script>

<main>
  <nav class="navbar navbar-expand-md navbar-dark bg-dark shadow-sm fixed-top">
    <a class="navbar-brand" href="/">Zap Esp32</a>
    <button class="navbar-toggler" type="button" data-bs-toggle="collapse" data-bs-target="#navbarNav" aria-controls="navbarNav" aria-expanded={isOpen ? "true" : "false"} aria-label="Toggle navigation" on:click={toggleNavbar}>
      <span class="navbar-toggler-icon"></span>
    </button>
    <div class="collapse navbar-collapse" id="navbarNav" class:show={isOpen}>
      <ul class="navbar-nav ms-auto">
        {#each links as { name, id, icon }}
          <li class="nav-item">
            <a class="nav-link {activeLink === id ? 'active' : ''}" href="#"  on:click={() => setActiveLink(id)}>
              {name} <FontAwesomeIcon icon={icon}></FontAwesomeIcon>
            </a>
          </li>
        {/each}
      </ul>
    </div>
  </nav>
  <div class="container pt-5 mt-5">
    <div class="row justify-content-center">
        {#if activeLink === 'zaparoo'}
          <div class="col-md-8 col-lg-6">
            <ZaparooConfig></ZaparooConfig>
          </div>
        {/if}
        {#if activeLink === 'esp32'}
          <div class="col-md-8 col-lg-6">
            <Esp32Config></Esp32Config>
          </div>
        {/if}
        {#if activeLink === 'defaults'}
          <div class="col-md-8 col-lg-8">  
            <Esp32Defaults></Esp32Defaults>
          </div>
        {/if}
        {#if activeLink === 'create'}
          <div class="col-md-8 col-lg-8">  
            <CreateMode></CreateMode>
          </div>
        {/if}
        {#if activeLink === 'uid-control'}
          <div class="col-md-8 col-lg-8">  
            <UidMode></UidMode>
          </div>
        {/if}
    </div>
  </div>
</main>
