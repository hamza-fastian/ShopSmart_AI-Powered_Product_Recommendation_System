// Shared app shell: session, navbar/footer, toasts, product card + detail modal.

// ---------- helpers ----------
function escapeHtml(s) {
  return String(s).replace(/[&<>"']/g, (c) => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' }[c]));
}
const money = (n) => Number(n).toLocaleString('en-PK', { maximumFractionDigits: 2 });
const priceHtml = (n) => `<span class="price"><small>Rs</small>${money(n)}</span>`;
const starsHtml = (r) => `<span class="stars">${icon('star')} ${Number(r).toFixed(1)}</span>`;
const qs = (sel, root = document) => root.querySelector(sel);
const qsa = (sel, root = document) => [...root.querySelectorAll(sel)];

// Product registry so image fallbacks can rebuild a tile from an id.
const PRODUCT_INDEX = {};
function indexProducts(list) { list.forEach((p) => (PRODUCT_INDEX[p.id] = p)); }
window.__tile = (id) => tileHtml(PRODUCT_INDEX[id] || { name: '', category: '' });

// ---------- session (customer ID is the backend's identity) ----------
const Session = {
  get() { try { return JSON.parse(localStorage.getItem('shopsmart.user')); } catch (e) { return null; } },
  set(c) { try { localStorage.setItem('shopsmart.user', JSON.stringify({ id: c.id, name: c.name })); } catch (e) {} },
  clear() { try { localStorage.removeItem('shopsmart.user'); } catch (e) {} },
};

// Verifies the saved session against the backend; clears it if the customer no longer exists.
async function currentCustomer() {
  const s = Session.get();
  if (!s) return null;
  try { return await Api.customer(s.id); }
  catch (e) { if (e.status === 404) Session.clear(); throw e; }
}

async function requireAuth() {
  if (!Session.get()) { location.replace('login.html?next=' + encodeURIComponent(location.pathname.split('/').pop() + location.search)); return null; }
  try { return await currentCustomer(); }
  catch (e) {
    if (e.status === 404) { location.replace('login.html'); return null; }
    throw e;
  }
}

function logout() { Session.clear(); location.href = 'index.html'; }

// ---------- toast ----------
function toast(msg, type = '') {
  let box = qs('.toasts');
  if (!box) { box = document.createElement('div'); box.className = 'toasts'; document.body.appendChild(box); }
  const t = document.createElement('div');
  t.className = 'toast ' + type;
  t.innerHTML = `${icon(type === 'error' ? 'alert' : 'check')}<span>${escapeHtml(msg)}</span>`;
  box.appendChild(t);
  setTimeout(() => { t.style.opacity = '0'; t.style.transition = 'opacity .3s'; setTimeout(() => t.remove(), 300); }, 3800);
}

// ---------- chrome ----------
const LOGO = `<a class="brand" href="index.html"><span class="brand-mark">${icon('sparkles')}</span><span>Shop<b>Smart</b></span></a>`;

function renderChrome() {
  const page = document.body.dataset.page;
  const user = Session.get();
  const link = (href, label, key) => `<a href="${href}" class="${page === key ? 'active' : ''}">${label}</a>`;
  const nav = qs('#site-nav');
  if (nav) {
    nav.outerHTML = `<header class="nav" id="nav"><div class="container nav-inner">
      ${LOGO}
      <nav class="nav-links">
        ${link('index.html', 'Home', 'home')}
        ${link('products.html', 'Products', 'products')}
        ${user ? link('dashboard.html', 'Dashboard', 'dashboard') + link('recommendations.html', 'For You', 'recs') + link('history.html', 'History', 'history') + link('preferences.html', 'Preferences', 'prefs') + link('profile.html', 'Profile', 'profile') : ''}
      </nav>
      <div class="nav-actions">
        ${user
          ? `<div class="user-chip"><span class="avatar">${escapeHtml(user.name.charAt(0).toUpperCase())}</span><div><strong>${escapeHtml(user.name)}</strong><small>Customer #${user.id}</small></div></div>
             <button class="btn btn-ghost btn-sm" id="logout-btn">${icon('logout')} Logout</button>`
          : `<a class="btn btn-ghost btn-sm" href="login.html">Log in</a><a class="btn btn-primary btn-sm" href="signup.html">Get started</a>`}
      </div>
      <button class="menu-btn" id="menu-btn" aria-label="Menu">${icon('menu')}</button>
    </div></header>`;
    const navEl = qs('#nav');
    qs('#menu-btn').onclick = () => navEl.classList.toggle('open');
    const lo = qs('#logout-btn'); if (lo) lo.onclick = logout;
  }
  const foot = qs('#site-footer');
  if (foot) {
    foot.outerHTML = `<footer class="footer"><div class="container">
      <div class="footer-grid">
        <div>${LOGO}<p>AI-powered personalized shopping, built on a C++ Data Structures engine: hash tables, trees and a max-heap ranking every recommendation.</p></div>
        <div><h5>Explore</h5><a href="products.html">All products</a><a href="recommendations.html">Recommendations</a><a href="dashboard.html">Dashboard</a></div>
        <div><h5>Account</h5><a href="profile.html">Profile</a><a href="preferences.html">Preferences</a><a href="history.html">Purchase history</a></div>
      </div>
      <div class="footer-bottom"><span>© ${new Date().getFullYear()} ShopSmart — university Data Structures project.</span><span>Frontend powered by the ShopSmart C++ API.</span></div>
    </div></footer>`;
  }
}

// ---------- reusable UI ----------
function skeletonCards(n = 8) {
  return Array.from({ length: n }, () => `<div class="card sk-card"><div class="skeleton sk-img"></div><div class="sk-body"><div class="skeleton" style="height:12px;width:40%"></div><div class="skeleton" style="height:18px;width:80%"></div><div class="skeleton" style="height:14px;width:55%"></div></div></div>`).join('');
}

function stateBlock({ iconName = 'alert', title, text, action }) {
  return `<div class="state fade-in"><div class="state-ico">${icon(iconName)}</div><h3>${escapeHtml(title)}</h3><p>${text}</p>${action || ''}</div>`;
}

function errorBlock(err, retryId = 'retry-btn') {
  return stateBlock({
    iconName: err.network ? 'server' : 'alert',
    title: err.network ? 'Backend not reachable' : 'Something went wrong',
    text: escapeHtml(err.message),
    action: `<button class="btn btn-primary" id="${retryId}">${icon('refresh')} Try again</button>`,
  });
}

function productCard(p, opt = {}) {
  const rec = opt.rec;
  const badge = rec ? `<span class="badge rec">${icon('sparkles')} #${rec.rank} for you</span>` : '';
  const stock = p.stock <= 0 ? `<span class="badge out">Out of stock</span>` : p.stock <= 5 ? `<span class="badge low">Only ${p.stock} left</span>` : '';
  return `<article class="card pcard" data-id="${p.id}" style="animation-delay:${Math.min((opt.i || 0) * 35, 350)}ms">
    <div class="thumb" data-open>${productImageHtml(p)}${badge}${stock}</div>
    <div class="pcard-body">
      <span class="pcard-cat">${escapeHtml(p.category)}</span>
      <h3 data-open>${escapeHtml(p.name)}</h3>
      <span class="pcard-brand">${escapeHtml(p.brand)}</span>
      <div class="pcard-meta">${priceHtml(p.price)}${starsHtml(p.rating)}</div>
      <div class="pcard-actions">
        <button class="btn btn-ghost btn-sm" data-open>Details</button>
        <button class="btn btn-soft btn-sm" data-open ${p.stock <= 0 ? 'disabled' : ''}>${icon('cart')} Buy</button>
      </div>
    </div>
  </article>`;
}

// Wire clicks on rendered cards/items inside `root` to open the detail modal.
function bindProductOpen(root, getProduct, getRec) {
  if (root.__productOpenBound) return; // pages re-render often; bind each root only once
  root.__productOpenBound = true;
  root.addEventListener('click', (e) => {
    const open = e.target.closest('[data-open]');
    if (!open) return;
    const holder = open.closest('[data-id]');
    if (!holder) return;
    const p = getProduct(Number(holder.dataset.id));
    if (p) openProductModal(p, { rec: getRec ? getRec(p.id) : null });
  });
}

function openProductModal(p, opt = {}) {
  const rec = opt.rec;
  const user = Session.get();
  const back = document.createElement('div');
  back.className = 'modal-back';
  const why = rec
    ? `<div class="why"><h4>${icon('sparkles')} Why ShopSmart picked this — score ${Number(rec.score).toFixed(1)}</h4><ul>${rec.breakdown.map((b) => `<li><span>${escapeHtml(b.reason)}</span><b>+${b.points}</b></li>`).join('')}</ul></div>`
    : '';
  back.innerHTML = `<div class="modal" role="dialog" aria-modal="true" aria-label="${escapeHtml(p.name)}">
    <button class="modal-close" aria-label="Close">${icon('x')}</button>
    <div class="thumb">${productImageHtml(p)}</div>
    <div class="modal-body">
      <span class="pcard-cat">${escapeHtml(p.category)}</span>
      <h2>${escapeHtml(p.name)}</h2>
      <div style="display:flex;gap:14px;align-items:center">${starsHtml(p.rating)}<span class="pcard-brand">by ${escapeHtml(p.brand)}</span></div>
      <div style="font-size:1.6rem">${priceHtml(p.price)}</div>
      <div class="facts">
        <div class="fact"><small>Rating</small><strong>${Number(p.rating).toFixed(1)} / 5</strong></div>
        <div class="fact"><small>Bought</small><strong>${p.purchaseCount} times</strong></div>
        <div class="fact"><small>In stock</small><strong>${p.stock > 0 ? p.stock + ' units' : 'Out of stock'}</strong></div>
        <div class="fact"><small>Product ID</small><strong>#${p.id}</strong></div>
      </div>
      ${why}
      <div id="modal-msg"></div>
      <div style="margin-top:auto;padding-top:8px">
        ${user
          ? `<button class="btn btn-primary btn-block" id="buy-btn" ${p.stock <= 0 ? 'disabled' : ''}>${icon('cart')} ${p.stock <= 0 ? 'Out of stock' : 'Buy now · Rs ' + money(p.price)}</button>`
          : `<a class="btn btn-primary btn-block" href="login.html">Log in to purchase</a>`}
      </div>
    </div>
  </div>`;
  document.body.appendChild(back);
  document.body.style.overflow = 'hidden';
  const close = () => { back.remove(); document.body.style.overflow = ''; document.removeEventListener('keydown', onKey); };
  const onKey = (e) => { if (e.key === 'Escape') close(); };
  document.addEventListener('keydown', onKey);
  back.addEventListener('click', (e) => { if (e.target === back) close(); });
  qs('.modal-close', back).onclick = close;

  const buy = qs('#buy-btn', back);
  if (buy) buy.onclick = async () => {
    buy.disabled = true; buy.innerHTML = '<span class="spinner"></span> Processing…';
    try {
      const r = await Api.purchase(user.id, p.id);
      Object.assign(p, r.product); // keep in-memory product in sync (stock, purchaseCount)
      _productsCache = null;
      toast(`Purchased ${p.name}. Remaining budget: Rs ${money(r.customer.budget)}`, 'success');
      window.dispatchEvent(new CustomEvent('shopsmart:purchased', { detail: r }));
      close();
    } catch (e) {
      qs('#modal-msg', back).innerHTML = `<div class="alert error">${icon('alert')}<span>${escapeHtml(e.message)}</span></div>`;
      buy.disabled = false; buy.innerHTML = `${icon('cart')} Buy now · Rs ${money(p.price)}`;
    }
  };
}

document.addEventListener('DOMContentLoaded', renderChrome);
