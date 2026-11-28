// Dashboard: customer, recommendations and purchases all come from the C++ backend.
let dashRecMap = {};

async function initDashboard() {
  const root = qs('#dash');
  let customer;
  try { customer = await requireAuth(); }
  catch (e) { root.innerHTML = errorBlock(e); qs('#retry-btn').onclick = initDashboard; return; }
  if (!customer) return;

  root.innerHTML = `<div class="skeleton" style="height:170px;border-radius:26px"></div><div class="grid product-grid dash-section">${skeletonCards(4)}</div>`;

  let products, recs;
  try {
    [products, recs] = await Promise.all([Api.cachedProducts(true), Api.recommendations(customer.id, 8)]);
  } catch (e) { root.innerHTML = errorBlock(e); qs('#retry-btn').onclick = initDashboard; return; }
  indexProducts(products);
  dashRecMap = {}; recs.forEach((r) => (dashRecMap[r.product.id] = r));

  const cats = {};
  products.forEach((p) => (cats[p.category] = (cats[p.category] || 0) + 1));
  const bought = customer.purchaseHistory.map((id) => PRODUCT_INDEX[id]).filter(Boolean).reverse();
  const hasPrefs = customer.preferredCategories.length || customer.preferredBrands.length;
  const first = customer.name.split(' ')[0];

  root.innerHTML = `
    <section class="welcome fade-in">
      <div>
        <span class="pill" style="background:rgba(255,255,255,.18);color:#fff;border-color:rgba(255,255,255,.3)"><i style="background:#fff;color:var(--primary)">${icon('sparkles')}</i> ShopSmart — AI-Powered Product Recommendation System</span>
        <h1 style="margin-top:16px">Welcome back, ${escapeHtml(first)} 👋</h1>
        <p>${recs.length ? `We've ranked ${products.length} products for you. Your best match is <b>${escapeHtml(recs[0].product.name)}</b> with a score of ${recs[0].score.toFixed(1)}.` : 'No recommendations available right now.'}</p>
      </div>
      <div class="welcome-actions">
        <a class="btn btn-light" href="recommendations.html">${icon('sparkles')} See all recommendations</a>
        <a class="btn btn-ghost" href="products.html">Browse catalog</a>
      </div>
    </section>

    <form class="dash-search" id="dash-search"><input class="input" id="dash-q" placeholder="Search products by name or brand…" aria-label="Search products"><button class="btn btn-primary" type="submit">${icon('search')} Search</button></form>

    <div class="grid stat-grid">
      <div class="card stat"><span class="cat-ico">${icon('wallet')}</span><div><small>Remaining budget</small><strong id="stat-budget">Rs ${money(customer.budget)}</strong></div></div>
      <div class="card stat"><span class="cat-ico">${icon('receipt')}</span><div><small>Products purchased</small><strong>${customer.purchaseHistory.length}</strong></div></div>
      <div class="card stat"><span class="cat-ico">${icon('layers')}</span><div><small>Products in catalog</small><strong>${products.length}</strong></div></div>
      <div class="card stat"><span class="cat-ico">${icon('tag')}</span><div><small>Preferred categories</small><strong>${customer.preferredCategories.length || '—'}</strong></div></div>
    </div>

    <section class="dash-section">
      <div class="section-head"><div><span class="eyebrow">Recommended for you</span><h2>Your top picks</h2><p>Ranked by the ShopSmart max-heap. Open any product to see the points behind its score.</p></div><a class="btn btn-ghost btn-sm" href="recommendations.html">View all ${icon('arrow')}</a></div>
      ${hasPrefs ? '' : `<div class="alert info" style="margin-bottom:18px">${icon('alert')}<span>You haven't saved any preferred categories or brands yet, so scores currently rely on your budget, product ratings and popularity.</span></div>`}
      <div class="grid product-grid" id="dash-recs">${recs.length ? recs.map((r, i) => productCard(r.product, { rec: r, i })).join('') : stateBlock({ iconName: 'sparkles', title: 'No recommendations yet', text: 'Every product is either bought or out of stock.' })}</div>
    </section>

    <div class="dash-cols">
      <section class="card panel">
        <h3>Recent purchases</h3>
        <div id="purchases">${bought.length ? bought.slice(0, 5).map((p) => `<div class="purchase-item" data-id="${p.id}" data-open><div class="thumb">${productImageHtml(p)}</div><div><strong>${escapeHtml(p.name)}</strong><span>${escapeHtml(p.category)} · Rs ${money(p.price)}</span></div></div>`).join('') : `<p style="color:var(--muted)">You haven't bought anything yet. Your purchases will appear here and boost similar products in your recommendations.</p>`}</div>
      </section>
      <aside style="display:grid;gap:22px">
        <section class="card panel"><h3>Browse categories</h3><div class="chip-row">${Object.entries(cats).map(([c, n]) => `<a class="chip" href="products.html?category=${encodeURIComponent(c)}">${escapeHtml(c)} · ${n}</a>`).join('')}</div></section>
        <section class="card panel"><h3>Quick actions</h3><div class="quick">
          <a href="recommendations.html">${icon('sparkles')} My recommendations</a>
          <a href="products.html?sort=rating">${icon('star')} Top-rated products</a>
          <a href="products.html?sort=price-asc">${icon('tag')} Cheapest first</a>
        </div></section>
      </aside>
    </div>`;

  qs('#dash-search').addEventListener('submit', (e) => {
    e.preventDefault();
    const q = qs('#dash-q').value.trim();
    location.href = 'products.html' + (q ? '?q=' + encodeURIComponent(q) : '');
  });
  bindProductOpen(root, (id) => PRODUCT_INDEX[id], (id) => dashRecMap[id]);
}

// After a purchase the budget, stock and ranking all change, so reload from the backend.
window.addEventListener('shopsmart:purchased', initDashboard);
document.addEventListener('DOMContentLoaded', initDashboard);
