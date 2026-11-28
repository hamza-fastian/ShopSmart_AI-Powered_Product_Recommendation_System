// Profile: read-only view of the Customer record. The backend has no way to rename a
// customer, so only budget and preferences are editable (on the Preferences page).
async function initProfile() {
  const root = qs('#root');
  let customer, products;
  try {
    customer = await requireAuth(); if (!customer) return;
    products = await Api.cachedProducts(true);
  } catch (e) { root.innerHTML = errorBlock(e); qs('#retry-btn').onclick = initProfile; return; }
  indexProducts(products);
  const spent = customer.purchaseHistory.map((id) => PRODUCT_INDEX[id]).filter(Boolean).reduce((s, p) => s + p.price, 0);
  const tags = (arr) => arr.length ? `<div class="chip-row" style="justify-content:flex-end">${arr.map((x) => `<span class="chip on" style="cursor:default">${escapeHtml(x)}</span>`).join('')}</div>` : '<b>None yet</b>';
  root.innerHTML = `
    <section class="card profile-head fade-in">
      <div class="avatar-lg">${escapeHtml(customer.name.charAt(0).toUpperCase())}</div>
      <div style="flex:1;min-width:200px"><span class="eyebrow">ShopSmart customer</span><h1>${escapeHtml(customer.name)}</h1><p style="color:var(--muted)">Customer ID <b style="color:var(--ink)">#${customer.id}</b></p></div>
      <div style="display:flex;gap:10px;flex-wrap:wrap"><a class="btn btn-primary" href="preferences.html">Edit preferences</a><button class="btn btn-ghost" id="logout2">${icon('logout')} Log out</button></div>
    </section>
    <div class="two-col">
      <section class="card panel"><h3>Account details</h3><div class="kv">
        <div><span>Name</span><b>${escapeHtml(customer.name)}</b></div>
        <div><span>Customer ID</span><b>${customer.id}</b></div>
        <div><span>Budget remaining</span><b>Rs ${money(customer.budget)}</b></div>
        <div><span>Products purchased</span><b>${customer.purchaseHistory.length}</b></div>
        <div><span>Total value purchased</span><b>Rs ${money(spent)}</b></div>
        <div><span>Preferred categories</span>${tags(customer.preferredCategories)}</div>
        <div><span>Preferred brands</span>${tags(customer.preferredBrands)}</div>
      </div><p class="hint" style="margin-top:14px">The ShopSmart backend does not support renaming a customer, so the name is read-only.</p></section>
      <aside class="card panel"><h3>Shortcuts</h3><div class="quick">
        <a href="recommendations.html">${icon('sparkles')} My recommendations</a>
        <a href="history.html">${icon('receipt')} Purchase history</a>
        <a href="preferences.html">${icon('tag')} Preferences</a></div></aside>
    </div>`;
  qs('#logout2').onclick = logout;
}
document.addEventListener('DOMContentLoaded', initProfile);
