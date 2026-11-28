// Product image mapping.
//
// Real photos go in frontend/assets/images/products/ and are named by PRODUCT ID
// (ids come from data/products.csv), e.g. 5.jpg for the Laptop. The server lists
// which files exist (GET /api/product-images) and they are picked up automatically.
// A product without a photo shows a clean category/type tile instead
// (no broken images, no unrelated stock pictures).
const PRODUCT_PHOTOS = {};
const PHOTO_DIR = 'assets/images/products/';

const TILE_COLORS = {
  Electronics: ['#eef2ff', '#4f46e5'],
  Books: ['#fff7ed', '#c2410c'],
  Home: ['#ecfdf5', '#047857'],
  Fashion: ['#fdf2f8', '#be185d'],
  Accessories: ['#f5f3ff', '#6d28d9'],
  'Personal Care': ['#ecfeff', '#0e7490'],
  Sports: ['#f0fdf4', '#15803d'],
  Furniture: ['#fefce8', '#a16207'],
};

// Most specific keyword first.
const GLYPH_RULES = [
  [/laptop/, 'laptop'], [/tablet/, 'tablet'], [/iphone|smartphone|mobile cover|screen protector/, 'phone'],
  [/headphone|earbud|headset/, 'headphones'], [/speaker|soundbar/, 'speaker'], [/watch/, 'watch'],
  [/monitor|television|projector/, 'monitor'], [/keyboard/, 'keyboard'], [/mouse/, 'mouse'],
  [/webcam|camera/, 'camera'], [/microphone/, 'mic'],
  [/book|novel|grammar|programming|computer science|artificial|machine learning|operating|networking|engineering|cloud|cyber|database|web development/, 'book'],
  [/chair|table/, 'chair'], [/lamp|bulb/, 'bulb'], [/shoes/, 'shoe'], [/shirt|jacket|jeans/, 'shirt'],
  [/bag|backpack|wallet/, 'bag'], [/football|ball|racket|bat\b/, 'ball'], [/dumbbell|yoga|treadmill/, 'dumbbell'],
  [/bicycle/, 'bike'], [/bottle/, 'bottle'], [/charger|cable|power|usb/, 'plug'],
  [/ssd|hard drive|ram|processor|graphics|motherboard|cooler|case|router|printer/, 'chip'],
];
const CATEGORY_GLYPH = { Electronics: 'chip', Books: 'book', Home: 'home', Fashion: 'shirt', Accessories: 'bag', 'Personal Care': 'sparkles', Sports: 'ball', Furniture: 'chair' };

function glyphFor(p) {
  const name = p.name.toLowerCase();
  for (const [re, g] of GLYPH_RULES) if (re.test(name)) return g;
  return CATEGORY_GLYPH[p.category] || 'box';
}

function tileHtml(p) {
  const [bg, fg] = TILE_COLORS[p.category] || TILE_COLORS.Electronics;
  return `<div class="tile" style="--tile-bg:${bg};--tile-fg:${fg}">${icon(glyphFor(p))}</div>`;
}


// Returns the product's actual downloaded image.
function productImageHtml(p) {
  const imagePath = `/assets/images/products/${p.id}.jpg`;

  return `
    <img
      src="${imagePath}"
      alt="${escapeHtml(p.name)}"
      loading="lazy"
      onerror="this.outerHTML=window.__tile(${p.id})"
    >
  `;
}
