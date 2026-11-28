# ShopSmart – AI-Powered Product Recommendation System

ShopSmart is a web-based product recommendation system developed as a Data Structures and Algorithms (DSA) project. It provides customers with a simple online shopping experience and recommends products according to their preferences, budget, purchase history, and product ratings.

The project uses **C++ for the backend** and **HTML, CSS, and JavaScript for the frontend**. It uses multiple data structures to manage products, customers, searching, sorting, and recommendations efficiently.

---

## 1. Project Overview

ShopSmart allows users to:

- Create an account and log in.
- Browse the available product catalog.
- Search for products by name or brand.
- Filter products by category.
- Sort products by price, rating, popularity, and recommendations.
- View product details.
- Purchase products.
- Manage customer preferences.
- Receive personalized product recommendations.
- View product stock availability.

The recommendation system uses customer preferences and purchase history to calculate product scores and rank suitable products.

## 2. Technologies Used

| Component | Technology |
|---|---|
| Backend | C++ |
| Frontend | HTML5, CSS3, JavaScript |
| Compiler | MSYS2 UCRT64 GCC |
| Development Environment | Visual Studio Code |
| Communication | HTTP API |
| Product Data | CSV |
| Product Images | JPG |

## 3. Data Structures Implemented

The project demonstrates the practical implementation of the following data structures:

### Hash Table
- Stores and retrieves products and customer information.
- Uses separate chaining for collision handling.
- Resizes when the load factor exceeds 1.

### Binary Search Tree (BST)
- Organizes products according to their prices.
- Supports efficient price-based searching.

### AVL Tree
- Stores products using their names.
- Automatically balances itself after insertions and deletions.
- Maintains efficient searching operations.

### Max Heap
- Used by the recommendation system to rank products according to their recommendation scores.
- Helps retrieve highly recommended products.

### Circular Linked List
- Supports circular navigation through the product catalog.

### Vector
- Maintains the product collection and supports product operations.

## 4. Recommendation System

ShopSmart uses a rule-based recommendation algorithm to calculate product scores based on customer information.

The scoring system considers:

| Condition | Score |
|---|---:|
| Product belongs to a preferred category | +3 |
| Product matches a preferred brand | +2 |
| Product is within the customer's budget | +2 |
| Product belongs to a previously purchased category | +2 |
| Product rating | Added to score |
| Product has at least 50 purchases | +1 |

The recommendation system:
- Checks customer preferences.
- Considers purchase history.
- Skips products that are out of stock.
- Skips products already purchased by the customer.
- Ranks suitable products using a Max Heap.

## 5. Project Structure

The project is organized into backend and frontend directories.

```text
ShopSmart_AI-Powered_Product_Recommendation_System/
│
├── data/
│   └── products.csv
│
├── frontend/
│   ├── assets/
│   │   └── images/
│   │       └── products/
│   │           ├── 1.jpg
│   │           ├── 2.jpg
│   │           ├── 3.jpg
│   │           └── ...
│   │
│   ├── css/
│   ├── js/
│   ├── index.html
│   ├── login.html
│   ├── signup.html
│   ├── dashboard.html
│   ├── products.html
│   └── recommendations.html
│
├── server.cpp
├── server.exe
├── README.md
└── [Other C++ header and source files]
```

*Note: The structure above highlights the main project files. Additional C++ header and source files are included in the project.*

## 6. Product Images

Product images are stored locally inside:

```text
frontend/assets/images/products/
```

Each image is named according to its product ID.

For example:

```text
1.jpg
2.jpg
3.jpg
4.jpg
...
```

The frontend automatically loads the corresponding image using the product ID.

If an image is unavailable, the frontend displays a fallback product illustration.

## 7. Requirements

Before running the project, make sure you have:

- Visual Studio Code
- C/C++ extension for VS Code
- MSYS2 UCRT64 GCC compiler
- Windows operating system

The GCC compiler should be available at:

```text
C:\msys64\ucrt64\bin
```

## 8. How to Run the Project

Follow these steps to run ShopSmart using Visual Studio Code.

### Step 1: Open the Project

Open the ShopSmart project folder in Visual Studio Code.

### Step 2: Open the VS Code Terminal

Open the integrated terminal:

```text
Terminal → New Terminal
```

Make sure the terminal is using PowerShell and the current directory is the project root, where `server.exe` is located.

### Step 3: Add the MSYS2 Compiler Path

Copy and paste this command into the VS Code PowerShell terminal:

```powershell
$env:Path = "C:\msys64\ucrt64\bin;" + $env:Path
```

### Step 4: Start the Server

Run:

```powershell
.\server.exe
```

The C++ server will start and host the frontend and backend API.

### Step 5: Open ShopSmart in Your Browser

Open your browser and visit:

```text
http://localhost:8080
```

The ShopSmart website should now be running.

### Important Notes

- Keep the VS Code terminal open while using the website.
- Do not close the terminal while the server is running.
- To stop the server, press `Ctrl + C` in the terminal.
- Whenever you restart your computer or close VS Code, repeat Steps 2–4 to run the project again.
- The `server.exe` executable must already be compiled and present in the project root.

## 9. Backend API

The C++ server provides API endpoints used by the frontend.

| Endpoint | Purpose |
|---|---|
| `/api/health` | Checks server status |
| `/api/products` | Retrieves product catalog |
| `/api/recommendations` | Retrieves personalized product recommendations |

The frontend communicates with these endpoints to retrieve product information and display updated catalog and recommendation data.

## 10. Frontend Pages

| Page | Description |
|---|---|
| `index.html` | Main landing page |
| `login.html` | Existing customer login |
| `signup.html` | New customer registration |
| `dashboard.html` | Customer dashboard |
| `products.html` | Product catalog with search, filtering, and sorting |
| `recommendations.html` | Personalized product recommendations |

## 11. Main Features

- User registration and login.
- Product catalog with locally stored product images.
- Product searching and category filtering.
- Price, rating, popularity, and recommendation-based sorting.
- Product details and purchasing functionality.
- Customer preferences and purchase history.
- Personalized product recommendations.
- Product stock management.
- Backend integration with a browser-based frontend.

## 12. Project Objective

The main objective of ShopSmart is to demonstrate how fundamental data structures and algorithms can be used in a practical e-commerce application.

The project combines data structure concepts with a functional web interface to provide an organized shopping experience and personalized product recommendations.

---

**ShopSmart – Smart Shopping, Personalized Recommendations.**