const CACHE_NAME = 'fractonica-v0.0.19';
const urlsToCache = [
    './index.html',
    './manifest.json',
    './icon-192x192.png',
    './icon-512x512.png',
    './main.js',
    './main.wasm'
];

let oldCacheDeleted = false;
// Install event: Caching static assets
self.addEventListener('install', (event) => {
    console.log('Service Worker: Installing cache ' + CACHE_NAME);
    event.waitUntil(
        caches.open(CACHE_NAME)
            .then((cache) => {
                console.log('Service Worker: Opened cache ' + CACHE_NAME);
                return cache.addAll(urlsToCache);
            })
            .then(() => {
                // Force the new service worker to activate immediately after installation
                // This skips the waiting phase. Be cautious with this in production if you
                // have very long-lived connections, as it might cut them off.
                // For development/debugging cache issues, it's very useful.
                console.log('Service Worker: Installation complete, skipping waiting.');
                return self.skipWaiting();
            })
            .catch(error => {
                console.error('Service Worker: Failed to cache during install:', error);
            })
    );
});

// Activate event: Cleaning up old caches
// ... (rest of your sw.js code) ...

// Activate event: Cleaning up old caches and triggering reload
self.addEventListener('activate', (event) => {
    console.log('Service Worker: Activating cache ' + CACHE_NAME);
    const cacheWhitelist = [CACHE_NAME];

    event.waitUntil(
        caches.keys().then((cacheNames) => {
            return Promise.all(
                cacheNames.map((cacheName) => {
                    if (!cacheWhitelist.includes(cacheName) && cacheName.startsWith('fractonica-')) {
                        console.log('Service Worker: Deleting old cache:', cacheName);
                        oldCacheDeleted = true;
                        return caches.delete(cacheName);
                    }
                })
            );
        })
            .then(() => {
                console.log('Service Worker: Old caches cleanup complete.');
                return self.clients.claim().then(() => {
                    if (oldCacheDeleted) {
                        console.log('Service Worker: Old cache(s) deleted. Sending message to clients for reload...');
                        // Send a message to all active clients (tabs/windows)
                        return self.clients.matchAll({ type: 'window' }).then(clients => {
                            clients.forEach(client => {
                                client.postMessage({ type: 'RELOAD_PAGE' });
                            });
                        });
                    } else {
                        console.log('Service Worker: No old caches found for deletion, no reload needed.');
                    }
                });
            })
            .catch(error => {
                console.error('Service Worker: Activation, cleanup, or message sending failed:', error);
            })
    );
});

// Fetch event: Serving assets from cache or network
self.addEventListener('fetch', (event) => {
    // IMPORTANT: Only process requests for HTTP/HTTPS resources
    if (!event.request.url.startsWith('http')) {
        // Let the browser handle non-http/https requests (e.g., chrome-extension://)
        return;
    }

    event.respondWith(
        caches.open(CACHE_NAME).then(cache => { // <--- Open the *current* cache first
            return cache.match(event.request).then(response => { // <--- Try to match in *current* cache
                // If resource is in the current cache, return it
                if (response) {
                    return response;
                }

                // If not in current cache, try network, and if successful, add to current cache
                const fetchRequest = event.request.clone();
                return fetch(fetchRequest).then(
                    function(response) {
                        // Check if we received a valid response AND it's from our origin
                        // (type 'basic' for same-origin requests)
                        if (!response || !response.ok || response.type !== 'basic') {
                            return response;
                        }

                        const responseToCache = response.clone();
                        cache.put(event.request, responseToCache) // <--- Put into *current* cache
                            .catch(e => console.error('Service Worker: Cache put failed:', e, event.request.url));

                        return response;
                    }
                ).catch(error => {
                    console.error('Service Worker: Fetch failed:', error, event.request.url);
                    // Optional: Return an offline fallback for critical assets like index.html
                    // if (event.request.mode === 'navigate') {
                    //   return caches.match('./offline.html');
                    // }
                    throw error; // Re-throw to propagate the error
                });
            });
        })
    );
});