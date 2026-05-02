#include "MatchingEngine.h"
#include <iostream>
#include <cassert>
#include <vector>

// Test framework helpers
int totalTests = 0;
int passedTests = 0;

void testPass(const std::string& testName) {
    std::cout << "✓ " << testName << " PASSED" << std::endl;
    totalTests++;
    passedTests++;
}

void testFail(const std::string& testName, const std::string& reason) {
    std::cout << "✗ " << testName << " FAILED: " << reason << std::endl;
    totalTests++;
}

#define ASSERT_TRUE(condition, message) \
    if (!(condition)) { \
        testFail(__func__, message); \
        return; \
    }

#define ASSERT_EQ(actual, expected, message) \
    if ((actual) != (expected)) { \
        testFail(__func__, message); \
        return; \
    }

// Test 1: Basic Limit Order Add
void testBasicLimitOrder() {
    MatchingEngine engine;
    
    auto [order, trades] = engine.submitLimitOrder("AAPL", OrderSide::BUY, 150.0, 100);
    
    ASSERT_TRUE(order != nullptr, "Order should be created");
    ASSERT_EQ(trades.size(), 0, "No trades should occur");
    ASSERT_EQ(order->status, OrderStatus::NEW, "Order should be NEW");
    
    auto book = engine.getOrderBook("AAPL");
    ASSERT_EQ(book->getBestBid(), 150.0, "Best bid should be 150.0");
    
    testPass(__func__);
}

// Test 2: Immediate Full Match
void testImmediateFullMatch() {
    MatchingEngine engine;
    
    // Place buy order
    auto [buyOrder, trades1] = engine.submitLimitOrder("AAPL", OrderSide::BUY, 150.0, 100);
    
    // Place matching sell order
    auto [sellOrder, trades2] = engine.submitLimitOrder("AAPL", OrderSide::SELL, 150.0, 100);
    
    ASSERT_EQ(trades2.size(), 1, "Should have 1 trade");
    ASSERT_EQ(buyOrder->status, OrderStatus::FILLED, "Buy order should be filled");
    ASSERT_EQ(sellOrder->status, OrderStatus::FILLED, "Sell order should be filled");
    ASSERT_EQ(trades2[0].quantity, 100, "Trade quantity should be 100");
    ASSERT_EQ(trades2[0].price, 150.0, "Trade price should be 150.0");
    
    testPass(__func__);
}

// Test 3: Partial Fill
void testPartialFill() {
    MatchingEngine engine;
    
    auto [buyOrder, trades1] = engine.submitLimitOrder("AAPL", OrderSide::BUY, 150.0, 100);
    auto [sellOrder, trades2] = engine.submitLimitOrder("AAPL", OrderSide::SELL, 150.0, 50);
    
    ASSERT_EQ(trades2.size(), 1, "Should have 1 trade");
    ASSERT_EQ(sellOrder->status, OrderStatus::FILLED, "Sell order should be fully filled");
    ASSERT_EQ(buyOrder->status, OrderStatus::PARTIALLY_FILLED, "Buy order should be partially filled");
    ASSERT_EQ(buyOrder->filledQuantity, 50, "50 shares should be filled");
    ASSERT_EQ(buyOrder->getRemainingQuantity(), 50, "50 shares should remain");
    
    testPass(__func__);
}

// Test 4: Multiple Price Levels
void testMultiplePriceLevels() {
    MatchingEngine engine;
    
    engine.submitLimitOrder("AAPL", OrderSide::BUY, 150.0, 100);
    engine.submitLimitOrder("AAPL", OrderSide::BUY, 149.0, 100);
    engine.submitLimitOrder("AAPL", OrderSide::SELL, 151.0, 100);
    engine.submitLimitOrder("AAPL", OrderSide::SELL, 152.0, 100);
    
    auto book = engine.getOrderBook("AAPL");
    ASSERT_EQ(book->getBestBid(), 150.0, "Best bid should be 150.0");
    ASSERT_EQ(book->getBestAsk(), 151.0, "Best ask should be 151.0");
    ASSERT_EQ(book->getSpread(), 1.0, "Spread should be 1.0");
    
    testPass(__func__);
}

