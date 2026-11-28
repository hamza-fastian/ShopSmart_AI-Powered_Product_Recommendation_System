// Product catalog: real products from the backend, filtered/sorted in the browser.
let ALL = [];
let recMap = {};       // productId -> recommendation (only when logged in)
const state = { q: '', category: '', sort: 'featured' };

const SORTS = {
  featured: null,
  'price-asc': (a, b) => a.price - b.price,
  'price-desc': (a, b) => b.price - a.price,
  name: (a, b) => a.name.localeCompare(b.name),
  rating: (a, b) => b.rating - a.rating || b.purchaseCount - a.purchaseCount,
  popular: (a, b) => b.purchaseCount - a.purchaseCount,
  recommended: (a, b) => (recMap[a.id] ? recMap[a.id].rank : 1e9) - (recMap[b.id] ? recMap[b.id].rank : 1e9),
};

async function initProducts() {
  const grid = qs('#grid');
  grid.innerHTML = skeletonCards(12);
  try {
    ALL = await Api.cachedProducts(true);
    indexProducts(ALL);
    const user = Session.get();
    recMap = {};
    if (user) {
      try { (await Api.recommendations(user.id, 200)).forEach((r) => (recMap[r.product.id] = r)); }
      catch (e) { if (e.status === 404) Session.clear(); /* recommendations are optional on this page */ }
    }
  } catch (e) {
    qs('#count').textContent = '';
    grid.innerHTML = `<div style="grid-column:1/-1">${errorBlock(e)}</div>`;
    qs('#retry-btn').onclick = initProducts;
    return;
  }

  // controls built from the real data
  const cats = [...new Set(ALL.map((p) => p.category))].sort();
  qs('#category').innerHTML = '<option value="">All categories</option>' + cats.map((c) => `<option>${escapeHtml(c)}</option>`).join('');
  if (Object.keys(recMap).length && !qs('#sort option[value="recommended"]')) {
    qs('#sort').insertAdjacentHTML('afterbegin', '<option value="recommended">Recommended for you</option>');
  }

  // initial state from URL (?q=&category=&sort=)
  const params = new URLSearchParams(location.search);
  state.q = params.get('q') || '';
  state.category = cats.includes(params.get('category')) ? params.get('category') : '';
  const wanted = params.get('sort');
  state.sort = wanted && SORTS.hasOwnProperty(wanted) && (wanted !== 'recommended' || Object.keys(recMap).length) ? wanted : (Object.keys(recMap).length ? 'recommended' : 'featured');
  qs('#q').value = state.q; qs('#category').value = state.category; qs('#sort').value = state.sort;
  render();
}

function render() {
  const grid = qs('#grid');
  const term = state.q.toLowerCase();
  let list = ALL.filter((p) =>
    (!state.category || p.category === state.category) &&
    (!term || p.name.toLowerCase().includes(term) || p.brand.toLowerCase().includes(term)));
  const cmp = SORTS[state.sort];
  if (cmp) list = [...list].sort(cmp);

  // category pills reflect the current selection
  const cats = [...new Set(ALL.map((p) => p.category))].sort();
  qs('#pills').innerHTML = ['', ...cats].map((c) => `<span class="chip ${state.category === c ? 'on' : ''}" data-cat="${escapeHtml(c)}">${c ? escapeHtml(c) : 'All'}</span>`).join('');

  qs('#count').innerHTML = `<strong>${list.length}</strong> of ${ALL.length} products${state.category ? ' in ' + escapeHtml(state.category) : ''}${state.q ? ` matching “${escapeHtml(state.q)}”` : ''}`;

  if (!ALL.length) {
    grid.innerHTML = `<div style="grid-column:1/-1">${stateBlock({ iconName: 'box', title: 'The catalog is empty', text: 'No products were loaded. Check data/products.csv.' })}</div>`;
    return;
  }
  if (!list.length) {
    grid.innerHTML = `<div style="grid-column:1/-1">${stateBlock({ iconName: 'search', title: 'No products found', text: 'Try a different search term or clear your filters.', action: '<button class="btn btn-primary" id="empty-clear">Clear filters</button>' })}</div>`;
    qs('#empty-clear').onclick = clearFilters;
    return;
  }
  grid.innerHTML = list.map((p, i) => productCard(p, { rec: recMap[p.id] && recMap[p.id].rank <= 10 ? recMap[p.id] : null, i })).join('');
}

function syncUrl() {
  const p = new URLSearchParams();
  if (state.q) p.set('q', state.q);
  if (state.category) p.set('category', state.category);
  if (state.sort !== 'featured') p.set('sort', state.sort);
  history.replaceState(null, '', location.pathname + (p.toString() ? '?' + p : ''));
}
function clearFilters() {
  state.q = ''; state.category = ''; state.sort = Object.keys(recMap).length ? 'recommended' : 'featured';
  qs('#q').value = ''; qs('#category').value = ''; qs('#sort').value = state.sort;
  syncUrl(); render();
}

document.addEventListener('DOMContentLoaded', () => {
  let t;
  qs('#q').addEventListener('input', (e) => { clearTimeout(t); t = setTimeout(() => { state.q = e.target.value.trim(); syncUrl(); render(); }, 150); });
  qs('#category').addEventListener('change', (e) => { state.category = e.target.value; syncUrl(); render(); });
  qs('#sort').addEventListener('change', (e) => { state.sort = e.target.value; syncUrl(); render(); });
  qs('#clear').addEventListener('click', clearFilters);
  qs('#pills').addEventListener('click', (e) => {
    const chip = e.target.closest('.chip'); if (!chip) return;
    state.category = chip.dataset.cat; qs('#category').value = state.category; syncUrl(); render();
  });
  bindProductOpen(qs('#grid'), (id) => PRODUCT_INDEX[id], (id) => recMap[id]);
  initProducts();
});

// stock and ranking change after a purchase
window.addEventListener('shopsmart:purchased', initProducts);
