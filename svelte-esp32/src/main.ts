import { mount } from 'svelte'
import './app.css'
import 'bootstrap/dist/css/bootstrap.min.css';
import { library } from '@fortawesome/fontawesome-svg-core';
import { faHome, faUser } from '@fortawesome/free-solid-svg-icons';
import { FontAwesomeIcon } from '@fortawesome/svelte-fontawesome';
import 'bootstrap/dist/js/bootstrap.bundle.min.js';
import App from './App.svelte'

library.add(faHome, faUser);

const app = mount(App, {
  target: document.getElementById('app')!,
})

export default app
