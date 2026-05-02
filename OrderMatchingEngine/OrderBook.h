#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include "Order.h"
#include <map>
#include <unordered_map>
#include <list>
#include <memory>
#include <vector>

// Price level containing all orders at a specific price
struct PriceLevel {
    double price;
    uint64_t totalQuantity;
    std::list<std::shared_ptr<Order>> orders;
    
    PriceLevel(double p) : price(p), totalQuantity(0) {}
    
    void addOrder(std::shared_ptr<Order> order) {
        orders.push_back(order);
        totalQuantity += order->getRemainingQuantity();
    }
    
    void removeOrder(std::list<std::shared_ptr<Order>>::iterator it) {
        // Subtract the original order quantity, not remaining
        totalQuantity -= (*it)->quantity;
        orders.erase(it);
    }
    
    bool isEmpty() const {
        return orders.empty();
    }
};

class OrderBook {
private:
    std::string symbol_;
    
    // Buy side: sorted descending (highest price first)
    std::map<double, std::shared_ptr<PriceLevel>, std::greater<double>> bids_;
    
    // Sell side: sorted ascending (lowest price first)
    std::map<double, std::shared_ptr<PriceLevel>> asks_;
    
    // Fast lookup for order cancellation
    std::unordered_map<uint64_t, std::shared_ptr<Order>> orderMap_;
    std::unordered_map<uint64_t, std::pair<double, std::list<std::shared_ptr<Order>>::iterator>> orderLocation_;
    
    std::vector<Trade> trades_;
    uint64_t nextTradeId_;
    
    // Internal matching logic
    std::vector<Trade> matchLimitOrder(std::shared_ptr<Order> order);
    std::vector<Trade> matchMarketOrder(std::shared_ptr<Order> order);
    void addOrderToBook(std::shared_ptr<Order> order);
    Trade executeTrade(std::shared_ptr<Order> buyOrder, std::shared_ptr<Order> sellOrder, 
                       double price, uint64_t quantity);
    
public:
    OrderBook(const std::string& symbol);
    
    // Core operations
    std::vector<Trade> addOrder(std::shared_ptr<Order> order);
    bool cancelOrder(uint64_t orderId);
    
    // Query functions
    std::shared_ptr<Order> getOrder(uint64_t orderId) const;
    double getBestBid() const;
    double getBestAsk() const;
    double getMidPrice() const;
    double getSpread() const;
    uint64_t getBidVolume() const;
    uint64_t getAskVolume() const;
    
    // Debug/monitoring
    void printBook(int levels = 5) const;
    const std::vector<Trade>& getTrades() const { return trades_; }
};

#endif // ORDERBOOK_H