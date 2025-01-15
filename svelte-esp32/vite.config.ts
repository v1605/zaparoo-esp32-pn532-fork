import { defineConfig, loadEnv } from 'vite'
import { svelte } from '@sveltejs/vite-plugin-svelte'
import { viteSingleFile } from "vite-plugin-singlefile"

function getBackendWs(){
  const backendIP = "ws://" + getBackendIP();
  console.log('Configured WebSocket proxy target:', backendIP);
  return backendIP;
}

function getBackendHttp(){
  const backendIP = "http://" + getBackendIP();
  console.log('Configured Http proxy target:', backendIP);
  return backendIP;
}

function getBackendIP() {
  const env = loadEnv("all", process.cwd());
  const backendIP = env.VITE_ESP_IP || 'localhost:8080';
  return backendIP;
}

export default defineConfig({
  plugins: [svelte(), viteSingleFile()],
  server: {
    proxy: {
      '/ws': {
        target: getBackendWs(), // Backend server URL
        changeOrigin: true, // Adjust the `Host` header to match the target,
      },
      '/file': {
        target: getBackendHttp(), // Backend server URL
        changeOrigin: true, // Adjust the `Host` header to match the target,
      },
      '/get-folder-contents': {
        target: getBackendHttp(), // Backend server URL
        changeOrigin: true, // Adjust the `Host` header to match the target,
      },
    },
  },
});
