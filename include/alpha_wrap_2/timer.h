#ifndef AW2_TIMER_H
#define AW2_TIMER_H

#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <iomanip>

namespace aw2 {

class Timer {
public:
    Timer(const std::string& name = "Timer") 
        : name_(name), 
          accumulated_time_(std::chrono::nanoseconds::zero()),
          is_running_(false) {}

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

    // Create a child timer
    Timer* create_child(const std::string& name) {
        children_.push_back(std::make_unique<Timer>(name));
        return children_.back().get();
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

    // Get time spent in children
    double children_time_ms() const {
        double total_children_time = 0.0;
        for (const auto& child : children_) {
            total_children_time += child->elapsed_ms();
        }
        return total_children_time;
    }

    // Get self time (excluding children)
    double self_time_ms() const {
        return elapsed_ms() - children_time_ms();
    }

    // Print hierarchical timing report
    void print_hierarchy(int indent = 0) const {
        std::string prefix(indent * 2, ' ');
        double total_time = elapsed_ms();
        
        if (indent == 0) {
            std::cout << prefix << name_ << ": " << total_time << " ms (100.0%)" << std::endl;
        } else {
            std::cout << prefix << "├─ " << name_ << ": " << total_time << " ms" << std::endl;
        }
        
        // Print children
        for (const auto& child : children_) {
            child->print_hierarchy(indent + 1);
        }
        
        // Print self time if there are children
        if (!children_.empty()) {
            std::string self_prefix(indent * 2 + 2, ' ');
            double self_time = self_time_ms();
            double self_percentage = (self_time / total_time) * 100.0;
            std::cout << self_prefix << "└─ [self]: " << self_time << " ms (" 
                      << std::fixed << std::setprecision(1) << self_percentage << "%)" << std::endl;
        }
    }

private:
    std::string name_;
    std::vector<std::unique_ptr<Timer>> children_;
    std::chrono::high_resolution_clock::time_point start_time_;
    std::chrono::nanoseconds accumulated_time_;
    bool is_running_;
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
        std::cout << "\n=== TIMING REPORT ===" << std::endl;
        for (const auto& timer : root_timers_) {
            timer->print_hierarchy();
            std::cout << std::endl;
        }
        std::cout << "=================================" << std::endl;
    }

private:
    std::vector<std::unique_ptr<Timer>> root_timers_;
};

} // namespace aw2

#endif // AW2_TIMER_H