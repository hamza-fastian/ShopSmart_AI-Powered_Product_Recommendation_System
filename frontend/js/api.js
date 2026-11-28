// Centralized backend configuration + API client.
// The C++ server (server.cpp) serves this frontend AND the API on the same
// port, so by default requests are same-origin. If you open the frontend some
// other way (e.g. VS Code Live Server), requests go to BACKEND_URL instead.
const BACKEND_URL = 'http://localhost:8080';

// Resolved once per page load: same origin if the page is served by the ShopSmart
// server (it answers /api/health), otherwise BACKEND_URL. You can also force a
// server with  ?api=http://host:port  (remembered in localStorage).
let API_BASE = null;
let _baseReady = null;
function resolveApiBase() {
  if (_baseReady) return _baseReady;
  _baseReady = (async () => {
    const override = new URLSearchParams(location.search).get('api');
    if (override) { try { localStorage.setItem('shopsmart.api', override); } catch (e) {} }
    let saved = null;
    try { saved = localStorage.getItem('shopsmart.api'); } catch (e) {}
    if (override || saved) return (override || saved).replace(/\/$/, '');
    if (location.protocol.startsWith('http')) {
      try {
        const r = await fetch('/api/health');
        if (r.ok && (await r.json()).status === 'ok') return '';
      } catch (e) { /* not served by ShopSmart -> use BACKEND_URL */ }
    }
    return BACKEND_URL;
  })().then((b) => { API_BASE = b; return b; });
  return _baseReady;
}

class ApiError extends Error {
  constructor(message, status, network) { super(message); this.status = status; this.network = !!network; }
}

async function request(path, options = {}) {
  const base = await resolveApiBase();
  const controller = new AbortController();
  const timer = setTimeout(() => controller.abort(), options.timeout || 10000);
  let res;
  try {
    res = await fetch(base + path, {
      method: options.method || 'GET',
      headers: options.body ? { 'Content-Type': 'application/json' } : undefined,
      body: options.body ? JSON.stringify(options.body) : undefined,
      signal: controller.signal,
    });
  } catch (e) {
    throw new ApiError(
      'Cannot reach the ShopSmart server. Make sure it is running (see frontend/README.md), then try again.',
      0, true);
  } finally { clearTimeout(timer); }

  let data = null;
  try { data = await res.json(); } catch (e) { /* non-JSON response */ }
  if (!res.ok) throw new ApiError((data && data.error) || `Request failed (${res.status})`, res.status);
  return data;
}

const Api = {
  health: () => request('/api/health'),
  products: () => request('/api/products'),
  product: (id) => request(`/api/products/${id}`),
  login: (id) => request('/api/login', { method: 'POST', body: { id } }),
  register: (payload) => request('/api/register', { method: 'POST', body: payload }),
  customer: (id) => request(`/api/customers/${id}`),
  recommendations: (id, k = 10) => request(`/api/recommendations/${id}?k=${k}`),
  savePreferences: (id, payload) => request(`/api/customers/${id}/preferences`, { method: 'POST', body: payload }),
  purchase: (customerId, productId) =>
    request(`/api/customers/${customerId}/purchase`, { method: 'POST', body: { productId } }),
};

// Products rarely change during a visit; cache them per page load.
let _productsCache = null;
Api.cachedProducts = async (force) => {
  if (!_productsCache || force) {
    _productsCache = await Api.products();
    // Photos are optional: any failure here just means tiles are shown.
    try { Object.assign(PRODUCT_PHOTOS, await request('/api/product-images')); } catch (e) { /* ignore */ }
  }
  return _productsCache;
};
