#ifndef ORDER_H
#define ORDER_H

#include <string>
#include <cstdint>
#include <chrono>

enum class OrderType {
    LIMIT,
    MARKET
};

enum class OrderSide {
    BUY,
    SELL
};

enum class OrderStatus {
    NEW,
    PARTIALLY_FILLED,
    FILLED,
    CANCELLED
};

struct Order {
    uint64_t orderId;
    std::string symbol;
    OrderType type;
    OrderSide side;
    double price;
    uint64_t quantity;
    uint64_t filledQuantity;
    OrderStatus status;
    std::chrono::high_resolution_clock::time_point timestamp;
    
    Order(uint64_t id, const std::string& sym, OrderType t, OrderSide s, 
          double p, uint64_t q)
        : orderId(id), symbol(sym), type(t), side(s), price(p), 
          quantity(q), filledQuantity(0), status(OrderStatus::NEW),
          timestamp(std::chrono::high_resolution_clock::now()) {}
    
    uint64_t getRemainingQuantity() const {
        return quantity - filledQuantity;
    }
    
    bool isFilled() const {
        return filledQuantity >= quantity;
    }
};

struct Trade {
    uint64_t tradeId;
    uint64_t buyOrderId;
    uint64_t sellOrderId;
    std::string symbol;
    double price;
    uint64_t quantity;
    std::chrono::high_resolution_clock::time_point timestamp;
    
    Trade(uint64_t id, uint64_t buyId, uint64_t sellId, 
          const std::string& sym, double p, uint64_t q)
        : tradeId(id), buyOrderId(buyId), sellOrderId(sellId), 
          symbol(sym), price(p), quantity(q),
          timestamp(std::chrono::high_resolution_clock::now()) {}
};

#endif // ORDER_H