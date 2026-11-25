const productGrid = document.getElementById("productGrid");
const searchInput = document.getElementById("searchInput");
const categoryFilter = document.getElementById("categoryFilter");
const sortFilter = document.getElementById("sortFilter");
const productCount = document.getElementById("productCount");
const emptyState = document.getElementById("emptyState");
const cartCount = document.getElementById("cartCount");
const toast = document.getElementById("toast");

let products = [];
let cartItems = 0;
let toastTimer;

function parseCSVLine(line) {
  const values = [];
  let value = "";
  let insideQuotes = false;

  for (let i = 0; i < line.length; i++) {
    const char = line[i];

    if (char === '"' && line[i + 1] === '"' && insideQuotes) {
      value += '"';
      i++;
    } else if (char === '"') {
      insideQuotes = !insideQuotes;
    } else if (char === "," && !insideQuotes) {
      values.push(value.trim());
      value = "";
    } else {
      value += char;
    }
  }

  values.push(value.trim());
  return values;
}

function csvToProducts(csvText) {
  const lines = csvText.trim().split(/\r?\n/);
  if (lines.length < 2) return [];

  const headers = parseCSVLine(lines[0]).map(header => header.toLowerCase());
  return lines.slice(1).filter(line => line.trim()).map(line => {
    const values = parseCSVLine(line);
    const item = {};
    headers.forEach((header, index) => item[header] = values[index] ?? "");

    return {
      id: Number(item.id),
      name: item.name || "Unnamed product",
      category: item.category || "Other",
      brand: item.brand || "ShopSmart",
      price: Number(item.price) || 0,
      rating: Number(item.rating) || 0,
      purchaseCount: Number(item.purchasecount) || 0,
      stock: Number(item.stock) || 0
    };
  });
}

function getProductIcon(category, name) {
  const text = (category + " " + name).toLowerCase();
  if (/headphone|earbud|speaker|audio/.test(text)) return "🎧";
  if (/watch|fitness band/.test(text)) return "⌚";
  if (/keyboard/.test(text)) return "⌨️";
  if (/mouse/.test(text)) return "🖱️";
  if (/laptop|computer/.test(text)) return "💻";
  if (/phone|mobile/.test(text)) return "📱";
  if (/book|novel/.test(text)) return "📚";
  if (/shoe|sport|fashion/.test(text)) return "👟";
  if (/bag|backpack/.test(text)) return "🎒";
  if (/monitor|screen/.test(text)) return "🖥️";
  if (/camera/.test(text)) return "📷";
  return "✦";
}

function showToast(message) {
  toast.textContent = message;
  toast.classList.add("show");
  clearTimeout(toastTimer);
  toastTimer = setTimeout(() => toast.classList.remove("show"), 2200);
}

function renderProducts() {
  const searchTerm = searchInput.value.trim().toLowerCase();
  const selectedCategory = categoryFilter.value;
  let visibleProducts = products.filter(product => {
    const matchesSearch = [product.name, product.brand, product.category]
      .some(value => value.toLowerCase().includes(searchTerm));
    const matchesCategory = !selectedCategory || product.category === selectedCategory;
    return matchesSearch && matchesCategory;
  });

  if (sortFilter.value === "price-low") visibleProducts.sort((a, b) => a.price - b.price);
  if (sortFilter.value === "price-high") visibleProducts.sort((a, b) => b.price - a.price);
  if (sortFilter.value === "rating") visibleProducts.sort((a, b) => b.rating - a.rating);
  if (sortFilter.value === "popular") visibleProducts.sort((a, b) => b.purchaseCount - a.purchaseCount);

  productCount.textContent = visibleProducts.length + " products";
  productGrid.innerHTML = "";

  visibleProducts.forEach(product => {
    const card = document.createElement("article");
    card.className = "product-card";

    const visual = document.createElement("div");
    visual.className = "product-visual";
    const productId = document.createElement("span");
    productId.className = "product-id";
    productId.textContent = "PRODUCT #" + product.id;
    const emoji = document.createElement("span");
    emoji.className = "product-emoji";
    emoji.textContent = getProductIcon(product.category, product.name);
    visual.append(productId, emoji);

    const info = document.createElement("div");
    info.className = "product-info";
    const category = document.createElement("div");
    category.className = "product-category";
    category.textContent = product.category;
    const name = document.createElement("h3");
    name.className = "product-name";
    name.textContent = product.name;
    name.title = product.name;
    const brand = document.createElement("p");
    brand.className = "product-brand";
    brand.textContent = product.brand;
    const meta = document.createElement("div");
    meta.className = "product-meta";
    const price = document.createElement("strong");
    price.className = "product-price";
    price.textContent = "Rs. " + product.price.toLocaleString("en-PK");
    const rating = document.createElement("span");
    rating.className = "product-rating";
    rating.innerHTML = '<span>★</span> ' + product.rating.toFixed(1);
    meta.append(price, rating);

    const bottom = document.createElement("div");
    bottom.className = "product-bottom";
    const stock = document.createElement("span");
    stock.className = "stock-label" + (product.stock <= 0 ? " out" : "");
    stock.textContent = product.stock > 0 ? product.stock + " in stock" : "Out of stock";
    const addButton = document.createElement("button");
    addButton.className = "add-button";
    addButton.type = "button";
    addButton.textContent = product.stock > 0 ? "Add to cart +" : "Unavailable";
    addButton.disabled = product.stock <= 0;
    addButton.addEventListener("click", () => {
      cartItems++;
      cartCount.textContent = cartItems;
      showToast(product.name + " added to your demo cart");
    });
    bottom.append(stock, addButton);
    info.append(category, name, brand, meta, bottom);
    card.append(visual, info);
    productGrid.append(card);
  });

  emptyState.hidden = visibleProducts.length !== 0;
}

function populateCategories() {
  const categories = [...new Set(products.map(product => product.category))].sort();
  categories.forEach(category => {
    const option = document.createElement("option");
    option.value = category;
    option.textContent = category;
    categoryFilter.append(option);
  });
}

async function loadProducts() {
  try {
    const response = await fetch("../data/products.csv");
    if (!response.ok) throw new Error("Could not load data/products.csv");
    const csvText = await response.text();
    products = csvToProducts(csvText);

    if (products.length === 0) throw new Error("No product rows were found in the CSV file");
    populateCategories();
    renderProducts();
  } catch (error) {
    productGrid.innerHTML = "";
    productCount.textContent = "Products unavailable";
    emptyState.hidden = false;
    emptyState.textContent = "Could not load the product list. Open the project through a local web server (for example, VS Code Live Server) and make sure data/products.csv exists.";
    console.error("ShopSmart product loading error:", error);
  }
}

searchInput.addEventListener("input", renderProducts);
categoryFilter.addEventListener("change", renderProducts);
sortFilter.addEventListener("change", renderProducts);
document.getElementById("cartButton").addEventListener("click", () => {
  showToast("You have " + cartItems + " item(s) in your demo cart.");
});

loadProducts();
