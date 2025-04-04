/*  Inheritance-compatible base class for singleton design pattern. Derive a 
    class from this and use the same class as the template argument.
 */

#pragma once

#include <memory>

namespace patch
{
    template <class Derived>
    class Singleton
    {
    public:
        Singleton(const Singleton&) = delete;
        Singleton(Singleton&&) = delete;
        Singleton& operator=(const Singleton& other) = delete;
        Singleton& operator=(Singleton&&) = delete;

        static Derived* getInstance()
        {
            if (instance == nullptr)
            {
                instance = std::unique_ptr<Derived>(new Derived());
            }

            return instance.get();
        }

    protected:
        Singleton() = default;

        inline static std::unique_ptr<Derived> instance = nullptr;
    };

} // namespace norm