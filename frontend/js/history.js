// Purchase history: the backend stores the ordered list of purchased product IDs
// (Customer::purchaseHistory). It does not store dates or quantities, so none are shown.
async function initHistory() {
  const root = qs('#root');
  let customer, products;
  try {
    customer = await requireAuth(); if (!customer) return;
    products = await Api.cachedProducts(true);
  } catch (e) { root.innerHTML = errorBlock(e); qs('#retry-btn').onclick = initHistory; return; }
  indexProducts(products);

  const items = customer.purchaseHistory.map((id) => PRODUCT_INDEX[id]).filter(Boolean);
  const total = items.reduce((s, p) => s + p.price, 0);
  const head = `<div class="page-head" style="padding-top:8px"><span class="eyebrow">${icon('receipt')} Your orders</span><h1>Purchase history</h1><p>Products you have bought, oldest first. Each purchase boosts similar products in your recommendations.</p></div>`;

  if (!items.length) {
    root.innerHTML = head + stateBlock({ iconName: 'receipt', title: 'No purchases yet', text: 'When you buy a product it will appear here.', action: '<a class="btn btn-primary" href="products.html">Browse products</a>' });
    return;
  }
  root.innerHTML = head + `
    <div class="grid stat-grid" style="margin-top:20px">
      <div class="card stat"><span class="cat-ico">${icon('receipt')}</span><div><small>Items purchased</small><strong>${items.length}</strong></div></div>
      <div class="card stat"><span class="cat-ico">${icon('wallet')}</span><div><small>Total value (current prices)</small><strong>Rs ${money(total)}</strong></div></div>
      <div class="card stat"><span class="cat-ico">${icon('wallet')}</span><div><small>Budget remaining</small><strong>Rs ${money(customer.budget)}</strong></div></div>
    </div>
    <div class="card" style="margin-top:22px;overflow:hidden" id="h-list">
      ${items.map((p, i) => `<div class="h-row" data-id="${p.id}" data-open><span class="h-no">#${i + 1}</span><div class="thumb">${productImageHtml(p)}</div><div><strong>${escapeHtml(p.name)}</strong><span class="meta">${escapeHtml(p.category)} · ${escapeHtml(p.brand)} · ${starsHtml(p.rating)}</span></div>${priceHtml(p.price)}</div>`).join('')}
    </div>`;
  bindProductOpen(qs('#h-list'), (id) => PRODUCT_INDEX[id]);
}
window.addEventListener('shopsmart:purchased', initHistory);
document.addEventListener('DOMContentLoaded', initHistory);
