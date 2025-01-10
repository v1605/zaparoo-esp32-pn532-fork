<script lang="ts">
  import { FontAwesomeIcon } from '@fortawesome/svelte-fontawesome';
    import ZaparooConfig from './configTemplates/ZaparooConfig.svelte';
  let isOpen: boolean = false;

  const toggleNavbar = (): void => {
    isOpen = !isOpen;
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
        <li class="nav-item">
          <a class="nav-link active" href="#">Home</a>
        </li>
        <li class="nav-item">
          <a class="nav-link" href="#">About</a>
        </li>
        <li class="nav-item">
          <a class="nav-link" href="#">Contact</a>
        </li>
      </ul>
    </div>
  </nav>
  <div class="container pt-5 mt-5">
    <div class="row justify-content-center">
      <div class="col-md-8 col-lg-6">
        <ZaparooConfig></ZaparooConfig>
        <!--<div class="text-center mb-4">
          <h1 class="display-4 text-primary">Welcome to the Svelte ESP32 Web App</h1>
          <p class="lead text-muted">A modern web interface powered by Svelte and Bootstrap</p>
        </div>
        <button class="btn btn-primary btn-lg btn-block">Press me <FontAwesomeIcon icon="home" /> </button>-->
      </div>
    </div>
  </div>
</main>
