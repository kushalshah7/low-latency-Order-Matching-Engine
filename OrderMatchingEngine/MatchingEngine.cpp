#include "MatchingEngine.h"
#include <iostream>
#include <algorithm>
#include <iomanip>

MatchingEngine::MatchingEngine() : nextOrderId_(1) {}

void MatchingEngine::addSymbol(const std::string& symbol) {
    if (books_.find(symbol) == books_.end()) {
        books_[symbol] = std::make_shared<OrderBook>(symbol);
    }
}

std::pair<std::shared_ptr<Order>, std::vector<Trade>> 
MatchingEngine::submitLimitOrder(const std::string& symbol, OrderSide side,
                                 double price, uint64_t quantity) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    addSymbol(symbol);
    
    auto order = std::make_shared<Order>(nextOrderId_++, symbol, 
                                         OrderType::LIMIT, side, price, quantity);
    
    auto trades = books_[symbol]->addOrder(order);
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        endTime - startTime).count();
    
    stats_.addProcessingTime(duration);
    stats_.totalTrades += trades.size();
    
    return {order, trades};
}

std::pair<std::shared_ptr<Order>, std::vector<Trade>> 
MatchingEngine::submitMarketOrder(const std::string& symbol, OrderSide side,
                                  uint64_t quantity) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    addSymbol(symbol);
    
    auto order = std::make_shared<Order>(nextOrderId_++, symbol,
                                         OrderType::MARKET, side, 0.0, quantity);
    
    auto trades = books_[symbol]->addOrder(order);
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        endTime - startTime).count();
    
    stats_.addProcessingTime(duration);
    stats_.totalTrades += trades.size();
    
    return {order, trades};
}

bool MatchingEngine::cancelOrder(const std::string& symbol, uint64_t orderId) {
    auto it = books_.find(symbol);
    if (it == books_.end()) {
        return false;
    }
    
    return it->second->cancelOrder(orderId);
}

std::shared_ptr<OrderBook> MatchingEngine::getOrderBook(const std::string& symbol) {
    auto it = books_.find(symbol);
    return (it != books_.end()) ? it->second : nullptr;
}

std::shared_ptr<Order> MatchingEngine::getOrder(const std::string& symbol, 
                                                uint64_t orderId) {
    auto book = getOrderBook(symbol);
    return book ? book->getOrder(orderId) : nullptr;
}

void MatchingEngine::resetStats() {
    stats_ = PerformanceStats();
}

void PerformanceStats::calculateStats() {
    if (processingTimes.empty()) return;
    
    double sum = 0.0;
    for (double time : processingTimes) {
        sum += time;
    }
    avgProcessingTime = sum / processingTimes.size();
    
    std::vector<double> sorted = processingTimes;
    std::sort(sorted.begin(), sorted.end());
    
    size_t mid = sorted.size() / 2;
    if (sorted.size() % 2 == 0) {
        medianProcessingTime = (sorted[mid - 1] + sorted[mid]) / 2.0;
    } else {
        medianProcessingTime = sorted[mid];
    }
}

void PerformanceStats::print() const {
    std::cout << "\n=== Performance Statistics ===" << std::endl;
    std::cout << "Total Orders Processed: " << totalOrders << std::endl;
    std::cout << "Total Trades Executed: " << totalTrades << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Average Processing Time: " << avgProcessingTime << " μs" << std::endl;
    std::cout << "Median Processing Time: " << medianProcessingTime << " μs" << std::endl;
    std::cout << "Min Processing Time: " << minProcessingTime << " μs" << std::endl;
    std::cout << "Max Processing Time: " << maxProcessingTime << " μs" << std::endl;
    std::cout << "==============================\n" << std::endl;
}