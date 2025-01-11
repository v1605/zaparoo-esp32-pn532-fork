<script lang="ts">
    import { FontAwesomeIcon } from '@fortawesome/svelte-fontawesome';
    import ZaparooConfig from './configTemplates/ZaparooConfig.svelte';
    import type { NavBarLink } from './types/NavBarLink';
    let isOpen: boolean = false;
    let activeLink: string = "zaparoo"; 
    const links: NavBarLink[] = [
      { name: 'CREATE', id: 'create', icon: ['fab', 'nfc-symbol'] },
      { name: 'UID CONTROL', id: 'uid-control', icon: 'music' },
      { name: 'ZAPAROO', id: 'zaparoo', icon: 'gear' },
      { name: 'ESP32', id: 'esp32', icon: 'wrench' },
      { name: 'DEFAULTS', id: 'defaults', icon: 'sliders' },
      { name: 'FILES', id: 'files', icon: 'folder' },
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
      <div class="col-md-8 col-lg-6">
        {#if activeLink === 'zaparoo'}
          <ZaparooConfig></ZaparooConfig>
        {/if}
      </div>
    </div>
  </div>
</main>
