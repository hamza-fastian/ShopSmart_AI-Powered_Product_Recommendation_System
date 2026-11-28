// Login + signup. Both talk to the real backend (hash-table lookup / registerCustomer).
const AUTH_POINTS = [
  ['sparkles', 'Recommendations ranked by a max-heap, scored on your preferences'],
  ['wallet', 'Budget-aware: purchases update your remaining budget'],
  ['shield', 'Your data is stored by the ShopSmart C++ backend (customers.csv)'],
];

function setLoading(btn, loading, label) {
  btn.disabled = loading;
  btn.innerHTML = loading ? '<span class="spinner"></span> Please wait…' : label;
}
function fieldError(id, msg) {
  const input = qs('#' + id); const err = qs('#' + id + '-err');
  if (input) input.classList.toggle('invalid', !!msg);
  if (err) err.textContent = msg || '';
}
function showAlert(type, text) {
  qs('#form-alert').innerHTML = text ? `<div class="alert ${type}" style="margin-bottom:18px">${icon(type === 'error' ? 'alert' : 'check')}<span>${text}</span></div>` : '';
}
function safeNext() {
  const n = new URLSearchParams(location.search).get('next');
  return n && /^[a-z]+\.html(\?[\w=&%.-]*)?$/i.test(n) ? n : 'dashboard.html';
}

document.addEventListener('DOMContentLoaded', () => {
  const pts = qs('#auth-points');
  if (pts) pts.innerHTML = AUTH_POINTS.map(([i, t]) => `<div><i>${icon(i)}</i><span>${t}</span></div>`).join('');

  const loginForm = qs('#login-form');
  if (loginForm) initLogin(loginForm);
  const signupForm = qs('#signup-form');
  if (signupForm) initSignup(signupForm);
});

function initLogin(form) {
  const btn = qs('#submit');
  form.addEventListener('submit', async (e) => {
    e.preventDefault();
    showAlert('', '');
    const raw = qs('#cid').value.trim();
    fieldError('cid', '');
    if (!raw) return fieldError('cid', 'Please enter your Customer ID.');
    if (!/^\d+$/.test(raw)) return fieldError('cid', 'Customer IDs are numbers, for example 1001.');
    setLoading(btn, true);
    try {
      const customer = await Api.login(Number(raw));
      Session.set(customer);
      location.href = safeNext();
    } catch (err) {
      showAlert('error', escapeHtml(err.status === 404 ? 'No customer found with that ID. Check the number or create an account.' : err.message));
      setLoading(btn, false, 'Log in');
    }
  });
}

async function initSignup(form) {
  const btn = qs('#submit');
  const chosen = new Set();

  // categories come from the real catalog
  const chips = qs('#cat-chips');
  try {
    const products = await Api.cachedProducts();
    const cats = [...new Set(products.map((p) => p.category))].sort();
    chips.innerHTML = cats.map((c) => `<span class="chip" data-cat="${escapeHtml(c)}">${escapeHtml(c)}</span>`).join('');
    chips.addEventListener('click', (e) => {
      const chip = e.target.closest('.chip'); if (!chip) return;
      const c = chip.dataset.cat;
      chosen.has(c) ? chosen.delete(c) : chosen.add(c);
      chip.classList.toggle('on', chosen.has(c));
    });
  } catch (err) {
    chips.innerHTML = '';
    showAlert('error', escapeHtml(err.message));
  }

  form.addEventListener('submit', async (e) => {
    e.preventDefault();
    showAlert('', '');
    const name = qs('#name').value.trim();
    const budgetRaw = qs('#budget').value.trim().replace(/,/g, '');
    fieldError('name', ''); fieldError('budget', '');
    let ok = true;
    if (name.length < 2) { fieldError('name', 'Please enter your name (at least 2 characters).'); ok = false; }
    else if (/[,;\n]/.test(name)) { fieldError('name', 'Names cannot contain commas or semicolons.'); ok = false; }
    const budget = Number(budgetRaw);
    if (!budgetRaw || !isFinite(budget) || budget <= 0) { fieldError('budget', 'Enter a budget greater than 0.'); ok = false; }
    if (!ok) return;

    setLoading(btn, true);
    try {
      const customer = await Api.register({ name, budget, preferredCategories: [...chosen] });
      Session.set(customer);
      qs('#signup-card').innerHTML = `<div class="success-card">
        <div class="state-ico">${icon('check')}</div>
        <h1>You're in, ${escapeHtml(customer.name.split(' ')[0])}!</h1>
        <p style="color:var(--muted)">Save your Customer ID. You'll use it to log in next time.</p>
        <div class="id-box"><small>Your Customer ID</small><strong>${customer.id}</strong></div>
        <a class="btn btn-primary btn-block" href="dashboard.html">Go to my dashboard</a>
      </div>`;
      renderChrome_refresh();
    } catch (err) {
      showAlert('error', escapeHtml(err.message));
      setLoading(btn, false, 'Create account');
    }
  });
}

// Re-render the navbar after signup so it shows the logged-in user.
function renderChrome_refresh() {
  const nav = qs('#nav');
  if (!nav) return;
  const holder = document.createElement('div'); holder.id = 'site-nav';
  nav.replaceWith(holder);
  renderChrome();
}
