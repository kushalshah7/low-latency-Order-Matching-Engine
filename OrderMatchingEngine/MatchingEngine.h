#ifndef MATCHINGENGINE_H
#define MATCHINGENGINE_H

#include "OrderBook.h"
#include <unordered_map>
#include <memory>
#include <chrono>
#include <vector>

struct PerformanceStats {
    uint64_t totalOrders;
    uint64_t totalTrades;
    double avgProcessingTime;
    double medianProcessingTime;
    double minProcessingTime;
    double maxProcessingTime;
    std::vector<double> processingTimes;
    
    PerformanceStats() : totalOrders(0), totalTrades(0), avgProcessingTime(0.0),
                         medianProcessingTime(0.0), minProcessingTime(0.0),
                         maxProcessingTime(0.0) {}
    
    void addProcessingTime(double time) {
        processingTimes.push_back(time);
        totalOrders++;
        
        if (minProcessingTime == 0.0 || time < minProcessingTime) {
            minProcessingTime = time;
        }
        if (time > maxProcessingTime) {
            maxProcessingTime = time;
        }
    }
    
    void calculateStats();
    void print() const;
};

class MatchingEngine {
private:
    std::unordered_map<std::string, std::shared_ptr<OrderBook>> books_;
    uint64_t nextOrderId_;
    PerformanceStats stats_;
    
public:
    MatchingEngine();
    
    // Create order book for symbol
    void addSymbol(const std::string& symbol);
    
    // Submit orders
    std::pair<std::shared_ptr<Order>, std::vector<Trade>> 
    submitLimitOrder(const std::string& symbol, OrderSide side, 
                    double price, uint64_t quantity);
    
    std::pair<std::shared_ptr<Order>, std::vector<Trade>> 
    submitMarketOrder(const std::string& symbol, OrderSide side, 
                     uint64_t quantity);
    
    // Cancel order
    bool cancelOrder(const std::string& symbol, uint64_t orderId);
    
    // Query functions
    std::shared_ptr<OrderBook> getOrderBook(const std::string& symbol);
    std::shared_ptr<Order> getOrder(const std::string& symbol, uint64_t orderId);
    
    // Performance monitoring
    const PerformanceStats& getStats() const { return stats_; }
    void resetStats();
    void printStats() const { stats_.print(); }
};

#endif // MATCHINGENGINE_H