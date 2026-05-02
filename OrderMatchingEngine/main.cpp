#include "MatchingEngine.h"
#include <iostream>
#include <thread>
#include <chrono>

void printSeparator() {
    std::cout << "\n================================================\n" << std::endl;
}

void demoBasicTrading() {
    std::cout << "DEMO 1: Basic Trading Scenario" << std::endl;
    printSeparator();
    
    MatchingEngine engine;
    
    std::cout << "Submitting buy orders..." << std::endl;
    auto [buy1, t1] = engine.submitLimitOrder("AAPL", OrderSide::BUY, 150.00, 100);
    auto [buy2, t2] = engine.submitLimitOrder("AAPL", OrderSide::BUY, 149.50, 200);
    auto [buy3, t3] = engine.submitLimitOrder("AAPL", OrderSide::BUY, 149.00, 150);
    
    std::cout << "Submitting sell orders..." << std::endl;
    auto [sell1, t4] = engine.submitLimitOrder("AAPL", OrderSide::SELL, 150.50, 100);
    auto [sell2, t5] = engine.submitLimitOrder("AAPL", OrderSide::SELL, 151.00, 200);
    
    auto book = engine.getOrderBook("AAPL");
    book->printBook();
    
    std::cout << "\nSubmitting aggressive sell order to cross the spread..." << std::endl;
    auto [aggSell, trades] = engine.submitLimitOrder("AAPL", OrderSide::SELL, 149.50, 250);
    
    std::cout << "\nTrades executed: " << trades.size() << std::endl;
    for (const auto& trade : trades) {
        std::cout << "  Trade #" << trade.tradeId << ": "
                  << trade.quantity << " shares @ $" << trade.price << std::endl;
    }
    
    book->printBook();
}

void demoMarketOrders() {
    std::cout << "\n\nDEMO 2: Market Orders" << std::endl;
    printSeparator();
    
    MatchingEngine engine;
    
    // Build liquidity
    engine.submitLimitOrder("GOOGL", OrderSide::SELL, 2800.0, 50);
    engine.submitLimitOrder("GOOGL", OrderSide::SELL, 2801.0, 75);
    engine.submitLimitOrder("GOOGL", OrderSide::SELL, 2802.0, 100);
    
    auto book = engine.getOrderBook("GOOGL");
    std::cout << "Order book before market order:" << std::endl;
    book->printBook();
    
    std::cout << "\nSubmitting market buy order for 150 shares..." << std::endl;
    auto [order, trades] = engine.submitMarketOrder("GOOGL", OrderSide::BUY, 150);
    
    std::cout << "\nMarket order execution:" << std::endl;
    std::cout << "  Total fills: " << trades.size() << std::endl;
    for (const auto& trade : trades) {
        std::cout << "    " << trade.quantity << " shares @ $" << trade.price << std::endl;
    }
    
    double avgPrice = 0.0;
    for (const auto& trade : trades) {
        avgPrice += trade.price * trade.quantity;
    }
    avgPrice /= order->filledQuantity;
    std::cout << "  Average fill price: $" << avgPrice << std::endl;
    
    book->printBook();
}

void demoOrderCancellation() {
    std::cout << "\n\nDEMO 3: Order Management & Cancellation" << std::endl;
    printSeparator();
    
    MatchingEngine engine;
    
    auto [order1, t1] = engine.submitLimitOrder("MSFT", OrderSide::BUY, 380.0, 100);
    auto [order2, t2] = engine.submitLimitOrder("MSFT", OrderSide::BUY, 379.5, 150);
    auto [order3, t3] = engine.submitLimitOrder("MSFT", OrderSide::BUY, 379.0, 200);
    
    std::cout << "Submitted 3 buy orders" << std::endl;
    auto book = engine.getOrderBook("MSFT");
    book->printBook();
    
    std::cout << "\nCancelling middle order (ID: " << order2->orderId << ")..." << std::endl;
    bool cancelled = engine.cancelOrder("MSFT", order2->orderId);
    std::cout << "Cancellation " << (cancelled ? "successful" : "failed") << std::endl;
    
    book->printBook();
}

void demoHighFrequency() {
    std::cout << "\n\nDEMO 4: High-Frequency Simulation" << std::endl;
    printSeparator();
    
    MatchingEngine engine;
    engine.resetStats();
    
    std::cout << "Simulating 1000 high-frequency orders..." << std::endl;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 500; i++) {
        double basePrice = 100.0;
        double offset = (i % 20) * 0.01;
        
        engine.submitLimitOrder("HFT", OrderSide::BUY, basePrice - offset, 10);
        engine.submitLimitOrder("HFT", OrderSide::SELL, basePrice + offset + 0.05, 10);
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();
    
    std::cout << "Completed in " << totalTime << " ms" << std::endl;
    
    auto stats = engine.getStats();
    const_cast<PerformanceStats&>(stats).calculateStats();
    stats.print();
    
    auto book = engine.getOrderBook("HFT");
    std::cout << "Final book state:" << std::endl;
    std::cout << "  Bid volume: " << book->getBidVolume() << " shares" << std::endl;
    std::cout << "  Ask volume: " << book->getAskVolume() << " shares" << std::endl;
    std::cout << "  Spread: $" << book->getSpread() << std::endl;
}

void demoMultiSymbol() {
    std::cout << "\n\nDEMO 5: Multi-Symbol Trading" << std::endl;
    printSeparator();
    
    MatchingEngine engine;
    
    std::vector<std::string> symbols = {"AAPL", "GOOGL", "MSFT", "TSLA", "AMZN"};
    
    for (const auto& symbol : symbols) {
        double basePrice = 100.0 + rand() % 2000;
        engine.submitLimitOrder(symbol, OrderSide::BUY, basePrice, 100);
        engine.submitLimitOrder(symbol, OrderSide::SELL, basePrice + 1.0, 100);
    }
    
    std::cout << "Created order books for " << symbols.size() << " symbols:\n" << std::endl;
    
    for (const auto& symbol : symbols) {
        auto book = engine.getOrderBook(symbol);
        std::cout << symbol << ": "
                  << "Bid=" << book->getBestBid() 
                  << " Ask=" << book->getBestAsk()
                  << " Spread=" << book->getSpread() << std::endl;
    }
}

int main() {
    std::cout << "\n╔════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  Low-Latency Order Matching Engine Demo  ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════╝" << std::endl;
    
    demoBasicTrading();
    demoMarketOrders();
    demoOrderCancellation();
    demoHighFrequency();
    demoMultiSymbol();
    
    std::cout << "\n\n╔════════════════════════════════════════════╗" << std::endl;
    std::cout << "║           Demo Complete!                   ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════╝\n" << std::endl;
    
    return 0;
}