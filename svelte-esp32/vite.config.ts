import { defineConfig, loadEnv } from 'vite'
import { svelte } from '@sveltejs/vite-plugin-svelte'
import { viteSingleFile } from "vite-plugin-singlefile"

function getBackendIP() {
  const env = loadEnv("all", process.cwd());
  const backendIP = env.VITE_ESP_IP || 'ws://localhost:8080';
  console.log('Configured WebSocket proxy target:', backendIP);
  return backendIP;
}

export default defineConfig({
  plugins: [svelte(), viteSingleFile()],
  server: {
    proxy: {
      // Proxy all requests starting with "/api" to the backend
      '/ws': {
        target: getBackendIP(), // Backend server URL
        changeOrigin: true, // Adjust the `Host` header to match the target,
      },
    },
  },
});