// Test 5: Market Order
void testMarketOrder() {
    MatchingEngine engine;
    
    engine.submitLimitOrder("AAPL", OrderSide::SELL, 150.0, 50);
    engine.submitLimitOrder("AAPL", OrderSide::SELL, 151.0, 50);
    
    auto [order, trades] = engine.submitMarketOrder("AAPL", OrderSide::BUY, 75);
    
    ASSERT_EQ(trades.size(), 2, "Should have 2 trades");
    ASSERT_EQ(trades[0].quantity, 50, "First trade should be 50 shares");
    ASSERT_EQ(trades[1].quantity, 25, "Second trade should be 25 shares");
    ASSERT_EQ(trades[0].price, 150.0, "First trade at 150.0");
    ASSERT_EQ(trades[1].price, 151.0, "Second trade at 151.0");
    
    testPass(__func__);
}

// Test 6: Order Cancellation
void testOrderCancellation() {
    MatchingEngine engine;
    
    auto [order, trades] = engine.submitLimitOrder("AAPL", OrderSide::BUY, 150.0, 100);
    uint64_t orderId = order->orderId;
    
    auto book = engine.getOrderBook("AAPL");
    ASSERT_EQ(book->getBestBid(), 150.0, "Best bid should be 150.0");
    
    bool cancelled = engine.cancelOrder("AAPL", orderId);
    ASSERT_TRUE(cancelled, "Cancel should succeed");
    ASSERT_EQ(book->getBestBid(), 0.0, "No bids should remain");
    
    testPass(__func__);
}

// Test 7: Price-Time Priority
void testPriceTimePriority() {
    MatchingEngine engine;
    
    auto [order1, t1] = engine.submitLimitOrder("AAPL", OrderSide::BUY, 150.0, 100);
    auto [order2, t2] = engine.submitLimitOrder("AAPL", OrderSide::BUY, 150.0, 100);
    
    auto [sellOrder, trades] = engine.submitLimitOrder("AAPL", OrderSide::SELL, 150.0, 150);
    
    ASSERT_EQ(trades.size(), 2, "Should have 2 trades");
    ASSERT_EQ(order1->filledQuantity, 100, "First order should be fully filled");
    ASSERT_EQ(order2->filledQuantity, 50, "Second order should be partially filled");
    ASSERT_EQ(trades[0].buyOrderId, order1->orderId, "First trade with order1");
    ASSERT_EQ(trades[1].buyOrderId, order2->orderId, "Second trade with order2");
    
    testPass(__func__);
}

// Test 8: High-Frequency Order Flow
void testHighFrequencyFlow() {
    MatchingEngine engine;
    
    // Simulate high-frequency trading
    for (int i = 0; i < 1000; i++) {
        double price = 150.0 + (i % 10) * 0.1;
        engine.submitLimitOrder("AAPL", OrderSide::BUY, price, 10);
        engine.submitLimitOrder("AAPL", OrderSide::SELL, price + 0.5, 10);
    }
    
    auto book = engine.getOrderBook("AAPL");
    ASSERT_TRUE(book->getBidVolume() > 0, "Should have bid volume");
    ASSERT_TRUE(book->getAskVolume() > 0, "Should have ask volume");
    
    testPass(__func__);
}

// Test 9: Order Book Depth
void testOrderBookDepth() {
    MatchingEngine engine;
    
    for (int i = 0; i < 5; i++) {
        engine.submitLimitOrder("AAPL", OrderSide::BUY, 150.0 - i, 100);
        engine.submitLimitOrder("AAPL", OrderSide::SELL, 151.0 + i, 100);
    }
    
    auto book = engine.getOrderBook("AAPL");
    ASSERT_EQ(book->getBidVolume(), 500, "Total bid volume should be 500");
    ASSERT_EQ(book->getAskVolume(), 500, "Total ask volume should be 500");
    
    testPass(__func__);
}

// Test 10: Aggressive Market Order
void testAggressiveMarketOrder() {
    MatchingEngine engine;
    
    engine.submitLimitOrder("AAPL", OrderSide::SELL, 150.0, 100);
    engine.submitLimitOrder("AAPL", OrderSide::SELL, 151.0, 100);
    engine.submitLimitOrder("AAPL", OrderSide::SELL, 152.0, 100);
    
    auto [order, trades] = engine.submitMarketOrder("AAPL", OrderSide::BUY, 250);
    
    ASSERT_EQ(trades.size(), 3, "Should match across 3 price levels");
    ASSERT_EQ(order->filledQuantity, 250, "All 250 shares should be filled");
    ASSERT_EQ(order->status, OrderStatus::FILLED, "Order should be fully filled");
    
    testPass(__func__);
}

