// Landing page: everything shown comes from the real product data.
const CATEGORY_ICON = { Electronics: 'chip', Books: 'book', Home: 'home', Fashion: 'shirt', Accessories: 'bag', 'Personal Care': 'sparkles', Sports: 'ball', Furniture: 'chair' };

async function initHome() {
  const featured = qs('#featured');
  featured.innerHTML = skeletonCards(8);
  qs('#categories').innerHTML = Array.from({ length: 8 }, () => '<div class="skeleton" style="height:78px;border-radius:16px"></div>').join('');

  if (Session.get()) {
    qs('#hero-cta').innerHTML = `<a class="btn btn-primary" href="dashboard.html">Open my dashboard</a><a class="btn btn-ghost" href="recommendations.html">${icon('sparkles')} My recommendations</a>`;
  }

  let products;
  try { products = await Api.cachedProducts(); }
  catch (e) {
    featured.innerHTML = `<div style="grid-column:1/-1">${errorBlock(e)}</div>`;
    qs('#categories').innerHTML = '';
    qs('#hero-visual').innerHTML = '';
    qs('#retry-btn').onclick = initHome;
    return;
  }
  indexProducts(products);

  // hero stats
  const cats = {};
  products.forEach((p) => (cats[p.category] = (cats[p.category] || 0) + 1));
  const brands = new Set(products.map((p) => p.brand));
  qs('#hero-stats').innerHTML = `
    <div><strong>${products.length}</strong><span>Products</span></div>
    <div><strong>${Object.keys(cats).length}</strong><span>Categories</span></div>
    <div><strong>${brands.size}</strong><span>Brands</span></div>`;

  // categories
  qs('#categories').innerHTML = Object.entries(cats).sort((a, b) => b[1] - a[1]).map(([c, n]) =>
    `<a class="card cat-card" href="products.html?category=${encodeURIComponent(c)}"><span class="cat-ico">${icon(CATEGORY_ICON[c] || 'box')}</span><div><strong>${escapeHtml(c)}</strong><span>${n} products</span></div></a>`).join('');

  // featured = top rated, ties broken by popularity
  const top = [...products].filter((p) => p.stock > 0).sort((a, b) => b.rating - a.rating || b.purchaseCount - a.purchaseCount);
  featured.innerHTML = top.slice(0, 8).map((p, i) => productCard(p, { i })).join('');
  bindProductOpen(featured, (id) => PRODUCT_INDEX[id]);

  // hero floating cards: top rated from three different categories
  const seen = new Set(); const picks = [];
  for (const p of top) { if (!seen.has(p.category)) { seen.add(p.category); picks.push(p); } if (picks.length === 3) break; }
  const hv = qs('#hero-visual');
  hv.innerHTML = picks.map((p) => `<div class="float-card" data-id="${p.id}" data-open><div class="thumb">${productImageHtml(p)}</div><h4>${escapeHtml(p.name)}</h4><div class="row"><span>${escapeHtml(p.brand)}</span><b>Rs ${money(p.price)}</b></div></div>`).join('');
  bindProductOpen(hv, (id) => PRODUCT_INDEX[id]);
}

document.addEventListener('DOMContentLoaded', initHome);
