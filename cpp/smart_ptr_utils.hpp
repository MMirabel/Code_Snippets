/**
 * Smart Pointer Utilities and RAII Patterns
 * 
 * Header-only C++17 utilities for:
 * - Custom RAII wrappers
 * - Smart pointer factories
 * - Resource management patterns
 * 
 * Complessità: O(1) per creazione/distruzione
 * Thread-safety: Dipende dall'implementazione della risorsa
 */

#ifndef SMART_PTR_UTILS_HPP
#define SMART_PTR_UTILS_HPP

#include <memory>
#include <functional>
#include <type_traits>
#include <utility>
#include <iostream>
#include <fstream>
#include <mutex>

namespace utils {

// =============================================================================
// RAII WRAPPER FOR ARBITRARY RESOURCES
// =============================================================================

template<typename Resource, typename Deleter>
class RAIIWrapper {
private:
    Resource resource_;
    Deleter deleter_;
    bool valid_;

public:
    // Constructor
    template<typename... Args>
    RAIIWrapper(Deleter deleter, Args&&... args)
        : resource_(std::forward<Args>(args)...)
        , deleter_(std::move(deleter))
        , valid_(true) {}
    
    // Move constructor
    RAIIWrapper(RAIIWrapper&& other) noexcept
        : resource_(std::move(other.resource_))
        , deleter_(std::move(other.deleter_))
        , valid_(other.valid_) {
        other.valid_ = false;
    }
    
    // Move assignment
    RAIIWrapper& operator=(RAIIWrapper&& other) noexcept {
        if (this != &other) {
            reset();
            resource_ = std::move(other.resource_);
            deleter_ = std::move(other.deleter_);
            valid_ = other.valid_;
            other.valid_ = false;
        }
        return *this;
    }
    
    // Disable copy
    RAIIWrapper(const RAIIWrapper&) = delete;
    RAIIWrapper& operator=(const RAIIWrapper&) = delete;
    
    // Destructor
    ~RAIIWrapper() {
        reset();
    }
    
    // Access
    Resource& get() { return resource_; }
    const Resource& get() const { return resource_; }
    
    Resource& operator*() { return resource_; }
    const Resource& operator*() const { return resource_; }
    
    Resource* operator->() { return &resource_; }
    const Resource* operator->() const { return &resource_; }
    
    // Check validity
    explicit operator bool() const { return valid_; }
    
    // Release resource manually
    void reset() {
        if (valid_) {
            deleter_(resource_);
            valid_ = false;
        }
    }
};

// Factory function for RAII wrapper
template<typename Resource, typename Deleter, typename... Args>
auto make_raii(Deleter&& deleter, Args&&... args) {
    return RAIIWrapper<Resource, std::decay_t<Deleter>>(
        std::forward<Deleter>(deleter),
        std::forward<Args>(args)...
    );
}

// =============================================================================
// SMART POINTER FACTORIES
// =============================================================================

// Factory for unique_ptr with custom deleter
template<typename T, typename Deleter, typename... Args>
auto make_unique_with_deleter(Deleter&& deleter, Args&&... args) {
    return std::unique_ptr<T, std::decay_t<Deleter>>(
        new T(std::forward<Args>(args)...),
        std::forward<Deleter>(deleter)
    );
}

// Factory for shared_ptr with custom deleter
template<typename T, typename Deleter, typename... Args>
auto make_shared_with_deleter(Deleter&& deleter, Args&&... args) {
    return std::shared_ptr<T>(
        new T(std::forward<Args>(args)...),
        std::forward<Deleter>(deleter)
    );
}

// =============================================================================
// FILE HANDLE RAII
// =============================================================================

class FileHandle {
private:
    std::FILE* file_;
    
public:
    explicit FileHandle(const char* filename, const char* mode)
        : file_(std::fopen(filename, mode)) {
        if (!file_) {
            throw std::runtime_error("Failed to open file");
        }
    }
    
    ~FileHandle() {
        if (file_) {
            std::fclose(file_);
        }
    }
    
    // Move semantics
    FileHandle(FileHandle&& other) noexcept : file_(other.file_) {
        other.file_ = nullptr;
    }
    
    FileHandle& operator=(FileHandle&& other) noexcept {
        if (this != &other) {
            if (file_) {
                std::fclose(file_);
            }
            file_ = other.file_;
            other.file_ = nullptr;
        }
        return *this;
    }
    
    // Disable copy
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;
    
    // Access
    std::FILE* get() const { return file_; }
    explicit operator bool() const { return file_ != nullptr; }
    
    // Utility methods
    size_t write(const void* data, size_t size, size_t count) {
        return std::fwrite(data, size, count, file_);
    }
    
    size_t read(void* data, size_t size, size_t count) {
        return std::fread(data, size, count, file_);
    }
    
    void flush() {
        std::fflush(file_);
    }
};

// =============================================================================
// SCOPED GUARD
// =============================================================================

template<typename Callable>
class ScopedGuard {
private:
    Callable cleanup_;
    bool active_;
    
public:
    explicit ScopedGuard(Callable cleanup)
        : cleanup_(std::move(cleanup)), active_(true) {}
    
    ~ScopedGuard() {
        if (active_) {
            cleanup_();
        }
    }
    
    // Move semantics
    ScopedGuard(ScopedGuard&& other) noexcept
        : cleanup_(std::move(other.cleanup_)), active_(other.active_) {
        other.active_ = false;
    }
    
    ScopedGuard& operator=(ScopedGuard&& other) noexcept {
        if (this != &other) {
            if (active_) {
                cleanup_();
            }
            cleanup_ = std::move(other.cleanup_);
            active_ = other.active_;
            other.active_ = false;
        }
        return *this;
    }
    
