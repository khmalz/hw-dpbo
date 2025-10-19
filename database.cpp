#ifndef DATABASE_CPP
#define DATABASE_CPP

#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <chrono>

#include "repo/buyer.h"
#include "repo/seller.h"
#include "repo/bank.h"
#include "repo/transaction.h"
#include "repo/item.h"
#include "repo/items.h"

using namespace std;
using namespace chrono;

class Database
{
public:
   inline static vector<Buyer> buyers;
   inline static vector<Seller> sellers;
   inline static Bank mainBank;
   inline static vector<Transaction> transactionLog;

   inline static string globalMessage = "";
   inline static Buyer *loggedInBuyer = nullptr;
   inline static Seller *loggedInSeller = nullptr;
   inline static unsigned int nextItemId = 1;
   inline static unsigned int nextTransactionId = 1001;

   inline static void displayGlobalMessage()
   {
      if (!globalMessage.empty())
      {
         cout << "----------------------------------------\n";
         cout << ">> " << globalMessage << "\n";
         cout << "----------------------------------------\n\n";
         globalMessage = "";
      }
   }

   static void seedBuyerSeler()
   {
      buyers.emplace_back(1, "Alice Wonderland", "alice@example.com", 500000.0);
      mainBank.addCustomer(*(buyers.back().getCustomer()));
      buyers.emplace_back(2, "Bob The Builder", "bob@example.com", 750000.0);
      mainBank.addCustomer(*(buyers.back().getCustomer()));
      buyers.emplace_back(3, "Charlie Chaplin", "charlie@example.com", 300000.0);
      mainBank.addCustomer(*(buyers.back().getCustomer()));
      buyers.emplace_back(4, "Diana Prince", "diana@example.com", 1000000.0);
      mainBank.addCustomer(*(buyers.back().getCustomer()));
      buyers.emplace_back(5, "Sleepy Hollow", "sleepy@example.com", 10000.0);
      mainBank.addCustomer(*(buyers.back().getCustomer()));

      // Seller 1 (Alice)
      sellers.emplace_back(&buyers[0], "Alice's Emporium", "123 Main St", "alice.store@example.com");
      Items *aliceStore = sellers[0].getStoreItems();
      aliceStore->addItem(Item(nextItemId++, "Buku Tulis", 5000.0, 50));
      aliceStore->addItem(Item(nextItemId++, "Pensil 2B", 2000.0, 100));
      aliceStore->addItem(Item(nextItemId++, "Penghapus", 1500.0, 75));

      // Seller 2 (Bob)
      sellers.emplace_back(&buyers[1], "Bob's Hardware", "456 Oak Ave", "bob.hardware@example.com");
      Items *bobStore = sellers[1].getStoreItems();
      bobStore->addItem(Item(nextItemId++, "Palu", 50000.0, 20));
      bobStore->addItem(Item(nextItemId++, "Obeng Set", 75000.0, 15));
      bobStore->addItem(Item(nextItemId++, "Paku 1kg", 25000.0, 30));
   }

   inline static void seedDatabase()
   {
      if (!buyers.empty())
      {
         return;
      }

      cout << "Seeding database with initial data..." << endl;

      seedBuyerSeler();

      mt19937 rng(static_cast<unsigned int>(system_clock::now().time_since_epoch().count()));
      // Seeding Transaksi Historis ( antara 90 hari terakhir)
      uniform_int_distribution<int> buyerDist(0, buyers.size() - 1);
      uniform_int_distribution<int> sellerDist(0, sellers.size() - 1);
      uniform_int_distribution<int> qtyDist(1, 5);

      uniform_int_distribution<int> weekDist(1, 7);   // 1-7 days ago
      uniform_int_distribution<int> olderDist(8, 90); // 8-90 days ago

      int numberOfTransactions = 100;
      int todayCount = 15;    // Jumlah transaksi untuk HARI INI
      int lastWeekCount = 35; // Jumlah transaksi untuk 1-7 HARI LALU
      for (int i = 0; i < numberOfTransactions; ++i)
      {
         int buyerIndex = buyerDist(rng);
         int sellerIndex = sellerDist(rng);
         Buyer *buyer = &buyers[buyerIndex];
         Seller *seller = &sellers[sellerIndex];

         if (buyer->getId() == 5 || seller->getBuyer()->getId() == 5)
         {
            i--;
            continue;
         }

         Items *sellerItems = seller->getStoreItems();

         if (buyer->getId() == seller->getBuyer()->getId() || sellerItems->getItems().empty())
         {
            // Ulang
            i--;
            continue;
         }

         uniform_int_distribution<int> itemDist(0, sellerItems->getItems().size() - 1);
         int itemIndex = itemDist(rng);
         Item *item = &(sellerItems->getItems()[itemIndex]);

         // jumlah & hitung total harga
         int quantity = qtyDist(rng);
         double totalPrice = item->getPrice() * quantity;

         int daysAgo;
         if (i < todayCount)
         {
            daysAgo = 0;
         }
         else if (i < todayCount + lastWeekCount)
         {
            daysAgo = weekDist(rng);
         }
         else
         {
            daysAgo = olderDist(rng);
         }
         auto transactionTimestamp = system_clock::now() - hours(24 * daysAgo);

         uniform_int_distribution<int> statusDist(1, 100);
         int statusRoll = statusDist(rng);

         // 70% COMPLETED, 25% PAID, 5% CANCELED
         OrderStatus tStatus;
         if (statusRoll <= 70)
         {
            tStatus = COMPLETED;
         }
         else if (statusRoll <= 95)
         {
            tStatus = PAID;
         }
         else
         {
            tStatus = CANCELED;
         }

         unsigned int transactionId = nextTransactionId++;
         Transaction newTransaction(
             transactionId,
             buyer->getId(),
             seller->getBuyer()->getId(),
             item->getId(),
             item->getName(),
             seller->getStoreName(),
             quantity,
             totalPrice, transactionTimestamp, tStatus);

         Items *sellerStoreItems = seller->getStoreItems();
         if (sellerStoreItems)
         {
            bool itemExists = false;
            vector<Item> &items = sellerStoreItems->getItems();

            for (const auto &existingItem : items)
            {
               if (existingItem.getId() == item->getId())
               {
                  itemExists = true;
                  break;
               }
            }

            if (!itemExists)
            {
               uniform_int_distribution<int> stockDist(0, 10);
               int randomStock = stockDist(rng);

               Item newStoreItem(item->getId(), item->getName(), item->getPrice(), randomStock);
               sellerStoreItems->addItem(newStoreItem);
               cout << " -> Added item '" << item->getName() << "' to seller " << seller->getStoreName() << "'s store" << endl;
            }
         }

         if (tStatus != CANCELED)
         {
            buyer->getCustomer()->addBankHistoryRecord(BankTransactionType::WITHDRAWAL, totalPrice, transactionTimestamp);
            seller->getCustomerAccount()->addBankHistoryRecord(BankTransactionType::DEPOSIT, totalPrice, transactionTimestamp);
         }
      }

      cout << "Database seeded successfully!" << endl;
      cout << "----------------------------------------" << endl;
   }
};

#endif