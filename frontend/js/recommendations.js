// Recommendations: ranking and scores come from the backend's max-heap + scoreProduct().
let recsByProduct = {};
let recK = 10;

async function initRecs() {
  const root = qs('#recs-root');
  let customer;
  try { customer = await requireAuth(); }
  catch (e) { root.innerHTML = errorBlock(e); qs('#retry-btn').onclick = initRecs; return; }
  if (!customer) return;

  const list = root.querySelector('#rec-list');
  if (!list) root.innerHTML = `
    <div class="page-head" style="padding-top:8px">
      <span class="eyebrow">${icon('sparkles')} Personalized for ${escapeHtml(customer.name)}</span>
      <h1>Your recommendations</h1>
      <p>Every product is scored for you, then a max-heap extracts the top matches. Open one to see exactly why it ranked here.</p>
    </div>
    <div class="rec-toolbar">
      <div class="chip-row" id="k-chips">${[5, 10, 20].map((k) => `<span class="chip ${k === recK ? 'on' : ''}" data-k="${k}">Top ${k}</span>`).join('')}</div>
      <span class="hint" id="rec-summary"></span>
    </div>
    <div class="rec-list" id="rec-list"></div>`;
  const target = qs('#rec-list');
  target.innerHTML = Array.from({ length: 4 }, () => '<div class="skeleton" style="height:130px;border-radius:16px"></div>').join('');

  let recs;
  try {
    const products = await Api.cachedProducts(true);
    indexProducts(products);
    recs = await Api.recommendations(customer.id, recK);
  } catch (e) { target.innerHTML = errorBlock(e); qs('#retry-btn').onclick = initRecs; return; }

  recsByProduct = {}; recs.forEach((r) => (recsByProduct[r.product.id] = r));
  qs('#rec-summary').textContent = `Budget left: Rs ${money(customer.budget)} · ${customer.purchaseHistory.length} past purchases`;

  if (!recs.length) {
    target.innerHTML = stateBlock({ iconName: 'sparkles', title: 'No recommendations available', text: 'Every in-stock product has already been purchased.' });
    return;
  }
  const max = Math.max(...recs.map((r) => r.score), 1);
  target.innerHTML = recs.map((r, i) => {
    const p = r.product;
    return `<article class="card rec-item" data-id="${p.id}" style="animation-delay:${Math.min(i * 50, 400)}ms">
      <div class="rank">${r.rank}</div>
      <div class="thumb" data-open>${productImageHtml(p)}</div>
      <div class="rec-info">
        <span class="pcard-cat">${escapeHtml(p.category)}</span>
        <h3 data-open>${escapeHtml(p.name)}</h3>
        <div class="pcard-brand">${escapeHtml(p.brand)} · ${starsHtml(p.rating)}</div>
        <div class="reasons">${r.breakdown.map((b) => `<span class="reason"><b>+${b.points}</b>${escapeHtml(b.reason)}</span>`).join('')}</div>
      </div>
      <div class="rec-side">
        <div class="score-big">${r.score.toFixed(1)}<small>Match score</small></div>
        <div class="meter"><i style="width:0" data-w="${Math.round((r.score / max) * 100)}%"></i></div>
        ${priceHtml(p.price)}
        <button class="btn btn-primary btn-sm" data-open>${icon('cart')} View &amp; buy</button>
      </div>
    </article>`;
  }).join('');
  requestAnimationFrame(() => qsa('.meter i', target).forEach((el) => (el.style.width = el.dataset.w)));
  bindProductOpen(target, (id) => PRODUCT_INDEX[id], (id) => recsByProduct[id]);
}

document.addEventListener('DOMContentLoaded', () => {
  qs('#recs-root').addEventListener('click', (e) => {
    const chip = e.target.closest('#k-chips .chip'); if (!chip) return;
    recK = Number(chip.dataset.k);
    qsa('#k-chips .chip').forEach((c) => c.classList.toggle('on', c === chip));
    initRecs();
  });
  initRecs();
});
window.addEventListener('shopsmart:purchased', initRecs);