    // Disable copy
    ScopedGuard(const ScopedGuard&) = delete;
    ScopedGuard& operator=(const ScopedGuard&) = delete;
    
    // Dismiss the guard (don't execute cleanup)
    void dismiss() {
        active_ = false;
    }
};

// Factory for scoped guard
template<typename Callable>
auto make_scoped_guard(Callable&& cleanup) {
    return ScopedGuard<std::decay_t<Callable>>(std::forward<Callable>(cleanup));
}

// =============================================================================
// THREAD-SAFE SINGLETON
// =============================================================================

template<typename T>
class Singleton {
private:
    static std::once_flag flag_;
    static std::unique_ptr<T> instance_;
    
public:
    template<typename... Args>
    static T& instance(Args&&... args) {
        std::call_once(flag_, [&]() {
            // Use new directly since make_unique cannot access private constructor
            instance_.reset(new T(std::forward<Args>(args)...));
        });
        return *instance_;
    }
    
    // Disable copy and move
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete;
    
protected:
    Singleton() = default;
};

template<typename T>
std::once_flag Singleton<T>::flag_;

template<typename T>
std::unique_ptr<T> Singleton<T>::instance_;

} // namespace utils

// =============================================================================
// USAGE EXAMPLES
// =============================================================================

#ifdef SMART_PTR_UTILS_EXAMPLE

#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>

// Example resource class
class Resource {
private:
    int id_;
    
public:
    explicit Resource(int id) : id_(id) {
        std::cout << "Resource " << id_ << " created\n";
    }
    
    ~Resource() {
        std::cout << "Resource " << id_ << " destroyed\n";
    }
    
    int getId() const { return id_; }
    void doWork() {
        std::cout << "Resource " << id_ << " working...\n";
    }
};

// Custom deleter example
struct ResourceDeleter {
    void operator()(Resource* ptr) {
        std::cout << "Custom deleting resource " << ptr->getId() << "\n";
        delete ptr;
    }
};

// Example singleton class
class Logger : public utils::Singleton<Logger> {
    friend class utils::Singleton<Logger>;
    // Allow make_unique to access private constructor
    friend std::unique_ptr<Logger> std::make_unique<Logger>(const std::string&);
    
private:
    std::string prefix_;
    
    Logger(const std::string& prefix = "LOG") : prefix_(prefix) {}
    
public:
    void log(const std::string& message) {
        std::cout << "[" << prefix_ << "] " << message << "\n";
    }
};

void example_raii_wrapper() {
    std::cout << "=== RAII Wrapper Example ===\n";
    
    // RAII wrapper for vector with custom cleanup
    auto vec_wrapper = utils::make_raii<std::vector<int>>(
        [](auto& vec) { 
            std::cout << "Cleaning up vector with " << vec.size() << " elements\n";
        },
        10  // initial size
    );
    
    vec_wrapper->push_back(42);
    vec_wrapper->push_back(24);
    
    std::cout << "Vector size: " << vec_wrapper->size() << "\n";
    // Automatic cleanup when going out of scope
}

void example_smart_ptr_factories() {
    std::cout << "\n=== Smart Pointer Factories Example ===\n";
    
    // unique_ptr with custom deleter
    auto unique_res = utils::make_unique_with_deleter<Resource>(
        ResourceDeleter{}, 1
    );
    unique_res->doWork();
    
    // shared_ptr with custom deleter
    auto shared_res = utils::make_shared_with_deleter<Resource>(
        [](Resource* ptr) {
            std::cout << "Lambda deleting resource " << ptr->getId() << "\n";
            delete ptr;
        },
        2
    );
    shared_res->doWork();
}

void example_file_handle() {
    std::cout << "\n=== File Handle Example ===\n";
    
    try {
        utils::FileHandle file("/tmp/test.txt", "w");
        const char* data = "Hello, RAII World!\n";
        file.write(data, 1, strlen(data));
        file.flush();
        std::cout << "File written successfully\n";
        // File automatically closed when going out of scope
    } catch (const std::exception& e) {
        std::cout << "File operation failed: " << e.what() << "\n";
    }
}

void example_scoped_guard() {
    std::cout << "\n=== Scoped Guard Example ===\n";
    
    bool resource_acquired = false;
    
    // Simulate resource acquisition
    resource_acquired = true;
    std::cout << "Resource acquired\n";
    
    // Create guard to ensure cleanup
    auto guard = utils::make_scoped_guard([&]() {
        if (resource_acquired) {
            std::cout << "Cleaning up resource\n";
            resource_acquired = false;
        }
    });
    
    // Do some work...
    std::cout << "Doing work with resource\n";
    
    // Guard will automatically clean up on scope exit
}

void example_singleton() {
    std::cout << "\n=== Singleton Example ===\n";
    
    // Get singleton instances
    auto& logger1 = Logger::instance("APP");
    auto& logger2 = Logger::instance("SHOULD_BE_IGNORED");
    
    logger1.log("First message");
    logger2.log("Second message");  // Same instance as logger1
    
    std::cout << "logger1 and logger2 are same: " 
              << (&logger1 == &logger2 ? "true" : "false") << "\n";
}

int main() {
    std::cout << "Smart Pointer Utilities Examples\n";
    std::cout << "================================\n";
    
    example_raii_wrapper();
    example_smart_ptr_factories();
    example_file_handle();
    example_scoped_guard();
    example_singleton();
    
    std::cout << "\nAll examples completed!\n";
    return 0;
}

#endif // SMART_PTR_UTILS_EXAMPLE

#endif // SMART_PTR_UTILS_HPP