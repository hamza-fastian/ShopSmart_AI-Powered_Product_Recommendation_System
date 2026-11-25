
# ShopSmart – AI-Powered Product Recommendation System

ShopSmart is a C++ based product recommendation system developed using Data Structures and Algorithms.

It recommends products according to customer preferences, budget, purchase history, product ratings and popularity.

## Features

- Product catalog management
- Customer registration and management
- Product searching using Hash Table
- Price-based product organization using BST
- Name-based product organization using AVL Tree
- Circular product navigation
- Personalized product scoring
- Top-K product recommendations using Max Heap
- CSV-based data storage

## Data Structures Used

| Data Structure | Purpose |
|---|---|
| Hash Table | Product and customer searching |
| BST | Price-based product organization |
| AVL Tree | Name-based product organization |
| Circular Linked List | Product navigation |
| Max Heap | Recommendation ranking |
| Vector | Product collection and processing |

## Recommendation System

The recommendation score considers:

- Preferred category
- Preferred brand
- Customer budget
- Previous purchases
- Product rating
- Product popularity

The Max Heap ranks products according to their calculated scores.

## Project Structure

```text
ShopSmart/
│
├── AVL.h
├── BST.h
├── Catalog.h
├── CircularLL.h
├── Customer.h
├── CustomerManager.h
├── FileHandler.h
├── HashTable.h
├── MaxHeap.h
├── Product.h
├── Recommender.h
├── Scoring.h
├── main.cpp
├── test_A.cpp
├── test_B.cpp
│
└── data/
    ├── products.csv
    └── customers.csv
```

## Requirements

- C++ compiler supporting C++11
- Visual Studio Code or any C++ IDE

## Compilation

Compile the main application:

```bash
g++ -std=c++11 main.cpp -o ShopSmart
```

Run on Windows:

```bash
ShopSmart.exe
```

Run on Linux:

```bash
./ShopSmart
```

The program expects to run from the project root directory so it can access the data folder.

## Testing

The project also contains two separate test programs:

- `test_A.cpp`
- `test_B.cpp`

Compile and execute them separately because each contains its own main() function.

## Project Goal

The goal of ShopSmart is to demonstrate how different data structures can work together to build a personalized product recommendation system.
