#ifndef AW2_TIMER_H
#define AW2_TIMER_H

#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <iomanip>
#include <algorithm>

namespace aw2 {

class Timer {
public:
    Timer(const std::string& name = "Timer", Timer* parent = nullptr) 
        : name_(name), parent_(parent), is_running_(false) {
        reset();
        if (parent_) {
            parent_->add_child(this);
        }
    }

    // Start the timer
    void start() {
        if (!is_running_) {
            start_time_ = std::chrono::high_resolution_clock::now();
            is_running_ = true;
        }
    }

    // Pause the timer (accumulates elapsed time)
    void pause() {
        if (is_running_) {
            auto now = std::chrono::high_resolution_clock::now();
            accumulated_time_ += std::chrono::duration_cast<std::chrono::nanoseconds>(now - start_time_);
            is_running_ = false;
        }
    }

    // Resume the timer (continues from where it was paused)
    void resume() {
        start(); // Same as start - will only start if not already running
    }

    // Stop and reset the timer
    void stop() {
        pause();
        reset();
    }

    // Reset the timer to zero
    void reset() {
        accumulated_time_ = std::chrono::nanoseconds::zero();
        is_running_ = false;
    }

    // Create a child timer
    Timer* create_child(const std::string& name) {
        auto child = std::make_unique<Timer>(name, nullptr); // Don't pass parent to avoid double registration
        Timer* child_ptr = child.get();
        child_ptr->parent_ = this; // Set parent manually
        children_.push_back(child_ptr); // Add to children list
        owned_children_.push_back(std::move(child));
        return child_ptr;
    }

    // Add an existing timer as a child (for non-owned children)
    void add_child(Timer* child) {
        // Only add if not already in the list
        auto it = std::find(children_.begin(), children_.end(), child);
        if (it == children_.end()) {
            children_.push_back(child);
        }
    }

    // Get elapsed time in milliseconds
    double elapsed_ms() const {
        auto total_time = accumulated_time_;
        if (is_running_) {
            auto now = std::chrono::high_resolution_clock::now();
            total_time += std::chrono::duration_cast<std::chrono::nanoseconds>(now - start_time_);
        }
        return std::chrono::duration<double, std::milli>(total_time).count();
    }

    // Get elapsed time in microseconds
    double elapsed_us() const {
        auto total_time = accumulated_time_;
        if (is_running_) {
            auto now = std::chrono::high_resolution_clock::now();
            total_time += std::chrono::duration_cast<std::chrono::nanoseconds>(now - start_time_);
        }
        return std::chrono::duration<double, std::micro>(total_time).count();
    }

    // Get elapsed time in seconds
    double elapsed_s() const {
        return elapsed_ms() / 1000.0;
    }

    // Get time spent in children
    double children_time_ms() const {
        double total_children_time = 0.0;
        // Only count from children_ list to avoid double counting
        for (const auto& child : children_) {
            total_children_time += child->elapsed_ms();
        }
        return total_children_time;
    }

    // Get self time (excluding children)
    double self_time_ms() const {
        return elapsed_ms() - children_time_ms();
    }

    // Print elapsed time
    void print_elapsed(const std::string& message = "") const {
        std::string output = name_;
        if (!message.empty()) {
            output += " (" + message + ")";
        }
        output += ": " + std::to_string(elapsed_ms()) + " ms";
        std::cout << output << std::endl;
    }

    // Print hierarchical timing report
    void print_hierarchy(int indent = 0) const {
        std::string prefix(indent * 2, ' ');
        double self_time = self_time_ms();
        double total_time = elapsed_ms();
        
        if (indent == 0) {
            std::cout << prefix << "" << name_ << ": " << total_time << " ms (100.0%)" << std::endl;
        } else {
            double percentage = parent_ ? (total_time / parent_->elapsed_ms()) * 100.0 : 100.0;
            std::cout << prefix << "├─ " << name_ << ": " << total_time << " ms (" 
                      << std::fixed << std::setprecision(1) << percentage << "%)" << std::endl;
        }
        
        // Print children - only from children_ list to avoid duplication
        for (const auto& child : children_) {
            child->print_hierarchy(indent + 1);
        }
        
        // Print self time if there are children
        if (!children_.empty() || !owned_children_.empty()) {
            std::string self_prefix(indent * 2 + 2, ' ');
            double self_percentage = (self_time / total_time) * 100.0;
            std::cout << self_prefix << "└─ [self]: " << self_time << " ms (" 
                      << std::fixed << std::setprecision(1) << self_percentage << "%)" << std::endl;
        }
    }

    // Check if timer is currently running
    bool is_running() const {
        return is_running_;
    }

    // Get the timer name
    const std::string& name() const {
        return name_;
    }

    // Get parent timer
    Timer* parent() const {
        return parent_;
    }

    // Get children
    const std::vector<Timer*>& children() const {
        return children_;
    }

private:
    std::string name_;
    Timer* parent_;
    std::vector<Timer*> children_;  // Non-owned children
    std::vector<std::unique_ptr<Timer>> owned_children_;  // Owned children
    std::chrono::high_resolution_clock::time_point start_time_;
    std::chrono::nanoseconds accumulated_time_;
    bool is_running_;
};

// RAII Timer class - automatically starts on construction and prints on destruction
class ScopedTimer {
public:
    ScopedTimer(const std::string& name, Timer* parent = nullptr) : timer_(name, parent) {
        timer_.start();
    }

    ~ScopedTimer() {
        timer_.pause();
        timer_.print_elapsed();
    }

    Timer& get_timer() {
        return timer_;
    }

private:
    Timer timer_;
};

// Timer Registry for managing hierarchical timers
class TimerRegistry {
public:
    static TimerRegistry& instance() {
        static TimerRegistry instance;
        return instance;
    }

    Timer* create_root_timer(const std::string& name) {
        root_timers_.push_back(std::make_unique<Timer>(name));
        return root_timers_.back().get();
    }

    void print_all_hierarchies() const {
        std::cout << "\n=== IMING REPORT ===" << std::endl;
        for (const auto& timer : root_timers_) {
            timer->print_hierarchy();
            std::cout << std::endl;
        }
        std::cout << "=================================" << std::endl;
    }

    void clear() {
        root_timers_.clear();
    }

private:
    std::vector<std::unique_ptr<Timer>> root_timers_;
};

} // namespace aw2

#endif // AW2_TIMER_H