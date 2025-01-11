import { mount } from 'svelte'
import './app.css'
import 'bootstrap/dist/css/bootstrap.min.css';
import { library } from '@fortawesome/fontawesome-svg-core';
import { faHome, faUser, faMusic, faGear, faWrench, faSliders, faFolder} from '@fortawesome/free-solid-svg-icons';
import {faNfcSymbol} from '@fortawesome/free-brands-svg-icons';
import 'bootstrap/dist/js/bootstrap.bundle.min.js';
import App from './App.svelte'

library.add(faHome, faUser, faNfcSymbol, faMusic, faGear, faWrench, faSliders, faFolder);

const app = mount(App, {
  target: document.getElementById('app')!,
})

export default app
