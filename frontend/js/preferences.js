// Preferences: saved via POST /api/customers/:id/preferences into CustomerManager,
// then the real recommender is queried again so you can see the effect immediately.
async function initPrefs() {
  const root = qs('#root');
  let customer, products;
  try {
    customer = await requireAuth(); if (!customer) return;
    products = await Api.cachedProducts(true);
  } catch (e) { root.innerHTML = errorBlock(e); qs('#retry-btn').onclick = initPrefs; return; }
  indexProducts(products);

  const cats = [...new Set(products.map((p) => p.category))].sort();
  const brands = [...new Set(products.map((p) => p.brand))].sort((a, b) => a.localeCompare(b));
  const selCats = new Set(customer.preferredCategories);
  const selBrands = new Set(customer.preferredBrands);

  root.innerHTML = `
    <div class="page-head" style="padding-top:8px"><span class="eyebrow">${icon('sparkles')} Personalize</span><h1>Your preferences</h1><p>Preferred categories add +3 to a product's score, preferred brands +2, and staying within budget +2.</p></div>
    <div class="two-col">
      <form class="card panel" id="pref-form" novalidate>
        <div class="field"><label for="budget">Budget (Rs)</label><input class="input" id="budget" inputmode="decimal" value="${customer.budget}"><span class="hint">This is also your remaining balance: purchases reduce it.</span><div class="error-text" id="budget-err"></div></div>
        <div class="field"><label>Preferred categories</label><div class="chip-row" id="cat-chips">${cats.map((c) => `<span class="chip ${selCats.has(c) ? 'on' : ''}" data-v="${escapeHtml(c)}">${escapeHtml(c)}</span>`).join('')}</div></div>
        <div class="field"><label for="brand-q">Preferred brands <span class="hint" id="brand-count"></span></label><input class="input" id="brand-q" placeholder="Filter brands…" autocomplete="off">
          <div class="brand-scroll"><div class="chip-row" id="brand-chips"></div></div></div>
        <div id="pref-alert"></div>
        <div class="save-bar"><button class="btn btn-primary" id="save" type="submit">Save preferences</button><a class="btn btn-ghost" href="recommendations.html">See recommendations</a></div>
      </form>
      <aside class="card panel"><h3>${icon('sparkles')} Top 3 right now</h3><div id="preview"></div></aside>
    </div>`;

  const brandWrap = qs('#brand-chips');
  function drawBrands() {
    const term = qs('#brand-q').value.trim().toLowerCase();
    brandWrap.innerHTML = brands.filter((b) => !term || b.toLowerCase().includes(term))
      .map((b) => `<span class="chip ${selBrands.has(b) ? 'on' : ''}" data-v="${escapeHtml(b)}">${escapeHtml(b)}</span>`).join('') || '<span class="hint" style="padding:8px">No brands match.</span>';
    qs('#brand-count').textContent = selBrands.size ? `(${selBrands.size} selected)` : '';
  }
  drawBrands();
  qs('#brand-q').addEventListener('input', drawBrands);
  const toggle = (set) => (e) => { const c = e.target.closest('.chip'); if (!c || !c.dataset.v) return; const v = c.dataset.v; set.has(v) ? set.delete(v) : set.add(v); c.classList.toggle('on', set.has(v)); qs('#brand-count').textContent = selBrands.size ? `(${selBrands.size} selected)` : ''; };
  qs('#cat-chips').addEventListener('click', toggle(selCats));
  brandWrap.addEventListener('click', toggle(selBrands));

  async function drawPreview() {
    const box = qs('#preview');
    box.innerHTML = '<div class="skeleton" style="height:60px"></div>';
    try {
      const recs = await Api.recommendations(customer.id, 3);
      box.innerHTML = recs.length ? recs.map((r) => `<div class="purchase-item" data-id="${r.product.id}" data-open><div class="thumb">${productImageHtml(r.product)}</div><div><strong>#${r.rank} ${escapeHtml(r.product.name)}</strong><span>Score ${r.score.toFixed(1)} · Rs ${money(r.product.price)}</span></div></div>`).join('') : '<p style="color:var(--muted)">No recommendations available.</p>';
      const recMap = {}; recs.forEach((r) => (recMap[r.product.id] = r));
      bindProductOpen(box, (id) => PRODUCT_INDEX[id], (id) => recMap[id]);
    } catch (e) { box.innerHTML = `<div class="alert error">${icon('alert')}<span>${escapeHtml(e.message)}</span></div>`; }
  }
  drawPreview();

  qs('#pref-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    qs('#pref-alert').innerHTML = ''; qs('#budget-err').textContent = ''; qs('#budget').classList.remove('invalid');
    const b = Number(qs('#budget').value.trim().replace(/,/g, ''));
    if (!isFinite(b) || b < 0 || qs('#budget').value.trim() === '') { qs('#budget-err').textContent = 'Enter a budget of 0 or more.'; qs('#budget').classList.add('invalid'); return; }
    const btn = qs('#save'); btn.disabled = true; btn.innerHTML = '<span class="spinner"></span> Saving…';
    try {
      const saved = await Api.savePreferences(customer.id, { budget: b, preferredCategories: [...selCats], preferredBrands: [...selBrands] });
      customer = saved;
      toast('Preferences saved. Recommendations updated.', 'success');
      await drawPreview();
    } catch (err) { qs('#pref-alert').innerHTML = `<div class="alert error" style="margin-bottom:12px">${icon('alert')}<span>${escapeHtml(err.message)}</span></div>`; }
    btn.disabled = false; btn.textContent = 'Save preferences';
  });
}
document.addEventListener('DOMContentLoaded', initPrefs);