// Test 11: Empty Book Market Order
void testEmptyBookMarketOrder() {
    MatchingEngine engine;
    
    auto [order, trades] = engine.submitMarketOrder("AAPL", OrderSide::BUY, 100);
    
    ASSERT_EQ(trades.size(), 0, "No trades should occur");
    ASSERT_EQ(order->filledQuantity, 0, "No fills");
    ASSERT_EQ(order->status, OrderStatus::PARTIALLY_FILLED, "Should be partially filled (0%)");
    
    testPass(__func__);
}

// Test 12: Cancel Non-Existent Order
void testCancelNonExistent() {
    MatchingEngine engine;
    
    bool cancelled = engine.cancelOrder("AAPL", 99999);
    ASSERT_TRUE(!cancelled, "Cancel should fail for non-existent order");
    
    testPass(__func__);
}

// Test 13: Multiple Symbols
void testMultipleSymbols() {
    MatchingEngine engine;
    
    engine.submitLimitOrder("AAPL", OrderSide::BUY, 150.0, 100);
    engine.submitLimitOrder("GOOGL", OrderSide::BUY, 2800.0, 50);
    engine.submitLimitOrder("MSFT", OrderSide::SELL, 380.0, 75);
    
    auto appleBook = engine.getOrderBook("AAPL");
    auto googleBook = engine.getOrderBook("GOOGL");
    auto msftBook = engine.getOrderBook("MSFT");
    
    ASSERT_EQ(appleBook->getBestBid(), 150.0, "AAPL best bid");
    ASSERT_EQ(googleBook->getBestBid(), 2800.0, "GOOGL best bid");
    ASSERT_EQ(msftBook->getBestAsk(), 380.0, "MSFT best ask");
    
    testPass(__func__);
}

// Test 14: Large Order Matching
void testLargeOrderMatching() {
    MatchingEngine engine;
    
    // Build book with multiple orders
    for (int i = 0; i < 10; i++) {
        engine.submitLimitOrder("AAPL", OrderSide::SELL, 150.0, 100);
    }
    
    auto [order, trades] = engine.submitMarketOrder("AAPL", OrderSide::BUY, 1000);
    
    ASSERT_EQ(trades.size(), 10, "Should match all 10 orders");
    ASSERT_EQ(order->filledQuantity, 1000, "All 1000 shares filled");
    
    testPass(__func__);
}

// Test 15: Performance Stress Test
void testPerformanceStress() {
    MatchingEngine engine;
    engine.resetStats();
    
    // Submit 10000 orders
    for (int i = 0; i < 5000; i++) {
        double price = 150.0 + (rand() % 100) * 0.01;
        engine.submitLimitOrder("AAPL", OrderSide::BUY, price, 10 + rand() % 90);
        engine.submitLimitOrder("AAPL", OrderSide::SELL, price + 0.5, 10 + rand() % 90);
    }
    
    auto stats = engine.getStats();
    ASSERT_EQ(stats.totalOrders, 10000, "Should process 10000 orders");
    ASSERT_TRUE(stats.medianProcessingTime < 100.0, "Median time should be under 100 μs");
    
    std::cout << "  Performance: " << stats.medianProcessingTime << " μs median" << std::endl;
    
    testPass(__func__);
}

void runAllTests() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  Running Order Matching Engine Tests  " << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    testBasicLimitOrder();
    testImmediateFullMatch();
    testPartialFill();
    testMultiplePriceLevels();
    testMarketOrder();
    testOrderCancellation();
    testPriceTimePriority();
    testHighFrequencyFlow();
    testOrderBookDepth();
    testAggressiveMarketOrder();
    testEmptyBookMarketOrder();
    testCancelNonExistent();
    testMultipleSymbols();
    testLargeOrderMatching();
    testPerformanceStress();
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "  Test Results: " << passedTests << "/" << totalTests << " passed" << std::endl;
    std::cout << "========================================\n" << std::endl;
}

int main() {
    runAllTests();
    return (passedTests == totalTests) ? 0 : 1;
}