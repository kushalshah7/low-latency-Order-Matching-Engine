#include "OrderBook.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

OrderBook::OrderBook(const std::string& symbol) 
    : symbol_(symbol), nextTradeId_(1) {}

std::vector<Trade> OrderBook::addOrder(std::shared_ptr<Order> order) {
    std::vector<Trade> trades;
    
    if (order->type == OrderType::LIMIT) {
        trades = matchLimitOrder(order);
    } else {
        trades = matchMarketOrder(order);
    }
    
    // Add to book if not fully filled
    if (!order->isFilled() && order->type == OrderType::LIMIT) {
        addOrderToBook(order);
    }
    
    return trades;
}

std::vector<Trade> OrderBook::matchLimitOrder(std::shared_ptr<Order> order) {
    std::vector<Trade> trades;
    
    if (order->side == OrderSide::BUY) {
        // Match buy order against asks
        while (!order->isFilled() && !asks_.empty()) {
            auto& [price, priceLevel] = *asks_.begin();

            // Check if price crosses
            if (order->price < price) break;

            // Match against orders at this price level
            auto orderIt = priceLevel->orders.begin();
            while (orderIt != priceLevel->orders.end() && !order->isFilled()) {
                auto& bookOrder = *orderIt;
                uint64_t matchQty = std::min(order->getRemainingQuantity(),
                                             bookOrder->getRemainingQuantity());

                Trade trade = executeTrade(order, bookOrder, price, matchQty);
                trades.push_back(trade);

                if (bookOrder->isFilled()) {
                    orderLocation_.erase(bookOrder->orderId);
                    uint64_t eraseQty = bookOrder->quantity;
                    orderIt = priceLevel->orders.erase(orderIt);
                    priceLevel->totalQuantity -= eraseQty;
                } else {
                    ++orderIt;
                }
            }

            if (priceLevel->isEmpty()) {
                asks_.erase(asks_.begin());
            }
        }
    } else {
        // Match sell order against bids
        while (!order->isFilled() && !bids_.empty()) {
            auto& [price, priceLevel] = *bids_.begin();

            // Check if price crosses
            if (order->price > price) break;

            // Match against orders at this price level
            auto orderIt = priceLevel->orders.begin();
            while (orderIt != priceLevel->orders.end() && !order->isFilled()) {
                auto& bookOrder = *orderIt;
                uint64_t matchQty = std::min(order->getRemainingQuantity(),
                                             bookOrder->getRemainingQuantity());

                Trade trade = executeTrade(bookOrder, order, price, matchQty);
                trades.push_back(trade);

                if (bookOrder->isFilled()) {
                    orderLocation_.erase(bookOrder->orderId);
                    uint64_t eraseQty = bookOrder->quantity;
                    orderIt = priceLevel->orders.erase(orderIt);
                    priceLevel->totalQuantity -= eraseQty;
                } else {
                    ++orderIt;
                }
            }

            if (priceLevel->isEmpty()) {
                bids_.erase(bids_.begin());
            }
        }
    }
    
    return trades;
}

std::vector<Trade> OrderBook::matchMarketOrder(std::shared_ptr<Order> order) {
    std::vector<Trade> trades;
    
    if (order->side == OrderSide::BUY) {
        // Match buy order against asks
        while (!order->isFilled() && !asks_.empty()) {
            auto& [price, priceLevel] = *asks_.begin();

            auto orderIt = priceLevel->orders.begin();
            while (orderIt != priceLevel->orders.end() && !order->isFilled()) {
                auto& bookOrder = *orderIt;
                uint64_t matchQty = std::min(order->getRemainingQuantity(),
                                             bookOrder->getRemainingQuantity());

                Trade trade = executeTrade(order, bookOrder, price, matchQty);
                trades.push_back(trade);

                if (bookOrder->isFilled()) {
                    orderLocation_.erase(bookOrder->orderId);
                    uint64_t eraseQty = bookOrder->quantity;
                    orderIt = priceLevel->orders.erase(orderIt);
                    priceLevel->totalQuantity -= eraseQty;
                } else {
                    ++orderIt;
                }
            }

            if (priceLevel->isEmpty()) {
                asks_.erase(asks_.begin());
            }
        }
    } else {
        // Match sell order against bids
        while (!order->isFilled() && !bids_.empty()) {
            auto& [price, priceLevel] = *bids_.begin();

            auto orderIt = priceLevel->orders.begin();
            while (orderIt != priceLevel->orders.end() && !order->isFilled()) {
                auto& bookOrder = *orderIt;
                uint64_t matchQty = std::min(order->getRemainingQuantity(),
                                             bookOrder->getRemainingQuantity());

                Trade trade = executeTrade(bookOrder, order, price, matchQty);
                trades.push_back(trade);

                if (bookOrder->isFilled()) {
                    orderLocation_.erase(bookOrder->orderId);
                    uint64_t eraseQty = bookOrder->quantity;
                    orderIt = priceLevel->orders.erase(orderIt);
                    priceLevel->totalQuantity -= eraseQty;
                } else {
                    ++orderIt;
                }
            }

            if (priceLevel->isEmpty()) {
                bids_.erase(bids_.begin());
            }
        }
    }
    
    if (!order->isFilled()) {
        order->status = OrderStatus::PARTIALLY_FILLED;
    }
    
    return trades;
}

void OrderBook::addOrderToBook(std::shared_ptr<Order> order) {
    if (order->side == OrderSide::BUY) {
        // Add to bids
        if (bids_.find(order->price) == bids_.end()) {
            bids_[order->price] = std::make_shared<PriceLevel>(order->price);
        }
        
        auto& priceLevel = bids_[order->price];
        priceLevel->addOrder(order);
        
        orderMap_[order->orderId] = order;
        auto it = std::prev(priceLevel->orders.end());
        orderLocation_[order->orderId] = {order->price, it};
    } else {
        // Add to asks
        if (asks_.find(order->price) == asks_.end()) {
            asks_[order->price] = std::make_shared<PriceLevel>(order->price);
        }
        
        auto& priceLevel = asks_[order->price];
        priceLevel->addOrder(order);
        
        orderMap_[order->orderId] = order;
        auto it = std::prev(priceLevel->orders.end());
        orderLocation_[order->orderId] = {order->price, it};
    }
}

Trade OrderBook::executeTrade(std::shared_ptr<Order> buyOrder, 
                              std::shared_ptr<Order> sellOrder,
                              double price, uint64_t quantity) {
    buyOrder->filledQuantity += quantity;
    sellOrder->filledQuantity += quantity;
    
    if (buyOrder->isFilled()) {
        buyOrder->status = OrderStatus::FILLED;
    } else {
        buyOrder->status = OrderStatus::PARTIALLY_FILLED;
    }
    
    if (sellOrder->isFilled()) {
        sellOrder->status = OrderStatus::FILLED;
    } else {
        sellOrder->status = OrderStatus::PARTIALLY_FILLED;
    }
    
    Trade trade(nextTradeId_++, buyOrder->orderId, sellOrder->orderId,
                symbol_, price, quantity);
    trades_.push_back(trade);
    
    return trade;
}

bool OrderBook::cancelOrder(uint64_t orderId) {
    auto it = orderLocation_.find(orderId);
    if (it == orderLocation_.end()) {
        return false;
    }

    auto order = orderMap_[orderId];
    double price = it->second.first;
    auto orderIt = it->second.second;

    if (!order) {
        std::cerr << "Error: Null order pointer in cancelOrder." << std::endl;
        return false;
    }

    if (order->side == OrderSide::BUY) {
        auto priceLevelIt = bids_.find(price);
        if (priceLevelIt == bids_.end()) {
            std::cerr << "Error: Price level not found in bids during cancelOrder." << std::endl;
            return false;
        }
        auto& priceLevel = priceLevelIt->second;
        priceLevel->removeOrder(orderIt);
        if (priceLevel->isEmpty()) {
            bids_.erase(priceLevelIt);
        }
    } else {
        auto priceLevelIt = asks_.find(price);
        if (priceLevelIt == asks_.end()) {
            std::cerr << "Error: Price level not found in asks during cancelOrder." << std::endl;
            return false;
        }
        auto& priceLevel = priceLevelIt->second;
        priceLevel->removeOrder(orderIt);
        if (priceLevel->isEmpty()) {
            asks_.erase(priceLevelIt);
        }
    }

    order->status = OrderStatus::CANCELLED;
    orderMap_.erase(orderId);
    orderLocation_.erase(orderId);

    return true;
}

std::shared_ptr<Order> OrderBook::getOrder(uint64_t orderId) const {
    auto it = orderMap_.find(orderId);
    return (it != orderMap_.end()) ? it->second : nullptr;
}

double OrderBook::getBestBid() const {
    return bids_.empty() ? 0.0 : bids_.begin()->first;
}

double OrderBook::getBestAsk() const {
    return asks_.empty() ? 0.0 : asks_.begin()->first;
}

double OrderBook::getMidPrice() const {
    if (bids_.empty() || asks_.empty()) return 0.0;
    return (getBestBid() + getBestAsk()) / 2.0;
}

double OrderBook::getSpread() const {
    if (bids_.empty() || asks_.empty()) return 0.0;
    return getBestAsk() - getBestBid();
}

uint64_t OrderBook::getBidVolume() const {
    uint64_t total = 0;
    for (const auto& [price, level] : bids_) {
        total += level->totalQuantity;
    }
    return total;
}

uint64_t OrderBook::getAskVolume() const {
    uint64_t total = 0;
    for (const auto& [price, level] : asks_) {
        total += level->totalQuantity;
    }
    return total;
}

void OrderBook::printBook(int levels) const {
    std::cout << "\n=== Order Book: " << symbol_ << " ===" << std::endl;
    std::cout << "Best Bid: " << getBestBid() << " | Best Ask: " << getBestAsk() << std::endl;
    std::cout << "Spread: " << getSpread() << " | Mid: " << getMidPrice() << std::endl;
    std::cout << "\nAsks (Sell Orders):" << std::endl;
    
    int count = 0;
    for (auto it = asks_.rbegin(); it != asks_.rend() && count < levels; ++it, ++count) {
        std::cout << std::fixed << std::setprecision(2)
                  << "  " << it->first << " x " << it->second->totalQuantity 
                  << " (" << it->second->orders.size() << " orders)" << std::endl;
    }
    
    std::cout << "  --------------------" << std::endl;
    
    count = 0;
    for (const auto& [price, level] : bids_) {
        if (count++ >= levels) break;
        std::cout << std::fixed << std::setprecision(2)
                  << "  " << price << " x " << level->totalQuantity 
                  << " (" << level->orders.size() << " orders)" << std::endl;
    }
    
    std::cout << "\nBids (Buy Orders)" << std::endl;
}